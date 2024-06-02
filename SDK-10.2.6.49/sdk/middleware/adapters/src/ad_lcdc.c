/**
 ****************************************************************************************
 *
 * @file ad_lcdc.c
 *
 * @brief LCD controller adapter implementation
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configLCDC_ADAPTER

#include <stdint.h>
#include "hw_lcdc.h"
#include "osal.h"
#include "hw_gpio.h"
#include "hw_sys.h"
#include "resmgmt.h"
#include "sys_power_mgr.h"
#include "sys_clock_mgr.h"
#include "ad_pmu.h"
#include "ad_lcdc.h"

/**
 * \brief Compacts port / pin values in a single byte
 */
#define AD_LCDC_COMPACT_PINS(_port, _pin) \
        (_port << HW_GPIO_PIN_BITS) | (_pin & ((1 << HW_GPIO_PIN_BITS) - 1))

/**
 * \brief Returns the port value from the compacted value created with \ref AD_LCDC_COMPACT_PINS
 */
#define AD_LCDC_GET_PORT(value)                 (value >> HW_GPIO_PIN_BITS)

/**
 * \brief Returns the pin value from the compacted value created with \ref AD_LCDC_COMPACT_PINS
 */
#define AD_LCDC_GET_PIN(value)                  (value & ((1 << HW_GPIO_PIN_BITS) - 1))

/**
 * \brief Sets a bit in the provided value
 */
#define AD_LCDC_SET_BIT(value, bit)             ((value) |= (1U << (bit)))

/**
 * \brief Clears a bit in the provided value
 */
#define AD_LCDC_CLR_BIT(value, bit)             ((value) &= ~(1U << (bit)))

/**
 * \brief Checks if the provided handle is valid
 */
#define AD_LCDC_HANDLE_IS_VALID(x)              ((((ad_lcdc_data_t *)(x)) == &lcdc_data)           \
                                             && (((ad_lcdc_data_t *)(x))->conf != NULL))

/**
 * \brief Type of group of pins
 */
typedef enum {
        AD_LCDC_LATCH_TYPE_CTRL_SIG,            //!< Serial and parallel control signal pins
        AD_LCDC_LATCH_TYPE_EXT_CLK,             //!< External clock pins
} AD_LCDC_LATCH_TYPE;

/**
 * \brief LCDC run time data
 *
 * Structure keeps LCD related dynamic data.
 */
typedef struct {
        hw_lcdc_layer_t layer[HW_LCDC_LAYER_MAX];//!< Maintains layer configuration when adapter closes
        int16_t disp_offsetx;                   //!< Horizontal offset of first pixel in display's memory (MIPI devices)
        int16_t disp_offsety;                   //!< Vertical offset of first pixel in display's memory (MIPI devices)
        hw_lcdc_frame_t frame;                  //!< Frame dimensions for the partial mode
        bool layer_en[HW_LCDC_LAYER_MAX];       //!< Keeps track if layer is enabled (visible)
        bool frame_valid;                       //!< Validity flag for the frame (partial mode)
} ad_lcdc_device_data_t;

/**
 * \brief LCDC run time data
 *
 * Structure keeps LCD related dynamic data that live as long as configuration is open.
 */
typedef struct {
        const ad_lcdc_controller_conf_t *conf;  //!< LCDC controller current configuration
        ad_lcdc_device_data_t *data;            //!< LCD configuration / state
        OS_TASK owner;                          //!< Task that has acquired this device.
        OS_MUTEX busy;                          //!< Semaphore for thread safety
        OS_EVENT event;                         //!< Event for async calls
        ad_lcdc_user_cb callback;               //!< Callback function to call after transaction ends
        void *callback_data;                    //!< Callback data to pass to \p callback
        int lld_status;                         //!< Holds operation status provided by LLD
#if CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS
        sys_clk_t clock_conf;                   //!< Keeps track of the configured system clock by LCDC adapter
#endif /* CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS */
        bool cont_mode_active;                  //!< Keeps track if continuous mode is active
} ad_lcdc_data_t;

/**
 * \brief LCDC callback data
 */
typedef struct {
        ad_lcdc_data_t *lcdc;                   //!< Pointer to current \ref ad_lcdc_data_t structure
        HW_LCDC_ERR status;                     //!< Operation status
} lcdc_cb_data_t;

/**
 * \brief Holds current device for LCDC.
 */
__RETAINED static ad_lcdc_data_t lcdc_data;

/**
 * \brief Holds LCD specific data retained after calling \ref ad_lcdc_close function.
 */
__RETAINED static ad_lcdc_device_data_t lcdc_dev_data;

/**
 * \brief Array containing the fixed assignment signal pins
 */
static const uint8_t ad_lcdc_signal_gpios[] = {
        /*
         *
         *   JDI LCD I/F  |  Hsync/Vsync Parallel    |  MIPI DBI       |  LCD SPI3/4        |  GPIO
         *                |  (MIPI DPI-2)            |  Type B         |                    |
         * ---------------|--------------------------|-----------------|--------------------|--------
         *   HCK      | O | DPI_CLK              | O | DBIB_CSX   | O  | LCD_SPI_SCLK   | O | P0_14
         *   ENB      | O | DPI_DE               | O | DBIB_RESX  | O  | LCD_CS         | O | P0_18
         *   HST      | O | DPI_HSYNC            | O | DBIB_D/CX  | O  | LCD_SPI_SD     | O | P0_15
         *   VST      | O | DPI_VSYNC            | O | DBIB_WRX   | O  | LCD_SPI_SD1/DC | O | P0_16
         *   XRST     | O | DPI_SD               | O | DBIB_RDX   | O  | LCD_SPI_SD2    | O | P0_22
         *   VCK      | O | DPI_CM               | O | DBIB_STALL | I  | LCD_SPI_SI     | I | P0_09
         *   RED0     | O | DPI_RED0 (DATA[0])   | O | DBIB_DB0   | IO | LCD_SPI_SD3    | O | P0_17
         *   RED1     | O | DPI_RED1 (DATA[1])   | O | DBIB_DB1   | IO |                |   | P0_23
         *   GREEN0   | O | DPI_GREEN0 (DATA[2]) | O | DBIB_DB2   | IO |                |   | P0_24
         *   GREEN1   | O | DPI_GREEN1 (DATA[3]) | O | DBIB_DB3   | IO |                |   | P1_00
         *   BLUE0    | O | DPI_BLUE0 (DATA[4])  | O | DBIB_DB4   | IO |                |   | P1_01
         *   BLUE1    | O | DPI_BLUE1 (DATA[5])  | O | DBIB_DB5   | IO |                |   | P0_21
         *   VCOM/FRP | O |                      |   | DBIB_DB6   | IO | EXTCOMIN       | O | P0_19
         *            |   |                      |   | DBIB_DB7   | IO |                |   | P1_07
         *   XFRP     | O | DPI_READY            | I | DBIB_TE    | I  | LCD_TE         | I | P0_10
         */
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_9),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_10),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_14),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_15),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_16),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_17),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_18),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_19),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_21),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_22),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_23),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_24),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_0),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_1),
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_7),
};

/**
 * \brief Array containing the fixed assignment external clock pins
 */
static const uint8_t ad_lcdc_ext_gpios[] = {
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_19),//VCOM, FRP, EXTCOMIN
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_10),//XFRP
};

/*
 * FORWARD DECLARATIONS
 *****************************************************************************************
 */
static void ad_lcdc_gpio_configure(const ad_lcdc_io_conf_t *io_cfg, AD_LCDC_LATCH_TYPE type);
static void ad_lcdc_gpio_deconfigure(const ad_lcdc_io_conf_t *io_cfg, AD_LCDC_LATCH_TYPE type);
static bool ad_lcdc_check_cs(const ad_lcdc_io_conf_t *io_cfg);

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 * \brief Sets the external clock function to the dedicated pins
 *
 * \param [in] port             GPIO port
 * \param [in] pin              GPIO pin
 * \param [in] enable           External clock function
 */
static void ad_lcdc_gpio_set_ext_func(HW_GPIO_PORT port, HW_GPIO_PIN pin, bool enable)
{
        if (port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_19) {
                REG_SETF(CRG_TOP, SLP_MAP_REG, LCD_EXT_CLK_SLP_MAP, enable ? 1 : 0);
                if (enable) {
                        /* Ensure that the pin is not mapped to LCDC */
                        REG_SETF(GPIO, LCDC_MAP_CTRL_REG, MAP_ON_P0_19_EN, 0);
                }
        } else if (port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_10) {
                REG_SETF(CRG_TOP, SLP_MAP_REG, LCD_INV_EXT_CLK_SLP_MAP, enable ? 1 : 0);
                if (enable) {
                        /* Ensure that the pin is not mapped to LCDC */
                        REG_SETF(GPIO, LCDC_MAP_CTRL_REG, MAP_ON_P0_10_EN, 0);
                }
        }
}

/**
 * \brief Returns if a specific pin is configured as an external clock
 *
 * \param [in] port             GPIO port
 * \param [in] pin              GPIO pin
 *
 * \return true if pin is configured as external clock
 */
static bool ad_lcdc_gpio_get_ext_func(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        if (port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_19) {
                return REG_GETF(CRG_TOP, SLP_MAP_REG, LCD_EXT_CLK_SLP_MAP) ? true : false;
        } else if (port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_10) {
                return REG_GETF(CRG_TOP, SLP_MAP_REG, LCD_INV_EXT_CLK_SLP_MAP) ? true : false;
        }
        return false;
}

/**
 * \brief Configures the state, type and mode of an LCDC pin
 *
 * The LCDC has dedicated pins and no dedicated function. If a GPIO is configured as an LCDC pin is
 * controlled by the LCDC_MAP_CTRL_REG register. When a GPIO is configured as a LCDC pin, the pin
 * function is overridden.
 *
 * \param [in] port             GPIO port
 * \param [in] pin              GPIO pin
 * \param [in] mode             GPIO mode
 * \param [in] function         GPIO function
 * \param [in] high             Pin state to set
 */
static void ad_lcdc_configure_map_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode,
                                                        HW_GPIO_FUNC function, bool high)
{
        uint8_t pp = AD_LCDC_COMPACT_PINS(port, pin);
        for (size_t i = 0; i < ARRAY_LENGTH(ad_lcdc_signal_gpios); ++i) {
                if (pp == ad_lcdc_signal_gpios[i]) {
                        if (function == HW_GPIO_FUNC_LCD) {
                                AD_LCDC_SET_BIT(GPIO->LCDC_MAP_CTRL_REG, i);
                                function = HW_GPIO_FUNC_GPIO;
                        } else {
                                AD_LCDC_CLR_BIT(GPIO->LCDC_MAP_CTRL_REG, i);
                        }
                        break;
                }
        }

        hw_gpio_configure_pin(port, pin, mode, function, high);
}


#if CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS
/**
 * \brief Enables or disables the required clock configuration based on the interface type and speed
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] conf             Driver configuration
 * \param [in] enable           Enable or disable the clock configuration
 *
 * \return 0: success, <0: error
 */
static int ad_lcdc_configure_clock(ad_lcdc_handle_t handle, const ad_lcdc_driver_conf_t *conf, bool enable)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        cm_sys_clk_set_status_t clk_status = cm_sysclk_success;
        sys_clk_t sys_clock_req = sysclk_LP;

        {
                if (conf->hw_init.write_freq & HW_LCDC_CLK_PLL_BIT) {
                        sys_clock_req = sysclk_PLL160;
                } else if (conf->hw_init.write_freq & HW_LCDC_CLK_RCHS_BIT) {
                        sys_clock_req = sysclk_RCHS_96;
                }
        }

        if (sys_clock_req != lcdc->clock_conf || !enable) {
                if (enable) {
                        clk_status = cm_sys_clk_request(sys_clock_req);
                }
                if (lcdc->clock_conf != sysclk_LP) {
                        cm_sys_clk_release(lcdc->clock_conf);
                }

                /* Change state even if not successfully set */
                lcdc->clock_conf = enable ? sys_clock_req : sysclk_LP;
        }

        if (clk_status != cm_sysclk_success) {
                return AD_LCDC_ERROR_SRC_CLOCKS;
        }

        return AD_LCDC_ERROR_NONE;
}
#endif /* CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS */

/**
 * \brief Handles and translates the errors received from LLD to the adapter's corresponding
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] error            Error code received from LLD function
 *
 * \return 0: success, <0: error
 */
static AD_LCDC_ERROR ad_lcdc_error_translate(ad_lcdc_handle_t handle, HW_LCDC_ERR error)
{
        switch (error) {
        case HW_LCDC_ERR_UNSUPPORTED:
                return AD_LCDC_ERROR_UNSUPPORTED;
        case HW_LCDC_ERR_PARAM_INVALID:
                return AD_LCDC_ERROR_PARAM_INVALID;
        case HW_LCDC_ERR_UNDERFLOW:
                return AD_LCDC_ERROR_UNDERFLOW;
        case HW_LCDC_ERR_CONF_INVALID:
                return AD_LCDC_ERROR_DRIVER_CONF_INVALID;
        case HW_LCDC_ERR_NONE:
                return AD_LCDC_ERROR_NONE;
        default:
                return AD_LCDC_ERROR_UNKNOWN;
        }
}

/**
 * \brief Configures the state, type and mode of a GPIO pin
 *
 * \param [in] cfg              IO configuration
 * \param [in] voltage_level    Voltage level to be applied
 * \param [in] state            IO configuration state
 */
static void ad_lcdc_configure_pin(const ad_io_conf_t *cfg, HW_GPIO_POWER voltage_level,
                                                                        AD_IO_CONF_STATE state)
{
        HW_GPIO_MODE mode = state == AD_IO_CONF_ON ? cfg->on.mode : cfg->off.mode;
        HW_GPIO_FUNC function = state == AD_IO_CONF_ON ? cfg->on.function : cfg->off.function;
        bool high = state == AD_IO_CONF_ON ? cfg->on.high : cfg->off.high;

        if (!ad_lcdc_gpio_get_ext_func(cfg->port, cfg->pin)) {
                ad_lcdc_configure_map_pin(cfg->port, cfg->pin, mode, function, high);
        } else {
                hw_gpio_configure_pin(cfg->port, cfg->pin, mode,
                                function == HW_GPIO_FUNC_LCD ? HW_GPIO_FUNC_GPIO : function, high);
        }

        hw_gpio_configure_pin_power(cfg->port, cfg->pin, voltage_level);
}

void ad_lcdc_init(void)
{
        /* Adapter internal initializations */
        OS_MUTEX_CREATE(lcdc_data.busy);
        OS_EVENT_CREATE(lcdc_data.event);

        for (HW_LCDC_LAYER i = 0; i < HW_LCDC_LAYER_MAX; i++) {
                lcdc_dev_data.layer_en[i] = false;
        }
}

ad_lcdc_handle_t ad_lcdc_open(const ad_lcdc_controller_conf_t *conf)
{
        ad_lcdc_data_t *lcdc = &lcdc_data;

        /* Check input validity*/
        OS_ASSERT(conf);
        OS_ASSERT(conf->drv);

        /* Acquire resources */
        resource_acquire(RES_MASK(RES_ID_LCDC), RES_WAIT_FOREVER);

        /* Update adapter data */
        lcdc->conf = conf;
        lcdc->owner = OS_GET_CURRENT_TASK();
        lcdc->data = &lcdc_dev_data;
#if CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS
        lcdc->clock_conf = sysclk_LP;
#endif /* CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS */

        pm_sleep_mode_request(pm_mode_idle);

        if (ad_lcdc_reconfig(lcdc, conf->drv) == AD_LCDC_ERROR_NONE) {
                ad_lcdc_gpio_configure(conf->io, AD_LCDC_LATCH_TYPE_CTRL_SIG);

                return lcdc;
        }

        ad_lcdc_close(lcdc, false);
        return NULL;
}

int ad_lcdc_reconfig(ad_lcdc_handle_t handle, const ad_lcdc_driver_conf_t *conf)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        HW_LCDC_ERR hw_status;
        int ret = AD_LCDC_ERROR_UNKNOWN;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        /* If LCDC driver is already configured, check if the new configuration could be applied */
        if (lcdc->conf->drv->hw_init.phy_type != conf->hw_init.phy_type) {
                OS_ASSERT(0);
                ret = AD_LCDC_ERROR_DRIVER_CONF_INVALID;
                goto end;
        }

#if CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS
        ret = ad_lcdc_configure_clock(handle, conf, true);
        if (AD_LCDC_ERROR_NONE != ret) {
                OS_ASSERT(0);
                goto end;
        }
#endif /* CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS */


        hw_status = hw_lcdc_init(&conf->hw_init);
        OS_ASSERT(hw_status == HW_LCDC_ERR_NONE);
        if (hw_status != HW_LCDC_ERR_NONE) {
                goto restore;
        }

        /* Restore the partial update mode */
        if (lcdc->data->frame_valid) {
                hw_lcdc_set_update_region(&lcdc->data->frame);
        }
        /* Restore layer info */
        for (HW_LCDC_LAYER i = 0; i < HW_LCDC_LAYER_MAX; i++) {
                hw_lcdc_set_layer(i, lcdc->data->layer_en[i], &lcdc->data->layer[i]);
        }
        if (conf->palette_lut != NULL) {
                hw_lcdc_set_palette(0, conf->palette_lut, HW_LCDC_PALETTE_ENTRIES);
                hw_lcdc_set_palette_state(true);
        }

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;

restore:
#if CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS
        ad_lcdc_configure_clock(handle, conf, false);
#endif /* CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS */

end:
        OS_MUTEX_PUT(lcdc->busy);

        return ret;
}

int ad_lcdc_close(ad_lcdc_handle_t handle, bool force)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        hw_lcdc_config_t cfg = {
                .phy_type = HW_LCDC_PHY_NONE,
        };

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);
        if (!force) {
                if (hw_lcdc_is_busy()) {
                        OS_MUTEX_PUT(lcdc->busy);
                        return AD_LCDC_ERROR_CONTROLLER_BUSY;
                }

                if (ad_lcdc_check_cs(lcdc->conf->io))
                {
                        OS_MUTEX_PUT(lcdc->busy);
                        return AD_LCDC_ERROR_CONTROLLER_BUSY;
                }
        }

        ad_lcdc_gpio_deconfigure(lcdc->conf->io, AD_LCDC_LATCH_TYPE_CTRL_SIG);

        hw_lcdc_init(&cfg);

#if CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS
        ad_lcdc_configure_clock(handle, lcdc->conf->drv, false);
#endif /* CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS */

        /* Update adapter data */
        lcdc->conf = NULL;
        lcdc->owner = NULL;

        resource_release(RES_MASK(RES_ID_LCDC));
        OS_MUTEX_PUT(lcdc->busy);

        pm_sleep_mode_release(pm_mode_idle);

        return AD_LCDC_ERROR_NONE;
}

int ad_lcdc_io_config(const ad_lcdc_io_conf_t *io, AD_IO_CONF_STATE state)
{
        if (!io) {
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        /* Perform initial setup of the pins of each device */
        for (const ad_io_conf_t *cfg = io->io_list; cfg < io->io_cnt + io->io_list; cfg++) {
                if (cfg->port == HW_GPIO_PORT_NONE || cfg->pin == HW_GPIO_PIN_NONE) {
                        continue;
                }
                ad_lcdc_configure_pin(cfg, io->voltage_level, state);
                ad_io_set_pad_latch(io->io_list, io->io_cnt, AD_IO_PAD_LATCHES_OP_TOGGLE);
        }
        return AD_LCDC_ERROR_NONE;
}

/**
 * \brief Function checks if a CS pin is configured and if found it waits until it is de-asserted
 *
 * \param [in] io_cfg           LCD controller GPIO configuration
 */
static bool ad_lcdc_check_cs(const ad_lcdc_io_conf_t *io_cfg)
{
        const ad_io_conf_t *cfg;

        if (!io_cfg) {
                return false;
        }

        /* Ensure that transfer has been completed. IRQ does not ensure that since it is
         * called before the end of the transfer.  */
        for (cfg = io_cfg->io_list; cfg < io_cfg->io_cnt + io_cfg->io_list; cfg++) {
                if (cfg->on.function == HW_GPIO_FUNC_LCD && cfg->port == HW_GPIO_PORT_0
                        && cfg->pin == HW_GPIO_PIN_18) {
                        break;
                }
        }
        if (cfg != io_cfg->io_cnt + io_cfg->io_list) {
                bool state = !!(hw_lcdc_get_mipi_cfg() & HW_LCDC_MIPI_CFG_SPI_CSX_V);
                return (hw_gpio_get_pin_status(cfg->port, cfg->pin) == state);
        }
        return false;
}

/**
 * \brief Helper function that returns the pin list and its size associated to the provided type
 *
 * \param [out] pin_list        Pin list pointer
 * \param [in]  type            Type of pins to return
 *
 * \return >0 size of list, 0: error
 */
static size_t ad_lcdc_get_pins(const uint8_t **pin_list, AD_LCDC_LATCH_TYPE type)
{
        switch (type) {
        case AD_LCDC_LATCH_TYPE_CTRL_SIG:
                *pin_list = ad_lcdc_signal_gpios;
                return sizeof(ad_lcdc_signal_gpios);
        case AD_LCDC_LATCH_TYPE_EXT_CLK:
                *pin_list = ad_lcdc_ext_gpios;
                return sizeof(ad_lcdc_ext_gpios);
        default:
                return 0;
        }
}

/**
 * \brief Verifies that the provided pin configuration corresponds to a valid LCDC pin
 *
 * \param [in] pin_list         List of valid pins
 * \param [in] pin_list_size    Size of valid pins list
 * \param [in] cfg              Pin configuration
 * \param [in] type
 *
 * \return true if the
 */
static bool ad_lcdc_gpio_verify_pin(const uint8_t *pin_list, size_t pin_list_size, const ad_io_conf_t *cfg, AD_LCDC_LATCH_TYPE type)
{
        uint8_t pin = AD_LCDC_COMPACT_PINS(cfg->port, cfg->pin);

        if (cfg->port > HW_GPIO_PORT_NONE || cfg->pin > HW_GPIO_PIN_NONE) {
                return false;
        }
        if (cfg->on.function == HW_GPIO_FUNC_LCD) {
                size_t i;
                for (i = 0; i < pin_list_size; ++i) {
                        if (pin == pin_list[i]) {
                                break;
                        }
                }
                if (i == pin_list_size) {
                        return false;
                }
        } else {
                return false;
        }

        HW_LCDC_GPIO_IF iface = hw_lcdc_get_iface();

        /* Determine if the pin is an external clock based on the configured interface */
        if (cfg->port == HW_GPIO_PORT_0 && cfg->pin == HW_GPIO_PIN_19) {
                if (iface == HW_LCDC_GPIO_IF_JDI || iface == HW_LCDC_GPIO_IF_SPI) {
                        return type == AD_LCDC_LATCH_TYPE_EXT_CLK;
                } else {
                        return type != AD_LCDC_LATCH_TYPE_EXT_CLK;
                }
        } else if (cfg->port == HW_GPIO_PORT_0 && cfg->pin == HW_GPIO_PIN_10) {
                if (iface == HW_LCDC_GPIO_IF_JDI) {
                        return type == AD_LCDC_LATCH_TYPE_EXT_CLK;
                } else {
                        return type != AD_LCDC_LATCH_TYPE_EXT_CLK;
                }
        }

        return true;
}

/**
 * \brief Configure corresponding group of pins to be ready for use
 *
 * \param [in] io_cfg           LCD controller GPIO configuration
 * \param [in] type             Type of pins to configure
 */
static void ad_lcdc_gpio_configure(const ad_lcdc_io_conf_t *io_cfg, AD_LCDC_LATCH_TYPE type)
{
        const uint8_t *pin_list;
        size_t pin_list_size;

        if (!io_cfg) {
                return;
        }

        if ((pin_list_size = ad_lcdc_get_pins(&pin_list, type)) == 0) {
                return;
        }


        for (const ad_io_conf_t *cfg = io_cfg->io_list;
                cfg < io_cfg->io_cnt + io_cfg->io_list; cfg++) {

                if (!ad_lcdc_gpio_verify_pin(pin_list, pin_list_size, cfg, type)) {
                        continue;
                }

                if (type == AD_LCDC_LATCH_TYPE_EXT_CLK) {
                        ad_lcdc_gpio_set_ext_func(cfg->port, cfg->pin, true);
                }
                ad_lcdc_configure_pin(cfg, io_cfg->voltage_level, AD_IO_CONF_ON);

                ad_io_set_pad_latch(cfg, 1, AD_IO_PAD_LATCHES_OP_ENABLE);
        }
}

/**
 * \brief Configure corresponding group of pins to be ready for sleep
 *
 * \param [in] io_cfg           LCD controller GPIO configuration
 * \param [in] type             Type of pins to configure
 */
static void ad_lcdc_gpio_deconfigure(const ad_lcdc_io_conf_t *io_cfg, AD_LCDC_LATCH_TYPE type)
{
        const uint8_t *pin_list;
        size_t pin_list_size;

        if (!io_cfg) {
                return;
        }

        if ((pin_list_size = ad_lcdc_get_pins(&pin_list, type)) == 0) {
                return;
        }

        for (const ad_io_conf_t *cfg = io_cfg->io_list;
                cfg < io_cfg->io_cnt + io_cfg->io_list; cfg++) {
                if (!ad_lcdc_gpio_verify_pin(pin_list, pin_list_size, cfg, type)) {
                        continue;
                }

                if (type == AD_LCDC_LATCH_TYPE_EXT_CLK) {
                        ad_lcdc_gpio_set_ext_func(cfg->port, cfg->pin, false);
                }

                ad_lcdc_configure_pin(cfg, io_cfg->voltage_level, AD_IO_CONF_OFF);
                ad_io_set_pad_latch(cfg, 1, AD_IO_PAD_LATCHES_OP_DISABLE);
        }

}

/**
 * \brief Set state of the provided pin
 *
 * \param [in] io               LCD controller GPIO configuration
 * \param [in] pin_index        Pin to configure as provided by \ref AD_LCDC_COMPACT_PINS
 * \param [in] state            State of the pin
 */
static void ad_lcdc_set_gpio_state(const ad_lcdc_io_conf_t *io, uint8_t pin_index, bool state)
{
        HW_GPIO_PORT port = AD_LCDC_GET_PORT(pin_index);
        HW_GPIO_PIN pin = AD_LCDC_GET_PIN(pin_index);

        if (!io) {
                return;
        }

        if (port == HW_GPIO_PORT_NONE || pin == HW_GPIO_PIN_NONE) {
                return;
        }

        for (const ad_io_conf_t *cfg = io->io_list; cfg < io->io_cnt + io->io_list; cfg++) {
                if (port == cfg->port && pin == cfg->pin) {
                        if (hw_lcdc_get_iface() == HW_LCDC_GPIO_IF_DBIB && cfg->on.function == HW_GPIO_FUNC_LCD
                                && port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_18) {
                                hw_lcdc_mipi_set_resx(state);
                        } else if (hw_lcdc_get_iface() == HW_LCDC_GPIO_IF_DPI && cfg->on.function == HW_GPIO_FUNC_LCD) {
                                if (port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_22) {
                                        hw_lcdc_set_dpi_sd(state);
                                } else if (port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_9) {
                                        hw_lcdc_set_dpi_cm(state);
                                }
                        } else if (cfg->on.function == HW_GPIO_FUNC_GPIO) {
                                hw_gpio_configure_pin(port, pin, cfg->on.mode, cfg->on.function,
                                        state);
                        } else {
                                break;
                        }
                        hw_gpio_configure_pin_power(port, pin, io->voltage_level);
                        hw_gpio_pad_latch_enable(port, pin);
                        hw_gpio_pad_latch_disable(port, pin);
                        break;
                }
        }
}


int ad_lcdc_execute_cmds(ad_lcdc_handle_t handle, const uint8_t *cmds, size_t len)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        int ret = AD_LCDC_ERROR_NONE;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        for (size_t index = 0; (len > index) && (ret == AD_LCDC_ERROR_NONE); ++index) {
                switch (cmds[index]) {
                case LCDC_TAG_DELAY_US: {
                        uint16_t delay;
                        delay = cmds[++index];
                        delay |= ((uint16_t)cmds[++index]) << 8;
                        hw_clk_delay_usec(delay);
                        break;
                }
                case LCDC_TAG_DELAY_MS: {
                        uint16_t delay;
                        delay = cmds[++index];
                        delay |= ((uint16_t)cmds[++index]) << 8;
                        OS_DELAY_MS(delay);
                        break;
                }
                case LCDC_TAG_GPIO_SET_ACTIVE:
                        ad_lcdc_set_gpio_state(lcdc->conf->io, cmds[++index], true);
                        break;
                case LCDC_TAG_GPIO_SET_INACTIVE:
                        ad_lcdc_set_gpio_state(lcdc->conf->io, cmds[++index], false);
                        break;
                case LCDC_TAG_MIPI_CMD:
                        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, cmds[++index]);
                        break;
                case LCDC_TAG_MIPI_PARAM:
                        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, cmds[++index]);
                        break;
                case LCDC_TAG_MIPI_CMD_PARAM:
                        ret = ad_lcdc_dcs_cmd_params(handle, cmds[index + 2], &cmds[index + 3], cmds[index + 1]);
                        index += cmds[index + 1] + 2;
                        break;
                case LCDC_TAG_GEN_CMD_PARAM: {
                        size_t cmd_len = cmds[index + 1];
                        size_t param_len = cmds[index + 2];
                        {
                                ret = hw_lcdc_gen_cmd_params(&cmds[index + 3], cmd_len, &cmds[index + cmd_len + 3], param_len);
                                ret = ad_lcdc_error_translate(handle, ret);
                        }
                        index += cmd_len + param_len + 2;
                        break;
                }
                case LCDC_TAG_JDI_CMD:
                        hw_lcdc_jdi_serial_cmd_send(cmds[++index]);
                        break;
                case LCDC_TAG_EXT_CLK:
                        ret = ad_lcdc_set_external_clock(handle, cmds[++index]);
                        break;
                default:
                        OS_ASSERT(0);
                }
        }

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}

int ad_lcdc_dcs_cmd_params(ad_lcdc_handle_t handle, HW_LCDC_MIPI_DCS cmd, const uint8_t *params, size_t param_len)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        int ret;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);
        {
                ret = hw_lcdc_dcs_cmd_params(cmd, params, param_len);
                ret = ad_lcdc_error_translate(handle, ret);
        }

        OS_MUTEX_PUT(lcdc->busy);

        return ret;
}

int ad_lcdc_dcs_read(ad_lcdc_handle_t handle, HW_LCDC_MIPI_DCS cmd, uint8_t *data, size_t data_len, size_t dummy_ticks)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        int ret = 0;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);
        {
                ret = hw_lcdc_dcs_read(cmd, data, data_len, dummy_ticks);
                if (ret < 0) {
                        ret = ad_lcdc_error_translate(handle, ret);
                }
        }

        OS_MUTEX_PUT(lcdc->busy);

        return ret;
}

int ad_lcdc_get_lld_status(ad_lcdc_handle_t handle)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        int ret = 0;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return 0;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        ret = lcdc->lld_status;

        OS_MUTEX_PUT(lcdc->busy);

        return ret;
}

int ad_lcdc_set_external_clock(ad_lcdc_handle_t handle, bool enable)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        if ((hw_lcdc_get_external_clk() != HW_LCDC_EXT_CLK_OFF) != enable) {
                if (enable) {
                        ad_lcdc_gpio_configure(lcdc->conf->io, AD_LCDC_LATCH_TYPE_EXT_CLK);
                } else {
                        ad_lcdc_gpio_deconfigure(lcdc->conf->io, AD_LCDC_LATCH_TYPE_EXT_CLK);
                }
                hw_lcdc_set_external_clk(enable ? lcdc->conf->drv->ext_clk : HW_LCDC_EXT_CLK_OFF);
        }

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}

/**
 * \brief Set screen dimensions that are updated by the controller
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] frame            Frame parameters
 *
 * \return 0 on success, <0: error
 */
static int ad_lcdc_mipi_set_position(ad_lcdc_handle_t handle, const hw_lcdc_frame_t *frame)
{
        AD_LCDC_ERROR ret;
        uint8_t column[] = {
                frame->startx >> 8, frame->startx & 0xFF,
                frame->endx >> 8, frame->endx & 0xFF
        };
        uint8_t page[] = {
                frame->starty >> 8, frame->starty & 0xFF,
                frame->endy >> 8, frame->endy & 0xFF
        };

        ret = ad_lcdc_dcs_cmd_params(handle, HW_LCDC_MIPI_DCS_SET_COLUMN_ADDRESS, column, sizeof(column));
        if (ret != AD_LCDC_ERROR_NONE) {
                return ret;
        }
        return ad_lcdc_dcs_cmd_params(handle, HW_LCDC_MIPI_DCS_SET_PAGE_ADDRESS, page, sizeof(page));
}

int ad_lcdc_set_partial_update(ad_lcdc_handle_t handle, hw_lcdc_frame_t *frame)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        hw_lcdc_frame_t mipi_frame;
        AD_LCDC_ERROR ret = AD_LCDC_ERROR_NONE;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        frame->endx = MIN(frame->endx, lcdc->conf->drv->hw_init.resx - 1);
        frame->endy = MIN(frame->endy, lcdc->conf->drv->hw_init.resy - 1);

        hw_lcdc_set_update_region(frame);

        /* Re-apply layer info to adjust to new frame size */
        for (HW_LCDC_LAYER i = 0; i < HW_LCDC_LAYER_MAX; i++) {
                hw_lcdc_set_layer(i, lcdc->data->layer_en[i], &lcdc->data->layer[i]);
        }

        mipi_frame.startx = frame->startx + lcdc->data->disp_offsetx;
        mipi_frame.starty = frame->starty + lcdc->data->disp_offsety;
        mipi_frame.endx = frame->endx + lcdc->data->disp_offsetx;
        mipi_frame.endy = frame->endy + lcdc->data->disp_offsety;

        if (lcdc->conf->drv->set_position_cb != NULL) {
                lcdc->conf->drv->set_position_cb(&mipi_frame);
        } else {
                switch (lcdc->conf->drv->hw_init.phy_type) {
                case HW_LCDC_PHY_MIPI_DBIB:
                case HW_LCDC_PHY_QUAD_SPI:
                case HW_LCDC_PHY_DUAL_SPI:
                case HW_LCDC_PHY_MIPI_SPI3:
                case HW_LCDC_PHY_MIPI_SPI4:
                        ret = ad_lcdc_mipi_set_position(handle, &mipi_frame);
                        break;
                default:
                        break;
                }
        }

        lcdc->data->frame_valid = !((frame->startx == 0) && (frame->starty == 0)
                && (frame->endx == lcdc->conf->drv->hw_init.resx - 1)
                && (frame->endy == lcdc->conf->drv->hw_init.resy - 1));

        lcdc->data->frame = *frame;

        OS_MUTEX_PUT(lcdc->busy);

        return ret;
}

int ad_lcdc_exit_partial_update(ad_lcdc_handle_t handle)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        hw_lcdc_frame_t frame = {
                .startx = 0,
                .starty = 0,
                .endx = lcdc->conf->drv->hw_init.resx - 1,
                .endy = lcdc->conf->drv->hw_init.resy - 1,
        };

        return ad_lcdc_set_partial_update(lcdc, &frame);
}

int ad_lcdc_set_display_offset(ad_lcdc_handle_t handle, int16_t offsetx, int16_t offsety)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        lcdc->data->disp_offsetx = offsetx;
        lcdc->data->disp_offsety = offsety;

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}


int ad_lcdc_setup_layer(ad_lcdc_handle_t handle, HW_LCDC_LAYER layer_no, bool enable, const hw_lcdc_layer_t *layer)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        if (!lcdc->cont_mode_active) {
                OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);
                /* Check if LCDC HW driver is already in use */
                if (hw_lcdc_is_busy()) {
                        OS_MUTEX_PUT(lcdc->busy);
                        OS_ASSERT(0);
                        return AD_LCDC_ERROR_CONTROLLER_BUSY;
                }

                /* Store layer info */
                lcdc->data->layer_en[layer_no] = enable;
                if (enable) {
                        lcdc->data->layer[layer_no] = *layer;
                }

                hw_lcdc_set_layer(layer_no, enable, layer);

                OS_MUTEX_PUT(lcdc->busy);
        } else {
                uint32_t ulPreviousMask = 0;
                if (in_interrupt()) {
                        OS_ENTER_CRITICAL_SECTION_FROM_ISR(ulPreviousMask);
                } else {
                        OS_ENTER_CRITICAL_SECTION();
                }

                lcdc->data->layer_en[layer_no] = enable;
                if (enable) {
                        lcdc->data->layer[layer_no] = *layer;
                }

                if (in_interrupt()) {
                        OS_LEAVE_CRITICAL_SECTION_FROM_ISR(ulPreviousMask);
                } else {
                        OS_LEAVE_CRITICAL_SECTION();
                }

                hw_lcdc_set_layer(layer_no, enable, layer);
        }
        return AD_LCDC_ERROR_NONE;
}

/**
 * \brief Setup callback parameters and trigger a single frame transmission
 *
 * \param [in] cb               Callback to call (from ISR context)
 * \param [in] ud               User data to pass to \p cb
 */
static void ad_lcdc_send_one_frame(hw_lcdc_callback cb, void *ud)
{
        hw_lcdc_set_callback(cb, ud);
        hw_lcdc_enable_frame_end_irq(true);
        hw_lcdc_send_one_frame();
}

/**
 * \brief Enable tearing effect detection and setup corresponding callback.
 *
 * \param [in] polarity         TE detection polarity
 * \param [in] cb               Callback to call (from ISR context)
 * \param [in] ud               User data to pass to \p cb
 */
static void ad_lcdc_enable_tearing(HW_LCDC_TE mode, hw_lcdc_callback cb, void *ud)
{
        hw_lcdc_set_callback(cb, ud);
        hw_lcdc_set_tearing_effect(true, mode);
        hw_lcdc_enable_tearing_effect_irq(true);
}

/**
 * \brief Callback function, called when the transmission in blocking mode is complete.
 *
 * \param [in] status           Error indication of frame transfer
 * \param [in] user_data        User data provided when the corresponding _async function was called
 */
static void ad_lcdc_wait_event(HW_LCDC_ERR status, void *user_data)
{
        lcdc_cb_data_t *cb_data = (lcdc_cb_data_t *)user_data;
        hw_lcdc_enable_frame_end_irq(false);

        cb_data->status = status;

        OS_EVENT_SIGNAL_FROM_ISR(cb_data->lcdc->event);
}

/**
 * \brief Callback function, called on TE signal detection to initiate frame transmission.
 *
 * \note Function is used when in blocking mode transmission.
 *
 * \param [in] status           Error indication of frame transfer
 * \param [in] user_data        Data provided when the corresponding _async function was called
 */
static void ad_lcdc_tearing_callback(HW_LCDC_ERR status, void *user_data)
{
        lcdc_cb_data_t *cb_data = (lcdc_cb_data_t *)user_data;

        UNUSED_ARG(status);

        hw_lcdc_enable_tearing_effect_irq(false);
        hw_lcdc_set_tearing_effect(false, cb_data->lcdc->conf->drv->te_mode);

        ad_lcdc_send_one_frame(ad_lcdc_wait_event, cb_data);
}

int ad_lcdc_draw_screen(ad_lcdc_handle_t handle, OS_TICK_TIME timeout)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        int ret;
        lcdc_cb_data_t cb_data = { .lcdc = lcdc };

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        if (lcdc->conf->drv->te_enable) {
                {
                        ad_lcdc_enable_tearing(lcdc->conf->drv->te_mode, ad_lcdc_tearing_callback, &cb_data);
                }
        }
        else {
                ad_lcdc_send_one_frame(ad_lcdc_wait_event, &cb_data);
        }

        ret = OS_EVENT_WAIT(lcdc->event, timeout);

        ret = ret != OS_EVENT_SIGNALED ?
                                         AD_LCDC_ERROR_TIMEOUT :
                                         ad_lcdc_error_translate(handle, cb_data.status);

        OS_MUTEX_PUT(lcdc->busy);

        return ret;
}

/**
 * \brief Callback function, called when the transmission in async mode is complete.
 *
 * \param [in] status           Error indication of frame transfer
 * \param [in] user_data        Data provided when the corresponding _async function was called
 */
static void ad_lcdc_async_callback(HW_LCDC_ERR status, void *user_data)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)user_data;
        ad_lcdc_user_cb cb = lcdc->callback;
        void * cb_data = lcdc->callback_data;

        lcdc->callback = NULL;
        lcdc->callback_data = NULL;

        hw_lcdc_enable_frame_end_irq(false);

        /* A not NULL callback must be registered before starting a transaction */
        OS_ASSERT(cb != NULL);

        if (cb) {
                cb(ad_lcdc_error_translate(lcdc, status), cb_data);
        }
}

/**
 * \brief Callback function, called on TE signal detection to initiate frame transmission.
 *
 * \note Function is used when in async mode transmission.
 *
 * \param [in] status           Error indication of frame transfer
 * \param [in] user_data        Data provided when the corresponding _async function was called
 */
static void ad_lcdc_tearing_async_callback(HW_LCDC_ERR status, void *user_data)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)user_data;

        UNUSED_ARG(status);

        hw_lcdc_enable_tearing_effect_irq(false);
        hw_lcdc_set_tearing_effect(false, lcdc->conf->drv->te_mode);

        ad_lcdc_send_one_frame(ad_lcdc_async_callback, lcdc);
}

int ad_lcdc_draw_screen_async(ad_lcdc_handle_t handle, ad_lcdc_user_cb cb, void *user_data)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);
        /* Check if LCDC HW driver is already in use */
        if (hw_lcdc_is_busy()) {
                OS_MUTEX_PUT(lcdc->busy);
                OS_ASSERT(0);
                return AD_LCDC_ERROR_CONTROLLER_BUSY;
        }

        lcdc->callback = cb;
        lcdc->callback_data = user_data;

        if (lcdc->conf->drv->te_enable) {
                {
                        ad_lcdc_enable_tearing(lcdc->conf->drv->te_mode,
                                ad_lcdc_tearing_async_callback, lcdc);
                }
        }
        else {
                ad_lcdc_send_one_frame(ad_lcdc_async_callback, lcdc);
        }

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}

/**
 * \brief Callback function used in continuous mode frame update
 *
 * \param [in] status           Error indication of current frame
 * \param [in] handle           LCDC handle returned by ad_lcdc_open()
 */
static void continuous_mode_callback(HW_LCDC_ERR status, ad_lcdc_handle_t handle)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        ad_lcdc_user_cb cb = lcdc->callback;
        void *user_data = lcdc->callback_data;

        /* A not NULL callback must be registered before starting a transaction */
        OS_ASSERT(cb != NULL);

        if (cb) {
                cb(ad_lcdc_error_translate(lcdc, status), user_data);
        }
}

int ad_lcdc_continuous_update_start(ad_lcdc_handle_t handle, ad_lcdc_user_cb cb, void *user_data)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        /* Continuous mode cannot use tearing effect */
        OS_ASSERT(!lcdc->conf->drv->te_enable);

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        lcdc->callback = cb;
        lcdc->callback_data = user_data;

        hw_lcdc_set_callback(continuous_mode_callback, lcdc);

        hw_lcdc_set_continuous_mode(true);
        hw_lcdc_enable_frame_end_irq(true);

        lcdc->cont_mode_active = true;

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}

int ad_lcdc_continuous_update_stop(ad_lcdc_handle_t handle)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        lcdc->cont_mode_active = false;

        hw_lcdc_enable_frame_end_irq(false);
        hw_lcdc_set_continuous_mode(false);

        hw_lcdc_set_callback(NULL, NULL);

        lcdc->callback = NULL;
        lcdc->callback_data = NULL;

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}

ADAPTER_INIT(ad_lcdc_adapter, ad_lcdc_init);

#endif /* dg_configLCDC_ADAPTER */
