/**
 *****************************************************************************************
 *
 * @file hw_lcdc.c
 *
 * @brief Implementation of the LCD Controller Low Level Driver.
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#if dg_configUSE_HW_LCDC

#include <sdk_defs.h>
#include "hw_lcdc.h"
#include "hw_clk.h"
#include "hw_pmu.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

/**
 * \brief LCDC HW ID register value
 */
#define LCDC_MAGIC                      (0x87452365UL)

#define PHY_CFG_DEFAULT                 ( HW_LCDC_MIPI_CFG_RESX | \
                                          HW_LCDC_MIPI_CFG_TE_DIS )

#define JDI_SERIAL_CFG_DEFAULT          ( HW_LCDC_MIPI_CFG_DBI_EN    | \
                                          HW_LCDC_MIPI_CFG_SPI4      | \
                                          HW_LCDC_MIPI_CFG_SPI_JDI   | \
                                          HW_LCDC_MIPI_CFG_SPIX_REV  | \
                                          HW_LCDC_MIPI_CFG_SCAN_ADDR | \
                                          HW_LCDC_MIPI_CFG_SPI_HOLD  | \
                                          HW_LCDC_MIPI_CFG_SPI_CSX_V )

#define MIN_PART_UPDATE_WIDTH           (2U)
#define FPX_MOD                         (0)
#define BPX_MOD                         (0)


/**
 * \brief Minimum timing parameters
 *
 * Used for all interfaces that do not have specific timing requirements, i.e. serial interfaces
 */
#define MIN_BLX                         (2U)
#define MIN_BLY                         (1U)
#define MIN_FPX                         (1U)
#define MIN_FPY                         (1U)
#define MIN_BPX                         (1U)
#define MIN_BPY                         (1U)

/**
 * \brief Helper macros to calculate fractions by rounding
 *
 * \ref ROUND_UP rounds the result toward the higher value integer and \ref ROUND_TO_NEAREST to the
 * nearest integer
 */
#define ROUND_UP(numerator, denominator)         (((numerator) + ((denominator) - 1)) / (denominator))
#define ROUND_TO_NEAREST(numerator, denominator) (((numerator) + ((denominator) / 2)) / (denominator))

/**
 * \brief Macro that waits until condition is met or time out is reached
 */
#define WAIT_TIMEOUT(condition, timeout_us)     {                                                  \
                                                        int _timeout_us = (timeout_us);            \
                                                        while ((!(condition)) && _timeout_us > 0) {\
                                                                hw_clk_delay_usec(5);              \
                                                                _timeout_us -= 5;                  \
                                                        }                                          \
                                                }

/**
 * \brief Interrupt number and interrupt handler definitions
 */
#define HW_LCDC_IRQn            (LCD_IRQn)
#define HW_LCDC_Handler         LCD_Handler

/**
 * \brief LCD Controller low level driver internal data
 */
typedef struct {
        const hw_lcdc_config_t *lcdc;                           //!< LCDC active configuration
        hw_lcdc_callback        cb;                             //!< User callback function
        void                   *cb_data;                        //!< User callback data
        hw_lcdc_frame_t         active_area;                    //!< Active area of the LCD that is updated
        HW_LCDC_PHY             phy;                            //!< Physical connection type
        HW_LCDC_JDIS_CMD        jdis_update_cmd;                //!< JDI/Sharp update/refresh command
        HW_LCDC_QSPI_MODE       qspi_mode_config;               //!< Active QSPI mode configuration
        HW_LCDC_BLEND_MODE      blendmode[HW_LCDC_LAYER_MAX];   //!< Layer blend mode setting to be set when in continuous mode
        uint8_t                 alpha[HW_LCDC_LAYER_MAX];       //!< Layer global alpha setting to be set when in continuous mode
        int16_t                 stride[HW_LCDC_LAYER_MAX];      //!< Layer stride setting to be set when in continuous mode
        bool                    layer_dirty[HW_LCDC_LAYER_MAX]; //!< Indication if a layer parameter change needs to be applied when in continuous mode
} LCDC_Data;

/**
 * \brief LCD Controller low level driver internal data
 *
 * \warning LCD Controller data are not retained. The user must ensure that they are updated after
 * exiting sleep.
 */
static LCDC_Data lcdc_data;

/**
 * \brief Array of each prefetch level in bytes
 */
static const uint8_t hw_lcdc_prefetch_value[] = {
        [HW_LCDC_FIFO_PREFETCH_LVL_DISABLED] = 0,
        [HW_LCDC_FIFO_PREFETCH_LVL_1] = 52,
        [HW_LCDC_FIFO_PREFETCH_LVL_2] = 84,
        [HW_LCDC_FIFO_PREFETCH_LVL_4] = 108,
        [HW_LCDC_FIFO_PREFETCH_LVL_3] = 116,
};

/**
 * \brief Array of prefetch setting options in increasing order
 */
static const HW_LCDC_FIFO_PREFETCH_LVL hw_lcdc_prefetch_level[] = {
        HW_LCDC_FIFO_PREFETCH_LVL_DISABLED,
        HW_LCDC_FIFO_PREFETCH_LVL_1,
        HW_LCDC_FIFO_PREFETCH_LVL_2,
        HW_LCDC_FIFO_PREFETCH_LVL_4,
        HW_LCDC_FIFO_PREFETCH_LVL_3,
};

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 * \name                Register functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Set display resolution
 *
 * \param [in] x                Resolution X in pixels
 * \param [in] y                Resolution Y in pixels
 */
__STATIC_INLINE void set_resolution(uint16_t x, uint16_t y)
{
        uint32_t lcdc_resxy_reg = 0;
        HW_LCDC_REG_SET_FIELD(LCDC_RESXY_REG, RES_X, lcdc_resxy_reg, x);
        HW_LCDC_REG_SET_FIELD(LCDC_RESXY_REG, RES_Y, lcdc_resxy_reg, y);
        LCDC->LCDC_RESXY_REG = lcdc_resxy_reg;
}

/**
 * \brief Get display resolution
 *
 * \param [out] x               Resolution X in pixels
 * \param [out] y               Resolution Y in pixels
 */
__STATIC_INLINE void get_resolution(uint16_t *x, uint16_t *y)
{
        uint32_t lcdc_resxy_reg = LCDC->LCDC_RESXY_REG;
        *x = HW_LCDC_REG_GET_FIELD(LCDC_RESXY_REG, RES_X, lcdc_resxy_reg);
        *y = HW_LCDC_REG_GET_FIELD(LCDC_RESXY_REG, RES_Y, lcdc_resxy_reg);
}

/**
 * \brief Set front porch settings
 *
 * \param [in] x                Front porch X (lines)
 * \param [in] y                Front porch Y (pixel clocks)
 */
__STATIC_INLINE void set_front_porch(uint16_t x, uint16_t y)
{
        uint32_t lcdc_frontporchxy_reg = 0;
        HW_LCDC_REG_SET_FIELD(LCDC_FRONTPORCHXY_REG, FPORCH_X, lcdc_frontporchxy_reg, x);
        HW_LCDC_REG_SET_FIELD(LCDC_FRONTPORCHXY_REG, FPORCH_Y, lcdc_frontporchxy_reg, y);
        LCDC->LCDC_FRONTPORCHXY_REG = lcdc_frontporchxy_reg;
}

/**
 * \brief Get front porch settings
 *
 * \param [out] x               Front porch X (lines)
 * \param [out] y               Front porch Y (pixel clocks)
 */
__STATIC_INLINE void get_front_porch(uint16_t *x, uint16_t *y)
{
        uint32_t lcdc_frontporchxy_reg = LCDC->LCDC_FRONTPORCHXY_REG;
        *x = HW_LCDC_REG_GET_FIELD(LCDC_FRONTPORCHXY_REG, FPORCH_X, lcdc_frontporchxy_reg);
        *y = HW_LCDC_REG_GET_FIELD(LCDC_FRONTPORCHXY_REG, FPORCH_Y, lcdc_frontporchxy_reg);
}

/**
 * \brief Set blanking period
 *
 * \param [in] x                Blanking period X (VSYNC lines)
 * \param [in] y                Blanking period Y (HSYNC pulse length)
 */
__STATIC_INLINE void set_blanking(uint16_t x, uint16_t y)
{
        uint32_t lcdc_blankingxy_reg = 0;
        HW_LCDC_REG_SET_FIELD(LCDC_BLANKINGXY_REG, BLANKING_X, lcdc_blankingxy_reg, x);
        HW_LCDC_REG_SET_FIELD(LCDC_BLANKINGXY_REG, BLANKING_Y, lcdc_blankingxy_reg, y);
        LCDC->LCDC_BLANKINGXY_REG = lcdc_blankingxy_reg;
}

/**
 * \brief Get blanking period
 *
 * \param [out] x               Blanking period X (VSYNC lines)
 * \param [out] y               Blanking period Y (HSYNC pulse length)
 */
__STATIC_INLINE void get_blanking(uint16_t *x, uint16_t *y)
{
        uint32_t lcdc_blankingxy_reg = LCDC->LCDC_BLANKINGXY_REG;
        *x = HW_LCDC_REG_GET_FIELD(LCDC_BLANKINGXY_REG, BLANKING_X, lcdc_blankingxy_reg);
        *y = HW_LCDC_REG_GET_FIELD(LCDC_BLANKINGXY_REG, BLANKING_Y, lcdc_blankingxy_reg);
}

/**
 * \brief Set back porch settings
 *
 * \param [in] x                Back porch X (lines)
 * \param [in] y                Back porch Y (pixel clocks)
 */
__STATIC_INLINE void set_back_porch(uint16_t x, uint16_t y)
{
        uint32_t lcdc_backporchxy_reg = 0;
        HW_LCDC_REG_SET_FIELD(LCDC_BACKPORCHXY_REG, BPORCH_X, lcdc_backporchxy_reg, x);
        HW_LCDC_REG_SET_FIELD(LCDC_BACKPORCHXY_REG, BPORCH_Y, lcdc_backporchxy_reg, y);
        LCDC->LCDC_BACKPORCHXY_REG = lcdc_backporchxy_reg;
}

/**
 * \brief Get back porch settings
 *
 * \param [out] x               Back porch X (lines)
 * \param [out] y               Back porch Y (pixel clocks)
 */
__STATIC_INLINE void get_back_porch(uint16_t *x, uint16_t *y)
{
        uint32_t lcdc_backporchxy_reg = LCDC->LCDC_BACKPORCHXY_REG;
        *x = HW_LCDC_REG_GET_FIELD(LCDC_BACKPORCHXY_REG, BPORCH_X, lcdc_backporchxy_reg);
        *y = HW_LCDC_REG_GET_FIELD(LCDC_BACKPORCHXY_REG, BPORCH_Y, lcdc_backporchxy_reg);
}

/**
 * \brief Set frame start settings
 *
 * \param [in] x                Frame start X (lines)
 * \param [in] y                Frame start Y (pixel clocks)
 */
__STATIC_INLINE void set_frame_start(uint16_t x, uint16_t y)
{
        uint32_t lcdc_startxy_reg = 0;
        HW_LCDC_REG_SET_FIELD(LCDC_STARTXY_REG, START_X, lcdc_startxy_reg, x);
        HW_LCDC_REG_SET_FIELD(LCDC_STARTXY_REG, START_Y, lcdc_startxy_reg, y);
        LCDC->LCDC_STARTXY_REG = lcdc_startxy_reg;
}

/**
 * \brief Get back porch settings
 *
 * \param [out] x               Frame start X (lines)
 * \param [out] y               Frame start Y (pixel clocks)
 */
__STATIC_INLINE void get_frame_start(uint16_t *x, uint16_t *y)
{
        uint32_t lcdc_startxy_reg = LCDC->LCDC_STARTXY_REG;
        *x = HW_LCDC_REG_GET_FIELD(LCDC_STARTXY_REG, START_X, lcdc_startxy_reg);
        *y = HW_LCDC_REG_GET_FIELD(LCDC_STARTXY_REG, START_Y, lcdc_startxy_reg);
}

/**
 * \brief Set layer global alpha value
 *
 * \param [in] layer            Layer index
 * \param [in] blendmode        Blend mode of specific layer with previous layer or background color
 * \param [in] alpha            Global alpha value of layer
 */
__STATIC_INLINE void set_layer_blend_mode(HW_LCDC_LAYER layer, HW_LCDC_BLEND_MODE blendmode,uint8_t alpha)
{
        ASSERT_WARNING(layer < HW_LCDC_LAYER_MAX);
        uint32_t lcdc_layer_mode_reg = HW_LCDC_GET_LAYER_REG(LCDC_LAYER0_MODE_REG, layer);
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_MODE_REG, L0_DST_BLEND, lcdc_layer_mode_reg, blendmode >> 4);
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_MODE_REG, L0_SRC_BLEND, lcdc_layer_mode_reg, blendmode);
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_MODE_REG, L0_ALPHA, lcdc_layer_mode_reg, alpha);
        HW_LCDC_SET_LAYER_REG(LCDC_LAYER0_MODE_REG, layer, lcdc_layer_mode_reg);
}

/**
 * \brief Set layer mode settings
 *
 * \param [in] layer            Layer index
 * \param [in] enable           Enable/disable layer
 * \param [in] color            Color mode
 */
__STATIC_INLINE void set_layer_mode(HW_LCDC_LAYER layer, bool enable, HW_LCDC_LAYER_COLOR_MODE color)
{
        ASSERT_WARNING(layer < HW_LCDC_LAYER_MAX);
        uint32_t lcdc_layer_mode_reg = HW_LCDC_GET_LAYER_REG(LCDC_LAYER0_MODE_REG, layer);
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_MODE_REG, L0_COLOR_MODE, lcdc_layer_mode_reg, color);
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_MODE_REG, L0_EN, lcdc_layer_mode_reg, enable ? 1 : 0);
        HW_LCDC_SET_LAYER_REG(LCDC_LAYER0_MODE_REG, layer, lcdc_layer_mode_reg);
}

/**
 * \brief Set layer start (offset in pixels)
 *
 * \param [in] layer            Layer index
 * \param [in] x                Start X in pixels
 * \param [in] y                Start Y in pixels
 *
 * \warning Register value changes will have no effect in the currently generated frame.
 */
__STATIC_INLINE void set_layer_start(HW_LCDC_LAYER layer, int16_t x, int16_t y)
{
        ASSERT_WARNING(layer < HW_LCDC_LAYER_MAX);
        uint32_t lcdc_layer_startxy_reg = 0;
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_STARTXY_REG, L0_START_X, lcdc_layer_startxy_reg, x);
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_STARTXY_REG, L0_START_Y, lcdc_layer_startxy_reg, y);
        HW_LCDC_SET_LAYER_REG(LCDC_LAYER0_STARTXY_REG, layer, lcdc_layer_startxy_reg);
}

/**
 * \brief Set layer size in pixels
 *
 * \param [in] layer            Layer index
 * \param [in] x                Size X in pixels
 * \param [in] y                Size Y in pixels
 *
 * \warning Register value changes will have no effect in the currently generated frame.
 */
__STATIC_INLINE void set_layer_size(HW_LCDC_LAYER layer, uint16_t x, uint16_t y)
{
        ASSERT_WARNING(layer < HW_LCDC_LAYER_MAX);
        uint32_t lcdc_layer_sizexy_reg = 0;
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_SIZEXY_REG, L0_SIZE_X, lcdc_layer_sizexy_reg, x);
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_SIZEXY_REG, L0_SIZE_Y, lcdc_layer_sizexy_reg, y);
        HW_LCDC_SET_LAYER_REG(LCDC_LAYER0_SIZEXY_REG, layer, lcdc_layer_sizexy_reg);
}

/**
 * \brief Set layer base address
 *
 * \param [in] layer            Layer index
 * \param [in] addr             Base address
 *
 * \warning Register value changes will have no effect in the currently generated frame.
 */
__STATIC_INLINE void set_layer_base_addr(HW_LCDC_LAYER layer, uint32_t addr)
{
        ASSERT_WARNING(layer < HW_LCDC_LAYER_MAX);
       HW_LCDC_SETF_LAYER_REG(LCDC_LAYER0_BASEADDR_REG, L0_BASE_ADDR, addr, layer);
}

/**
 * \brief Set layer stride (distance from line to line in bytes)
 *
 * \param [in] layer            Layer index
 * \param [in] stride           Distance in bytes between consecutive lines
 */
__STATIC_INLINE void set_layer_stride(HW_LCDC_LAYER layer, int16_t stride)
{
        ASSERT_WARNING(layer < HW_LCDC_LAYER_MAX);
        HW_LCDC_SETF_LAYER_REG(LCDC_LAYER0_STRIDE_REG, L0_STRIDE, stride, layer);
}

/**
 * \brief Set layer resolution in pixels
 *
 * \param [in] layer            Layer index
 * \param [in] x                Resolution X in pixels
 * \param [in] y                Resolution Y in pixels
 *
 * \warning Register value changes will have no effect in the currently generated frame.
 */
__STATIC_INLINE void set_layer_resolution(HW_LCDC_LAYER layer, uint16_t x, uint16_t y)
{
        ASSERT_WARNING(layer < HW_LCDC_LAYER_MAX);
        uint32_t lcdc_layer_resxy_reg = 0;
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_RESXY_REG, L0_RES_X, lcdc_layer_resxy_reg, x);
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_RESXY_REG, L0_RES_Y, lcdc_layer_resxy_reg, y);
        HW_LCDC_SET_LAYER_REG(LCDC_LAYER0_RESXY_REG, layer, lcdc_layer_resxy_reg);
}


/**
 * \brief Set layer DMA prefetch level
 *
 * \param [in] layer            Layer index
 * \param [in] level            FIFO pre-fetch level as defined in \sa HW_LCDC_FIFO_PREFETCH_LVL
 *
 * \warning Register value changes will have no effect in the currently generated frame.
 */
__STATIC_INLINE void set_layer_dma_prefetch(HW_LCDC_LAYER layer, HW_LCDC_FIFO_PREFETCH_LVL level)
{
        ASSERT_WARNING(layer < HW_LCDC_LAYER_MAX);
        uint32_t lcdc_layer_stride_reg = HW_LCDC_GET_LAYER_REG(LCDC_LAYER0_STRIDE_REG, layer);
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_STRIDE_REG, L0_DMA_PREFETCH, lcdc_layer_stride_reg, level);
        HW_LCDC_SET_LAYER_REG(LCDC_LAYER0_STRIDE_REG, layer, lcdc_layer_stride_reg);
}
/** \} */

/**
 * \name                Display controller functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Reverse a byte MSB to LSB wise
 *
 * \param [in] val              Byte to be reversed
 *
 * \return The reversed byte
 */
__STATIC_INLINE uint8_t byte_reverse(uint8_t val)
{
        return __RBIT(val) >> 24;
}

uint32_t hw_lcdc_stride_size(HW_LCDC_LAYER_COLOR_MODE format, uint16_t width)
{
        uint32_t stride;
        switch (format) {
        default:
                stride = width * hw_lcdc_lcm_size(format);
                break;
        }
        return (stride + 3UL) & (~3UL);
}

/**
 * \brief Enables the LCD controller and configures clock source
 *
 * \param [in] phy              Configured PHY interface
 * \param [in] iface_freq       Configured interface frequency
 */
static void hw_lcdc_enable(HW_LCDC_PHY phy, HW_LCDC_FREQ iface_freq)
{
        uint32_t clk_sys_reg;
        uint32_t div = 0;
        bool src_div1 = (iface_freq & (HW_LCDC_CLK_PLL_BIT | HW_LCDC_CLK_RCHS_BIT)) ? true : false;

        div = iface_freq & ~(HW_LCDC_CLK_PLL_BIT | HW_LCDC_CLK_RCHS_BIT);
        div >>= (phy == HW_LCDC_PHY_DPI) ? 0 : 1;


        clk_sys_reg = REG_MSK(CRG_SYS, RESET_CLK_SYS_REG, LCD_CLK_SEL);
        clk_sys_reg |= REG_MSK(CRG_SYS, RESET_CLK_SYS_REG, LCD_ENABLE);
        CRG_SYS->RESET_CLK_SYS_REG = clk_sys_reg;

        clk_sys_reg = 0;
        REG_SET_FIELD(CRG_SYS, CLK_SYS_REG, LCD_CLK_SEL, clk_sys_reg, src_div1 ? 1 : 0);
        REG_SET_FIELD(CRG_SYS, CLK_SYS_REG, LCD_ENABLE, clk_sys_reg, 1);
        CRG_SYS->SET_CLK_SYS_REG = clk_sys_reg;


        hw_lcdc_set_iface_clk(MIN(div, HW_LCDC_CLK_DIV_MSK));
}

/**
 * \brief Disables LCD controller
 *
 * \param [in] phy              Previously configured PHY
 */
static void hw_lcdc_disable(HW_LCDC_PHY phy)
{
        uint32_t clk_sys_reg;

        CRG_SYS->SET_CLK_SYS_REG = REG_MSK(CRG_SYS, SET_CLK_SYS_REG, LCD_RESET_REQ);

        clk_sys_reg = REG_MSK(CRG_SYS, RESET_CLK_SYS_REG, LCD_RESET_REQ);
        clk_sys_reg |= REG_MSK(CRG_SYS, RESET_CLK_SYS_REG, LCD_CLK_SEL);
        clk_sys_reg |= REG_MSK(CRG_SYS, RESET_CLK_SYS_REG, LCD_ENABLE);
        CRG_SYS->RESET_CLK_SYS_REG = clk_sys_reg;
}

HW_LCDC_ERR hw_lcdc_init(const hw_lcdc_config_t *cfg)
{
        HW_LCDC_MODE mode = HW_LCDC_MODE_DISABLE;
        HW_LCDC_MIPI_CFG config = 0;
        HW_LCDC_OUTPUT_COLOR_MODE format;
        HW_LCDC_PHY entry_phy = lcdc_data.phy;

        if (!cfg) {
                return HW_LCDC_ERR_CONF_INVALID;
        }

        hw_lcdc_display_t lcd_timing = {
                .resx = cfg->resx, .resy = cfg->resy,
                .blx  = MIN_BLX,   .bly  = MIN_BLY,
                .fpx  = MIN_FPX,   .fpy  = MIN_FPY,
                .bpx  = MIN_BPX,   .bpy  = MIN_BPY,
        };

        if (cfg->phy_type == HW_LCDC_PHY_NONE) {
                NVIC_DisableIRQ(HW_LCDC_IRQn);
                NVIC_ClearPendingIRQ(HW_LCDC_IRQn);
                lcdc_data.lcdc = NULL;
                hw_lcdc_disable(entry_phy);
                return HW_LCDC_ERR_NONE;
        }

        if (lcdc_data.lcdc != NULL) {
                return HW_LCDC_ERR_UNSUPPORTED;
        }

        lcdc_data.lcdc = cfg;

        hw_lcdc_enable(cfg->phy_type, cfg->write_freq);

        if (hw_lcdc_get_id() != LCDC_MAGIC) {
                lcdc_data.lcdc = NULL;
                hw_lcdc_disable(entry_phy);
                return HW_LCDC_ERR_CONF_INVALID;
        }

        hw_lcdc_set_mode(HW_LCDC_MODE_DISABLE);

        HW_LCDC_REG_SETF(LCDC_INTERRUPT_REG, IRQ_TRIGGER_SEL, 1);
        hw_lcdc_enable_vsync_irq(false);
        NVIC_EnableIRQ(HW_LCDC_IRQn);

        format = cfg->format;
        lcdc_data.phy = cfg->phy_type;

        /* Enable underrun protection mechanism */
        mode |= HW_LCDC_MODE_UDERRUN_PREV;

        switch (cfg->phy_type) {
        case HW_LCDC_PHY_MIPI_DBIB:
                config = PHY_CFG_DEFAULT | HW_LCDC_MIPI_CFG_DBI_EN;
                config |= HW_LCDC_DBI_INTERFACE_WIDTH_DBIB_8 << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_INTERFACE_WIDTH);
                format &= ~HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DBIB_DATA_ORDER);
                hw_lcdc_set_iface(HW_LCDC_GPIO_IF_DBIB);
                break;
        case HW_LCDC_PHY_QUAD_SPI:
                config = PHY_CFG_DEFAULT | HW_LCDC_MIPI_CFG_DBI_EN | HW_LCDC_MIPI_CFG_SPI4 | HW_LCDC_MIPI_CFG_QSPI | HW_LCDC_MIPI_CFG_SPIDC_DQSPI;
                config |= HW_LCDC_DBI_INTERFACE_WIDTH_QSPI << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_INTERFACE_WIDTH);
                format &= ~HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DBIB_DATA_ORDER);
                hw_lcdc_set_iface(HW_LCDC_GPIO_IF_SPI);
                lcdc_data.qspi_mode_config = HW_LCDC_QSPI_MODE_AUTO;
                break;
        case HW_LCDC_PHY_DUAL_SPI:
                config = PHY_CFG_DEFAULT | HW_LCDC_MIPI_CFG_DBI_EN | HW_LCDC_MIPI_CFG_SPI_CPHA | HW_LCDC_MIPI_CFG_SPI_CPOL;
                config |= cfg->iface_conf.dspi.spi3 ? HW_LCDC_MIPI_CFG_SPI3 : HW_LCDC_MIPI_CFG_SPI4;
                config |= HW_LCDC_DBI_INTERFACE_WIDTH_DSPI << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_INTERFACE_WIDTH);
                config |= cfg->iface_conf.dspi.option << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_DATA_ORDER);
                config |= cfg->iface_conf.dspi.option == HW_LCDC_DSPI_OPT_2P3T2 ? HW_LCDC_MIPI_CFG_DSPI_SPIX : 0;
                format &= ~HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DBIB_DATA_ORDER);
                hw_lcdc_set_iface(HW_LCDC_GPIO_IF_SPI);
                break;
        case HW_LCDC_PHY_MIPI_SPI3:
                config = PHY_CFG_DEFAULT | HW_LCDC_MIPI_CFG_SPI3;
                config |= HW_LCDC_MIPI_CFG_DBI_EN | HW_LCDC_MIPI_CFG_SPI_CPHA | HW_LCDC_MIPI_CFG_SPI_CPOL;
                config |= HW_LCDC_DBI_INTERFACE_WIDTH_SPI << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_INTERFACE_WIDTH);
                hw_lcdc_set_iface(HW_LCDC_GPIO_IF_SPI);
                break;
        case HW_LCDC_PHY_MIPI_SPI4:
                config = PHY_CFG_DEFAULT | HW_LCDC_MIPI_CFG_SPI4;
                config |= HW_LCDC_MIPI_CFG_DBI_EN | HW_LCDC_MIPI_CFG_SPI_CPHA | HW_LCDC_MIPI_CFG_SPI_CPOL;
                config |= HW_LCDC_DBI_INTERFACE_WIDTH_SPI << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_INTERFACE_WIDTH);
                hw_lcdc_set_iface(HW_LCDC_GPIO_IF_SPI);
                break;
        case HW_LCDC_PHY_JDI_SPI:
                config = PHY_CFG_DEFAULT | JDI_SERIAL_CFG_DEFAULT;
                config |= HW_LCDC_DBI_INTERFACE_WIDTH_SPI << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_INTERFACE_WIDTH);
                switch (format) {
                case HW_LCDC_OCM_8RGB111_2:
                        lcdc_data.jdis_update_cmd = HW_LCDC_JDIS_CMD_UPDATE_4BIT;
                        break;
                case HW_LCDC_OCM_RGB111:
                        lcdc_data.jdis_update_cmd = HW_LCDC_JDIS_CMD_UPDATE_NATIVE;
                        break;
                case HW_LCDC_OCM_L1:
                default:
                        lcdc_data.jdis_update_cmd = HW_LCDC_JDIS_CMD_UPDATE_1BIT;
                        break;
                }
                hw_lcdc_set_iface(HW_LCDC_GPIO_IF_SPI);
                break;
        case HW_LCDC_PHY_SHARP_SPI:
                config = PHY_CFG_DEFAULT | JDI_SERIAL_CFG_DEFAULT | HW_LCDC_MIPI_CFG_INV_ADDR;
                config |= HW_LCDC_DBI_INTERFACE_WIDTH_SPI << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_INTERFACE_WIDTH);
                lcdc_data.jdis_update_cmd = HW_LCDC_JDIS_CMD_UPDATE_NATIVE;
                hw_lcdc_set_iface(HW_LCDC_GPIO_IF_SPI);
                break;
        case HW_LCDC_PHY_JDI_PARALLEL:
                config = PHY_CFG_DEFAULT;
                format &= ~HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DBIB_DATA_ORDER);

                lcd_timing.fpx = cfg->iface_conf.jdi_par.fpx;
                lcd_timing.blx = cfg->iface_conf.jdi_par.blx;
                lcd_timing.bpx = cfg->iface_conf.jdi_par.bpx;

                lcd_timing.fpy = cfg->iface_conf.jdi_par.fpy;
                lcd_timing.bly = cfg->iface_conf.jdi_par.bly;
                lcd_timing.bpy = cfg->iface_conf.jdi_par.bpy;

                mode = HW_LCDC_MODE_JDIMIP | HW_LCDC_MODE_SCANDOUBLE;
                hw_lcdc_jdi_parallel(cfg->resx, cfg->resy, &cfg->iface_conf.jdi_par);
                hw_lcdc_set_iface(HW_LCDC_GPIO_IF_JDI);

                break;
        case HW_LCDC_PHY_DPI:
                config = PHY_CFG_DEFAULT;
                mode = HW_LCDC_MODE_P_RGB | HW_LCDC_MODE_MIPI_OFF | HW_LCDC_MODE_NEG_H | HW_LCDC_MODE_NEG_V;
                format &= ~HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DBIB_DATA_ORDER);
                HW_LCDC_REG_SETF(LCDC_FMTCTRL_REG, JDIP_DPI_MASK_READY, cfg->iface_conf.dpi.enable_dpi_ready ? 0 : 1);
                lcd_timing.fpx = cfg->iface_conf.dpi.fpx + FPX_MOD;
                lcd_timing.blx = cfg->iface_conf.dpi.blx;
                lcd_timing.bpx = cfg->iface_conf.dpi.bpx + BPX_MOD;

                lcd_timing.fpy = cfg->iface_conf.dpi.fpy;
                lcd_timing.bly = cfg->iface_conf.dpi.bly;
                lcd_timing.bpy = cfg->iface_conf.dpi.bpy;

                hw_lcdc_set_iface(HW_LCDC_GPIO_IF_DPI);
                break;
        default:
                lcdc_data.lcdc = NULL;
                hw_lcdc_disable(entry_phy);
                return HW_LCDC_ERR_CONF_INVALID;
        }

        lcdc_data.cb = NULL;

        /* Apply dethering setting */
        HW_LCDC_REG_SET_FIELD(LCDC_MODE_REG, DITH_MODE, mode, cfg->dither);

        /* Modify predefined settings using the configuration parameters */
        config ^= cfg->cfg_extra_flags;

        hw_lcdc_set_mipi_cfg(config | format);

        hw_lcdc_set_mode(mode ^ cfg->mode);

        hw_lcdc_set_lcd_timing(&lcd_timing);

        return HW_LCDC_ERR_NONE;
}

void hw_lcdc_set_lcd_timing(const hw_lcdc_display_t *params)
{
        uint16_t dc_fpx, dc_fpy, dc_blx, dc_bly, dc_bpx, dc_bpy;
        uint16_t resx = params->resx;
        uint16_t resy = params->resy;

        lcdc_data.active_area.startx = 0;
        lcdc_data.active_area.starty = 0;
        lcdc_data.active_area.endx = resx - 1;
        lcdc_data.active_area.endy = resy - 1;

        switch (lcdc_data.phy) {
        case HW_LCDC_PHY_JDI_PARALLEL:
                resy *= 2;
                break;
        case HW_LCDC_PHY_JDI_SPI:
        case HW_LCDC_PHY_SHARP_SPI:
                /* Add an extra line at the end to produce the required dummy bytes */
                resy++;
                break;
        default:
                break;
        }

        dc_fpx = resx + params->fpx;
        dc_fpy = resy + params->fpy;
        dc_blx = dc_fpx + params->blx;
        dc_bly = dc_fpy + params->bly;
        dc_bpx = dc_blx + params->bpx;
        dc_bpy = dc_bly + params->bpy;

        set_resolution(resx, resy);
        set_front_porch(dc_fpx, dc_fpy);
        set_blanking(dc_blx, dc_bly);
        set_back_porch(dc_bpx, dc_bpy);
        set_frame_start(dc_fpx, dc_fpy - 1);
}

void hw_lcdc_set_update_region(hw_lcdc_frame_t *frame)
{
        int16_t modx, mody;
        uint16_t fpx, fpy;
        uint16_t blx, bly;
        uint16_t bpx, bpy;
        uint16_t resx, resy;
        uint16_t sx, sy;
        ASSERT_ERROR(frame->endx >= frame->startx);
        ASSERT_ERROR(frame->endy >= frame->starty);


        get_resolution(&resx, &resy);
        get_front_porch(&fpx, &fpy);
        get_blanking(&blx, &bly);
        get_back_porch(&bpx, &bpy);
        get_frame_start(&sx, &sy);

        uint16_t width = frame->endx - frame->startx + 1;

        /* If columns are less than minimum width, increase the update area */
        /* Firstly check how much can be increased on the left */
        if (width < MIN_PART_UPDATE_WIDTH) {
                uint16_t dec_startx = MIN(MIN_PART_UPDATE_WIDTH - width, frame->startx);
                frame->startx -= dec_startx;
                width += dec_startx;
        }
        /* If increase on the left not sufficient (too close to the border), increase the rest on
         * the right. No need to perform a limit check since we have reached the left border of the
         * screen */
        if (width < MIN_PART_UPDATE_WIDTH) {
                frame->endx += MIN_PART_UPDATE_WIDTH - width;
        }

        modx = frame->endx - lcdc_data.active_area.endx
                - (frame->startx - lcdc_data.active_area.startx);
        mody = frame->endy - lcdc_data.active_area.endy
                - (frame->starty - lcdc_data.active_area.starty);

        set_resolution(resx + modx, resy + mody);
        set_front_porch(fpx + modx, fpy + mody);
        set_blanking(blx + modx, bly + mody);
        set_back_porch(bpx + modx, bpy + mody);
        set_frame_start(sx + modx, sy + mody);

        lcdc_data.active_area = *frame;
}

/**
 * \brief Intersects the provided rectangulars
 *
 * \param [out] dst             Output of the calculated rectangular
 * \param [in]  src0            Input of the first rectangular
 * \param [in]  src1            Input of the second rectangular
 *
 * \return true if input rectangulars have a common area, false otherwise
 */
static bool hw_lcdc_intersect_rects(hw_lcdc_frame_t *dst, const hw_lcdc_frame_t *src0,
        const hw_lcdc_frame_t *src1)
{
        dst->startx = MAX(src0->startx, src1->startx);
        dst->starty = MAX(src0->starty, src1->starty);
        dst->endx = MIN(src0->endx, src1->endx);
        dst->endy = MIN(src0->endy, src1->endy);
        return !(dst->endx < dst->startx || dst->endy < dst->starty);
}

/**
 * \brief Returns the burst size according to the required size
 *
 * \param [in] size             Size in bytes to be fetched
 * \param [in] max_8_beats      Disables the use of 16 beat bursts
 *
 * \return burst size in bytes
 */
static int hw_lcdc_get_burst(int size, bool max_8_beats)
{
        if (size >= 64 && !max_8_beats) {
                return 64;
        } else if (size >= 32) {
                return 32;
        } else if (size >= 16) {
                return 16;
        } else {
                return size & ~0x3;
        }
}

/**
 * \brief Checks if provided layer settings can be supported by the prefetch level
 *
 * \param [in] width            Width in pixels of the layer
 * \param [in] height           Height in pixels of the layer
 * \param [in] format           Format of layer pixels
 * \param [in] dma_prefetch_lvl Prefetch level that overrides the layer structure
 * \param [in] burst_len        Configured maximum burst length
 *
 * \return true if provided settings are supported, false otherwise
 */
static bool _hw_lcdc_layer_is_valid(uint16_t width, uint16_t height, HW_LCDC_LAYER_COLOR_MODE format,
        HW_LCDC_FIFO_PREFETCH_LVL dma_prefetch_lvl, HW_LCDC_BURST_LEN burst_len)
{
        int line_sz, lines_min, prefetch_val, prefetch_mod, fifo_sz, fifo_space, line_rem,
                color_bytes, burst_max_sz;
        bool max_8_beats;

        if (dma_prefetch_lvl == HW_LCDC_FIFO_PREFETCH_LVL_DISABLED) {
                return true;
        }

        color_bytes = hw_lcdc_lcm_size(format);

        prefetch_mod = (12 - 3 * color_bytes) & ~3U;

        fifo_sz = 128 + 16 - prefetch_mod;
        prefetch_val = hw_lcdc_prefetch_value[dma_prefetch_lvl] - prefetch_mod;

        /* Calculate line size and ensure it is word aligned */
        line_sz = (width * color_bytes + 3) & ~3;

        /* Calculate minimum number of lines required to reach configured level */
        lines_min = prefetch_val / line_sz + 1;
        if (color_bytes == 1 && width == 2) {
                lines_min++;
        }

        /* Check if configured line number is above minimum */
        if (height < lines_min) {
                return false;
        }

        fifo_space = fifo_sz - (lines_min - 1) * line_sz;
        line_rem = 0;
        max_8_beats = burst_len == HW_LCDC_BURST_LEN_8_BEATS;
        burst_max_sz = hw_lcdc_get_burst(fifo_sz, max_8_beats);

        /* Calculate FIFO bytes */
        while (true) {
                int burst_sz;
                bool new_line = line_rem == 0;
                if (new_line) {
                        line_rem = line_sz;
                }
                burst_sz = hw_lcdc_get_burst(line_rem, max_8_beats);
                if (new_line) {
                        if (!((burst_sz == burst_max_sz && fifo_space >= burst_max_sz)
                                || fifo_space >= line_sz)) {
                                break;
                        }
                }

                if (burst_sz > fifo_space) {
                        break;
                }
                fifo_space -= burst_sz;
                line_rem -= burst_sz;
        }

        /* Check if bytes in fifo can reach prefetch level */
        if (fifo_sz - fifo_space <= prefetch_val) {
                return false;
        }

        return true;
}

bool hw_lcdc_layer_is_valid(const hw_lcdc_layer_t *layer, HW_LCDC_BURST_LEN burst_len)
{
        hw_lcdc_frame_t layer_frame;
        uint16_t width, height;

        /* Calculate actually transmitted area */
        layer_frame.startx = MAX(0, layer->startx);
        layer_frame.starty = MAX(0, layer->starty);
        layer_frame.endx = MAX(0, layer->startx + layer->resx - 1);
        layer_frame.endy = MAX(0, layer->starty + layer->resy - 1);

        if (!hw_lcdc_intersect_rects(&layer_frame, &lcdc_data.active_area, &layer_frame)) {
                return true;
        }

        width = layer_frame.endx - layer_frame.startx + 1;
        height = layer_frame.endy - layer_frame.starty + 1;

        return _hw_lcdc_layer_is_valid(width, height, layer->format, layer->dma_prefetch_lvl,
                burst_len);
}

/**
 * \brief Adjusts layer prefetch level according to provided layer parameters
 *
 * Function checks and decreases prefetch level if required by the provided layer resolution
 *
 * \param [in] layer_no         Layer index
 * \param [in] width            Width in pixels of the layer
 * \param [in] height           Height in pixels of the layer
 * \param [in] format           Format of layer pixels
 * \param [in] dma_prefetch_lvl Layer DMA prefetch level
 *
 * \return modified prefetch value
 */
static HW_LCDC_FIFO_PREFETCH_LVL hw_lcdc_adjust_prefetch(HW_LCDC_LAYER layer_no, uint16_t width,
        uint16_t height, HW_LCDC_LAYER_COLOR_MODE format, HW_LCDC_FIFO_PREFETCH_LVL dma_prefetch_lvl)
{
        HW_LCDC_BURST_LEN burst_len = hw_lcdc_get_layer_burst_len(layer_no);
        int idx, prefetch_val = hw_lcdc_prefetch_value[dma_prefetch_lvl];

        for (idx = ARRAY_LENGTH(hw_lcdc_prefetch_level) - 1; idx > 0; idx--) {
                /* Skip if configured level is already lower */
                if (hw_lcdc_prefetch_value[hw_lcdc_prefetch_level[idx]] > prefetch_val) {
                        continue;
                }

                if (_hw_lcdc_layer_is_valid(width, height, format, hw_lcdc_prefetch_level[idx],
                        burst_len)) {
                        break;
                }
        }

        return hw_lcdc_prefetch_level[idx];
}

bool hw_lcdc_set_layer(HW_LCDC_LAYER layer_no, bool enable, const hw_lcdc_layer_t *layer)
{
        if (enable) {
                HW_LCDC_FIFO_PREFETCH_LVL dma_prefetch_lvl = layer->dma_prefetch_lvl;
                uint16_t disp_resx, disp_resy;
                int16_t stride = layer->stride;
                uint32_t addr = black_orca_phy_addr(layer->baseaddr);
                uint32_t resx = layer->resx;
                uint32_t resy = layer->resy;
                int16_t sx = layer->startx - lcdc_data.active_area.startx;
                int16_t sy = layer->starty - lcdc_data.active_area.starty;
                uint32_t szx, szy;
                int endx, endy;

                if (lcdc_data.phy == HW_LCDC_PHY_JDI_PARALLEL) {
                        /* Perform JDI parallel only calculations */
                        if (sy >= 0) {
                                sy *= 2;
                                resy *= 2;
                        }
                        else {
                                resy = resy * 2 + sy;
                        }
                }

                szx = resx;
                szy = resy;
                endx = sx + resx;
                endy = sy + resy;

                get_resolution(&disp_resx, &disp_resy);

                if (stride == 0) {
                        stride = hw_lcdc_stride_size(layer->format, layer->resx);
                }

                if ((int)sx >= (int)disp_resx
                        || (int)sy >= (int)disp_resy
                        || endx <= 0
                        || endy <= 0) {
                        set_layer_mode(layer_no, false, 0);
                        return false;
                }
                if (sx < 0) {
                        uint8_t pixel_bytes = hw_lcdc_lcm_size(layer->format);
                        /* align sx to word */
                        if (pixel_bytes == 3) {
                                sx = sx / 4 * 4;
                        } else {
                                sx = sx * pixel_bytes / 4 * 4 / pixel_bytes;
                        }
                        addr -= (uint32_t)(sx * pixel_bytes);
                        szx += sx;
                        endx = szx;
                        sx = 0;
                }

                if (sy < 0) {
                        addr -= (uint32_t)(sy * stride);
                        szy += sy;
                        endy = szy;
                        sy = 0;
                }

                if (endx > disp_resx) {
                        endx = disp_resx;
                }

                if (endy > disp_resy) {
                        endy = disp_resy;
                }

                resx = szx = endx - sx;
                resy = szy = endy - sy;

                if (IS_OQSPIC_ADDRESS(addr)) {
                        addr += MEMORY_OQSPIC_S_BASE - MEMORY_OQSPIC_BASE;
                }
                ASSERT_ERROR((addr & 0x3) == 0); /* Ensure base address is word aligned */
                ASSERT_ERROR((stride & 0x3) == 0); /* Ensure stride has a proper length */

                set_layer_base_addr(layer_no, addr);
                set_layer_start(layer_no, sx, sy);
                dma_prefetch_lvl = hw_lcdc_adjust_prefetch(layer_no, resx, resy, layer->format, dma_prefetch_lvl);
                set_layer_dma_prefetch(layer_no, dma_prefetch_lvl);
                set_layer_size(layer_no, szx, szy);
                set_layer_resolution(layer_no, resx, resy);

                /* Permit change of unprotected registers only if continuous mode is off */
                if (!HW_LCDC_REG_GETF(LCDC_MODE_REG, MODE_EN)) {
                        set_layer_stride(layer_no, stride);
                        set_layer_blend_mode(layer_no, layer->blendmode, layer->alpha);
                } else {
                        GLOBAL_INT_DISABLE();
                        lcdc_data.layer_dirty[layer_no] = true;
                        lcdc_data.stride[layer_no] = stride;
                        lcdc_data.blendmode[layer_no] = layer->blendmode;
                        lcdc_data.alpha[layer_no] = layer->alpha;
                        GLOBAL_INT_RESTORE();
                }
        }

        set_layer_mode(layer_no, enable, layer->format);

        return enable;
}

void hw_lcdc_set_scs(HW_LCDC_SCS_CFG state)
{
        HW_LCDC_MIPI_CFG cfg = hw_lcdc_get_mipi_cfg() & ~HW_LCDC_MIPI_CFG_FRC_CSX_1;

        switch (state) {
        case HW_LCDC_SCS_AUTO:
                hw_lcdc_set_mipi_cfg(cfg);
                break;
        case HW_LCDC_SCS_AUTO_INV:
                hw_lcdc_set_mipi_cfg(cfg | HW_LCDC_MIPI_CFG_SPI_CSX_V);
                break;
        case HW_LCDC_SCS_HIGH:
                hw_lcdc_set_mipi_cfg(cfg | HW_LCDC_MIPI_CFG_FRC_CSX_1);
                break;
        case HW_LCDC_SCS_LOW:
                hw_lcdc_set_mipi_cfg(cfg | HW_LCDC_MIPI_CFG_FRC_CSX_0);
                break;
        }
}

HW_LCDC_SCS_CFG hw_lcdc_get_scs(void)
{
        HW_LCDC_MIPI_CFG cfg = hw_lcdc_get_mipi_cfg();

        switch (cfg & HW_LCDC_MIPI_CFG_FRC_CSX_1) {
        case HW_LCDC_MIPI_CFG_FRC_CSX_0:
                return HW_LCDC_SCS_LOW;
        case HW_LCDC_MIPI_CFG_SPI_CSX_V:
                return HW_LCDC_SCS_AUTO_INV;
        case HW_LCDC_MIPI_CFG_FRC_CSX_1:
                return HW_LCDC_SCS_HIGH;
        default:
                return HW_LCDC_SCS_AUTO;
        }
}

void hw_lcdc_set_hold(bool enable)
{
        uint32_t lcdc_dbib_cfg_reg = hw_lcdc_get_mipi_cfg();
        if (enable) {
                lcdc_dbib_cfg_reg |= HW_LCDC_MIPI_CFG_SPI_HOLD;
        } else {
                lcdc_dbib_cfg_reg &= ~HW_LCDC_MIPI_CFG_SPI_HOLD;
        }
        hw_lcdc_set_mipi_cfg(lcdc_dbib_cfg_reg);
}

void hw_lcdc_set_tearing_effect(bool enable, HW_LCDC_TE polarity)
{
        uint32_t lcdc_dbib_cfg_reg = hw_lcdc_get_mipi_cfg();

        HW_LCDC_REG_SETF(LCDC_GPIO_REG, TE_INV, polarity == HW_LCDC_TE_POL_LOW ? 0 : 1);
        if (enable) {
                lcdc_dbib_cfg_reg &= ~HW_LCDC_MIPI_CFG_TE_DIS;
        } else {
                lcdc_dbib_cfg_reg |= HW_LCDC_MIPI_CFG_TE_DIS;
        }
        hw_lcdc_set_mipi_cfg(lcdc_dbib_cfg_reg);
}

/**
 * \brief Enables / disables the dual SPI mode
 *
 * \param [in] enable           Enable state
 */
static void hw_lcdc_dspi_set_mode(bool enable)
{
        HW_LCDC_MIPI_CFG cfg = hw_lcdc_get_mipi_cfg();
        while (hw_lcdc_is_busy());
        if (enable) {
                cfg &= ~HW_LCDC_MIPI_CFG_SPI4;
                cfg |= HW_LCDC_MIPI_CFG_SPI3 | HW_LCDC_MIPI_CFG_DSPI | HW_LCDC_MIPI_CFG_SPIDC_DQSPI;
        } else {
                cfg &= ~(HW_LCDC_MIPI_CFG_SPI3 | HW_LCDC_MIPI_CFG_DSPI | HW_LCDC_MIPI_CFG_SPIDC_DQSPI);
                cfg |= lcdc_data.lcdc->iface_conf.dspi.spi3 ? HW_LCDC_MIPI_CFG_SPI3 : HW_LCDC_MIPI_CFG_SPI4;
        }
        hw_lcdc_set_mipi_cfg(cfg);
}

/**
 * \brief Sets the frame transfer command
 *
 * \param [in] cmd              Command to be set
 * \param [in] cmd_len          Command length in bytes to be set
 */
static void hw_lcdc_send_mipi_frame_cmd(const uint8_t *cmd, uint8_t cmd_len)
{
        hw_lcdc_set_hold(true);
        if (cmd_len) {
                for (int i = 0; i < cmd_len; i++) {
                        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD_FRAME, cmd[i]);
                }
        } else {
                hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD_FRAME, HW_LCDC_MIPI_DCS_WRITE_MEMORY_START);
        }
}

void hw_lcdc_send_one_frame(void)
{
        HW_LCDC_MODE mode = LCDC->LCDC_MODE_REG;
        uint8_t cmd_len = 0;
        const uint8_t *cmd;

        switch (lcdc_data.phy) {
        case HW_LCDC_PHY_DUAL_SPI:
                cmd = lcdc_data.lcdc->iface_conf.dspi.write_memory_cmd;
                cmd_len = lcdc_data.lcdc->iface_conf.dspi.write_memory_cmd_len;
                hw_lcdc_send_mipi_frame_cmd(cmd, cmd_len);
                hw_lcdc_set_hold(false);
                WAIT_TIMEOUT(LCDC->LCDC_STATUS_REG & HW_LCDC_REG_MSK(LCDC_STATUS_REG, SPI_RD_WR_OP), 10);
                hw_lcdc_dspi_set_mode(true);
                hw_lcdc_set_mipi_cfg(hw_lcdc_get_mipi_cfg() | HW_LCDC_MIPI_CFG_FRC_CSX_0);
                break;
        case HW_LCDC_PHY_QUAD_SPI:
                cmd = lcdc_data.lcdc->iface_conf.qspi.write_memory_cmd;
                cmd_len = lcdc_data.lcdc->iface_conf.qspi.write_memory_cmd_len;
                hw_lcdc_send_mipi_frame_cmd(cmd, cmd_len);
                hw_lcdc_set_mipi_cfg(hw_lcdc_get_mipi_cfg() | HW_LCDC_MIPI_CFG_FRC_CSX_0);
                break;
        case HW_LCDC_PHY_MIPI_DBIB:
                cmd = lcdc_data.lcdc->iface_conf.dbib.write_memory_cmd;
                cmd_len = lcdc_data.lcdc->iface_conf.dbib.write_memory_cmd_len;
                hw_lcdc_send_mipi_frame_cmd(cmd, cmd_len);
                hw_lcdc_set_mipi_cfg(hw_lcdc_get_mipi_cfg() | HW_LCDC_MIPI_CFG_FRC_CSX_0);
                break;
        case HW_LCDC_PHY_MIPI_SPI3:
        case HW_LCDC_PHY_MIPI_SPI4:
                cmd = lcdc_data.lcdc->iface_conf.spi.write_memory_cmd;
                cmd_len = lcdc_data.lcdc->iface_conf.spi.write_memory_cmd_len;
                hw_lcdc_send_mipi_frame_cmd(cmd, cmd_len);
                hw_lcdc_set_mipi_cfg(hw_lcdc_get_mipi_cfg() | HW_LCDC_MIPI_CFG_FRC_CSX_0);
                break;
        case HW_LCDC_PHY_JDI_SPI:
                hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD_FRAME, lcdc_data.jdis_update_cmd);
                hw_lcdc_mipi_cmd(HW_LCDC_MIPI_STORE_BADDR, lcdc_data.active_area.starty + 1);
                break;
        case HW_LCDC_PHY_SHARP_SPI:
                hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD_FRAME, lcdc_data.jdis_update_cmd);
                hw_lcdc_mipi_cmd(HW_LCDC_MIPI_STORE_BADDR,
                        byte_reverse(lcdc_data.active_area.starty + 1));
                break;
        default:
                break;
        }

        hw_lcdc_set_mode(mode | HW_LCDC_MODE_ONE_FRAME);
}

void hw_lcdc_set_continuous_mode(bool enable)
{
        HW_LCDC_MODE mode = LCDC->LCDC_MODE_REG;
        if (!enable || (lcdc_data.phy != HW_LCDC_PHY_JDI_PARALLEL
                && lcdc_data.phy != HW_LCDC_PHY_DPI))
        {

                hw_lcdc_set_mode(mode & ~HW_LCDC_MODE_ENABLE);
        } else {
                hw_lcdc_set_mode(mode | HW_LCDC_MODE_ENABLE);

                /* Block until transmission of the first frame starts */
                while (!HW_LCDC_REG_GETF(LCDC_STATUS_REG, STAT_VSYNC));
                while (HW_LCDC_REG_GETF(LCDC_STATUS_REG, STAT_VSYNC));
        }
}

/** \} */

/**
 * \name                MIPI functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Adds a command to the command FIFO
 *
 * \param [in] cmd              Command to be added
 */
static void hw_lcdc_mipi_add_cmd(uint32_t cmd)
{
        while (HW_LCDC_REG_GETF(LCDC_STATUS_REG, DBIB_CMD_FIFO_FULL));

        LCDC->LCDC_DBIB_CMD_REG = cmd;
}

void hw_lcdc_mipi_cmd(HW_LCDC_MIPI type, HW_LCDC_MIPI_DCS value)
{
        uint32_t lcdc_dbib_cmd_reg = 0;
        bool cmd = (type == HW_LCDC_MIPI_CMD || type == HW_LCDC_MIPI_CMD_FRAME
                || type == HW_LCDC_MIPI_READ || type == HW_LCDC_MIPI_STORE_BADDR) ? 1 : 0;
        bool line_addr = type == HW_LCDC_MIPI_STORE_BADDR ? 1 : 0;

        HW_LCDC_REG_SET_FIELD(LCDC_DBIB_CMD_REG, DBIB_CMD_SEND, lcdc_dbib_cmd_reg, cmd);


        switch (lcdc_data.phy) {
        case HW_LCDC_PHY_QUAD_SPI:
                switch (type) {
                case HW_LCDC_MIPI_CMD:
                case HW_LCDC_MIPI_CMD_FRAME:
                case HW_LCDC_MIPI_READ: {
                        uint8_t qspi_cmd;
                        bool serial;
                        if (lcdc_data.qspi_mode_config == HW_LCDC_QSPI_MODE_AUTO) {
                                serial = type == HW_LCDC_MIPI_CMD_FRAME ? false : true;
                        } else { /* HW_LCDC_QSPI_MODE_FORCE_SINGLE */
                                serial = true;
                        }
                        if (serial) {
                                if (type == HW_LCDC_MIPI_READ) {
                                        qspi_cmd = lcdc_data.lcdc->iface_conf.qspi.sss_read_cmd;
                                } else {
                                        qspi_cmd = lcdc_data.lcdc->iface_conf.qspi.sss_write_cmd;
                                }
                        } else {
                                qspi_cmd = lcdc_data.lcdc->iface_conf.qspi.ssq_write_cmd;
                        }

                        /* Send the QSPI command that sets the correct mode */
                        HW_LCDC_REG_SET_FIELD(LCDC_DBIB_CMD_REG, QSPI_SERIAL_CMD_TRANS, lcdc_dbib_cmd_reg, 1);
                        HW_LCDC_REG_SET_FIELD(LCDC_DBIB_CMD_REG, DBIB_CMD_VAL, lcdc_dbib_cmd_reg, qspi_cmd);

                        hw_lcdc_mipi_add_cmd(lcdc_dbib_cmd_reg);

                        /* Send the actual command in the address field */
                        HW_LCDC_REG_SET_FIELD(LCDC_DBIB_CMD_REG, QSPI_SERIAL_CMD_TRANS, lcdc_dbib_cmd_reg, 1);
                        HW_LCDC_REG_SET_FIELD(LCDC_DBIB_CMD_REG, CMD_WIDTH, lcdc_dbib_cmd_reg, lcdc_data.lcdc->iface_conf.qspi.cmd_width);
                        HW_LCDC_REG_SET_FIELD(LCDC_DBIB_CMD_REG, DBIB_CMD_VAL, lcdc_dbib_cmd_reg, value << 8);
                        break;
                }
                case HW_LCDC_MIPI_DATA:
                        HW_LCDC_REG_SET_FIELD(LCDC_DBIB_CMD_REG, QSPI_SERIAL_CMD_TRANS, lcdc_dbib_cmd_reg, 1);
                        HW_LCDC_REG_SET_FIELD(LCDC_DBIB_CMD_REG, DBIB_CMD_VAL, lcdc_dbib_cmd_reg, value);
                        break;
                default:
                        return;
                }
                break;
        case HW_LCDC_PHY_DUAL_SPI:
                if (hw_lcdc_get_mipi_cfg() & HW_LCDC_MIPI_CFG_DSPI) {
                        hw_lcdc_dspi_set_mode(false);
                }
                /* Falls through - No break */
        default:
                HW_LCDC_REG_SET_FIELD(LCDC_DBIB_CMD_REG, PART_UPDATE, lcdc_dbib_cmd_reg, line_addr);
                HW_LCDC_REG_SET_FIELD(LCDC_DBIB_CMD_REG, DBIB_CMD_VAL, lcdc_dbib_cmd_reg, value);
        }

        hw_lcdc_mipi_add_cmd(lcdc_dbib_cmd_reg);
}

int hw_lcdc_dcs_cmd_params(HW_LCDC_MIPI_DCS cmd, const uint8_t *params, size_t param_len)
{
        switch (lcdc_data.phy) {
        case HW_LCDC_PHY_MIPI_DBIB:
        case HW_LCDC_PHY_QUAD_SPI:
        case HW_LCDC_PHY_DUAL_SPI:
        case HW_LCDC_PHY_MIPI_SPI3:
        case HW_LCDC_PHY_MIPI_SPI4:
                return hw_lcdc_gen_cmd_params(&cmd, sizeof(cmd), params, param_len);
        default:
                return HW_LCDC_ERR_UNSUPPORTED;
        }
}

int hw_lcdc_gen_cmd_params(const uint8_t *cmds, size_t cmd_len, const uint8_t *params, size_t param_len)
{
        switch (lcdc_data.phy) {
        case HW_LCDC_PHY_QUAD_SPI:
                if (cmd_len > 1) {
                        return HW_LCDC_ERR_UNSUPPORTED;
                }
                /* Falls through - No break */
        case HW_LCDC_PHY_MIPI_DBIB:
        case HW_LCDC_PHY_DUAL_SPI:
        case HW_LCDC_PHY_MIPI_SPI3:
        case HW_LCDC_PHY_MIPI_SPI4: {
                size_t index = 0;
                bool hold = true;

                while (hw_lcdc_is_busy());

                hw_lcdc_set_hold(true);

                while (index < cmd_len) {
                        if (hold) {
                                if (HW_LCDC_REG_GETF(LCDC_STATUS_REG, DBIB_CMD_FIFO_FULL)) {
                                        /* FIFO is full, start transmitting and send the rest in the meantime */
                                        hw_lcdc_set_hold(false);
                                        hold = false;
                                }
                        }
                        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, cmds[index++]);
                }

                index = 0;

                while (index < param_len) {
                        if (hold) {
                                if (HW_LCDC_REG_GETF(LCDC_STATUS_REG, DBIB_CMD_FIFO_FULL)) {
                                        /* FIFO is full, start transmitting and send the rest in the meantime */
                                        hw_lcdc_set_hold(false);
                                        hold = false;
                                }
                        }
                        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, params[index++]);
                }

                if (hold) {
                        hw_lcdc_set_hold(false);
                }
                return HW_LCDC_ERR_NONE;
        }
        case HW_LCDC_PHY_JDI_SPI:
        case HW_LCDC_PHY_SHARP_SPI:
                hw_lcdc_jdi_serial_cmd_send(cmds[0]);
                return HW_LCDC_ERR_NONE;
        default:
                return HW_LCDC_ERR_UNSUPPORTED;
        }
}

void hw_lcdc_set_mipi_cfg(HW_LCDC_MIPI_CFG cfg)
{
        /* Make sure command queue is not full */
        while (HW_LCDC_REG_GETF(LCDC_STATUS_REG, DBIB_CMD_PENDING));
        LCDC->LCDC_DBIB_CFG_REG = cfg;
}

void hw_lcdc_mipi_set_qpsi_mode(HW_LCDC_QSPI_MODE mode)
{
        lcdc_data.qspi_mode_config = mode;
}

int hw_lcdc_dcs_read(HW_LCDC_MIPI_DCS cmd, uint8_t *data, size_t data_len, size_t dummy_ticks)
{
        switch (lcdc_data.phy) {
        case HW_LCDC_PHY_MIPI_DBIB:
        case HW_LCDC_PHY_QUAD_SPI:
        case HW_LCDC_PHY_DUAL_SPI:
        case HW_LCDC_PHY_MIPI_SPI3:
        case HW_LCDC_PHY_MIPI_SPI4:
                return hw_lcdc_gen_read(&cmd, sizeof(cmd), data, data_len, dummy_ticks);
        default:
                return HW_LCDC_ERR_UNSUPPORTED;
        }
}

/**
 * \brief Waits for the completion of a read operation on DBI interface.
 *
 * The function times out after a predefined time in case the read operation has already been started
 *
 * \param [in] ticks            Number of output clock ticks to wait for
 * \param [in] tick_period      Tick period in nanoseconds
 */
static void hw_lcdc_mipi_read_wait_idle(int ticks, uint32_t tick_period)
{
        if (ticks) {
                WAIT_TIMEOUT(false, ROUND_UP(tick_period * (ticks + 1), 1000UL));
        } else {
                WAIT_TIMEOUT(LCDC->LCDC_STATUS_REG & HW_LCDC_REG_MSK(LCDC_STATUS_REG, SPI_RD_WR_OP),
                        ROUND_UP(tick_period * 2, 1000UL));
                while (LCDC->LCDC_STATUS_REG & (HW_LCDC_REG_MSK(LCDC_STATUS_REG, SPI_RD_WR_OP) |
                        HW_LCDC_REG_MSK(LCDC_STATUS_REG, DBIB_OUT_TRANS_PENDING) |
                        HW_LCDC_REG_MSK(LCDC_STATUS_REG, DBIB_CMD_PENDING)));
        }
}

/**
 * \brief Calculates output clock tick period in nanoseconds
 *
 * \return tick period (in ns)
 */
static uint32_t hw_lcdc_mipi_get_clock_period(void)
{
        uint32_t tick_ns = 2000UL * HW_LCDC_REG_GETF(LCDC_CLKCTRL_REG, SEC_CLK_DIV);
        uint32_t sys_clk = REG_GETF(CRG_SYS, CLK_SYS_REG, LCD_CLK_SEL) ?
                                                                         hw_clk_get_sysclk_freq() :
                                                                         dg_configDIVN_FREQ;

        sys_clk /= 1000000UL;
        tick_ns = ROUND_UP(tick_ns, sys_clk);

        return tick_ns;
}

int hw_lcdc_gen_read(const uint8_t *cmd, size_t cmd_len, uint8_t *data, size_t data_len, size_t dummy_ticks)
{
        HW_LCDC_MIPI_CFG cfg = hw_lcdc_get_mipi_cfg();
        uint32_t tick_ns, value;
        size_t index = 0;
        uint8_t dummy_bits = dummy_ticks;
        bool sio_state = false, use_timer = false;

        switch (lcdc_data.phy) {
        case HW_LCDC_PHY_MIPI_DBIB:
                /* Convert ticks to bits */
                dummy_bits = dummy_ticks * 8;
                sio_state = false;
                /* DBIB transactions need to be timed out */
                use_timer = true;
                break;
        case HW_LCDC_PHY_QUAD_SPI:
                sio_state = lcdc_data.lcdc->iface_conf.qspi.si_on_so;
                if (cmd_len > 1) {
                        return HW_LCDC_ERR_UNSUPPORTED;
                }
                break;
        case HW_LCDC_PHY_DUAL_SPI:
                sio_state = lcdc_data.lcdc->iface_conf.dspi.si_on_so;
                break;
        case HW_LCDC_PHY_MIPI_SPI3:
        case HW_LCDC_PHY_MIPI_SPI4:
                sio_state = lcdc_data.lcdc->iface_conf.spi.si_on_so;
                break;
        default:
                return HW_LCDC_ERR_UNSUPPORTED;
        }

        /* Ensure interface is idle */
        while (hw_lcdc_is_busy());

        /* Change clock to the read configuration */
        hw_lcdc_enable(lcdc_data.phy, lcdc_data.lcdc->read_freq);
        hw_lcdc_set_spi_sio(sio_state);

        tick_ns = hw_lcdc_mipi_get_clock_period();

        /* Force CS to be asserted and clear hold */
        hw_lcdc_set_mipi_cfg((cfg | HW_LCDC_MIPI_CFG_FRC_CSX_0) & ~HW_LCDC_MIPI_CFG_SPI_HOLD);

        /* Send read command and wait until transfer ends */
        while (index < cmd_len) {
                hw_lcdc_mipi_cmd(HW_LCDC_MIPI_READ, cmd[index++]);
        }
        hw_lcdc_mipi_read_wait_idle(use_timer ? cmd_len + 2 : 0, tick_ns);

        /* Account for dummy cycle(s) */
        while (dummy_bits) {
                uint8_t bits = MIN(32, dummy_bits);

                hw_lcdc_set_read_cycles(bits);

                hw_lcdc_mipi_read_wait_idle(use_timer ? (bits / 8) : 0, tick_ns);

                dummy_bits -=bits;
        }

        index = 0;
        while (data_len) {
                uint8_t bytes = MIN(4, data_len);

                /* Receive actual data */
                hw_lcdc_set_read_cycles(bytes * 8);

                hw_lcdc_mipi_read_wait_idle(use_timer ? bytes : 0, tick_ns);

                value = hw_lcdc_get_read_data();

                for (int i = bytes - 1; i >= 0; i--, index++) {
                        data[index] = ((uint8_t *)&value)[i];
                }

                data_len -= bytes;
        }

        /* Restore CS and hold configuration */
        hw_lcdc_set_mipi_cfg(cfg);

        /* Restore write clock frequency */
        hw_lcdc_enable(lcdc_data.phy, lcdc_data.lcdc->write_freq);

        return index;
}

/** \} */

/**
 * \name                JDI / Sharp functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Release and restore HOLD flag
 *
 * Release the HOLD flag that binds commands and data to enable command transmission and restore it
 * if it was previously used.
 */
__STATIC_INLINE void jdi_serial_cmd_release(void)
{
        hw_lcdc_set_hold(false);
        while (HW_LCDC_REG_GETF(LCDC_STATUS_REG, DBIB_CMD_PENDING));
        hw_lcdc_set_hold(true);
}

void hw_lcdc_jdi_serial_cmd_send(HW_LCDC_JDIS_CMD cmd)
{
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, cmd);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_JDIS_CMD_NOP);
        jdi_serial_cmd_release();
}

void hw_lcdc_jdi_parallel(uint16_t resx, uint16_t resy, const hw_lcdc_jdi_par_t *jdi_par)
{
        uint16_t fpx, fpy, bpx, bpy, blx, bly, line;
        uint32_t vck_width, hck_width;
        uint32_t xrst_width, xrst_offset;
        uint32_t vst_width, vst_offset;
        uint32_t hst_width, hst_offset;
        uint32_t enb_width, enb_offset;

        fpx = jdi_par->fpx;
        blx = jdi_par->blx;
        bpx = jdi_par->bpx;

        fpy = jdi_par->fpy;
        bly = jdi_par->bly;
        bpy = jdi_par->bpy;

        line = (resx + fpx + blx + bpx) / 2;

        hck_width = 2;
        vck_width = line * hck_width;

        hst_width = hck_width;
        hst_offset = hck_width;

        enb_offset = vck_width / 4;
        enb_width = vck_width / 2;

        vst_width = vck_width;
        vst_offset = vck_width / 2;

        xrst_width = (resy * 2 + fpy + bly + bpy - 2);
        xrst_offset = vck_width / 4;

        uint32_t lcdc_fmtctrl_reg = LCDC->LCDC_FMTCTRL_REG;
        /* Number of format clock cycles of HST width - (tsHST + thHST) / format_clk */
        HW_LCDC_REG_SET_FIELD(LCDC_FMTCTRL_REG, JDIP_HST_WIDTH, lcdc_fmtctrl_reg, hst_width);
        /* Number of format clock cycles of VCK-to-HST delay - tdHST / format_clk */
        HW_LCDC_REG_SET_FIELD(LCDC_FMTCTRL_REG, JDIP_HST_OFFSET, lcdc_fmtctrl_reg, hst_offset);
        /* Number of format clock cycles of VST width - (tsVST + thVST) / format_clk */
        HW_LCDC_REG_SET_FIELD(LCDC_FMTCTRL_REG, JDIP_VST_WIDTH, lcdc_fmtctrl_reg, vst_width);
        /* Number of format clock cycles of VST-to-VCK delay - (twVCKL - tsVST) /format_clk + 2 */
        HW_LCDC_REG_SET_FIELD(LCDC_FMTCTRL_REG, JDIP_VST_OFFSET, lcdc_fmtctrl_reg, vst_offset + 2);
        LCDC->LCDC_FMTCTRL_REG = lcdc_fmtctrl_reg;

        uint32_t lcdc_fmtctrl_2_reg = LCDC->LCDC_FMTCTRL_2_REG;
        /* Number of format clock cycles of ENB width - twEN / format_clk */
        HW_LCDC_REG_SET_FIELD(LCDC_FMTCTRL_2_REG, JDIP_ENB_WIDTH, lcdc_fmtctrl_2_reg, enb_width);
        /* Number of format clock cycles of VCK-to-ENB delay - tsVCK / format_clk + 3 */
        HW_LCDC_REG_SET_FIELD(LCDC_FMTCTRL_2_REG, JDIP_ENB_OFFSET, lcdc_fmtctrl_2_reg, enb_offset + 3);
        /* Number of format clock cycles of XRST-to-VCK delay - (twVCKL - (tsXRST + tsVST)) / format_clk + 2 */
        HW_LCDC_REG_SET_FIELD(LCDC_FMTCTRL_2_REG, JDIP_XRST_OFFSET, lcdc_fmtctrl_2_reg, xrst_offset + 2);
        LCDC->LCDC_FMTCTRL_2_REG = lcdc_fmtctrl_2_reg;

        /* Number of format clock cycles of XRST width */
        HW_LCDC_REG_SETF(LCDC_FMTCTRL_3_REG, XRST_HIGH_STATE, xrst_width);
}
/** \} */

/**
 * \name                Interrupt functions
 *****************************************************************************************
 * \{
 */

void hw_lcdc_set_callback(hw_lcdc_callback cb, void *user_data)
{
        lcdc_data.cb = cb;
        lcdc_data.cb_data = user_data;
}

static void hw_lcdc_call_callback(HW_LCDC_ERR status, bool clear)
{
        hw_lcdc_callback cb = lcdc_data.cb;
        void *cb_data = lcdc_data.cb_data;

        if (clear) {
                lcdc_data.cb = NULL;
                lcdc_data.cb_data = NULL;
        }
        if (cb) {
                cb(status, cb_data);
        }
}

/**
 * \brief LCD Controller Interrupt Handler
 *
 */
void HW_LCDC_Handler(void)
{
        HW_LCDC_ERR status = HW_LCDC_ERR_NONE;

        SEGGER_SYSTEMVIEW_ISR_ENTER();

        /* In case of continuous mode, set unprotected layer registers as fast as possible */
        if (HW_LCDC_REG_GETF(LCDC_MODE_REG, MODE_EN)) {
                GLOBAL_INT_DISABLE();
                for (HW_LCDC_LAYER layer = 0; layer < HW_LCDC_LAYER_MAX; layer++) {
                        if (lcdc_data.layer_dirty[layer]) {
                                lcdc_data.layer_dirty[layer] = false;
                                set_layer_stride(layer, lcdc_data.stride[layer]);
                                set_layer_blend_mode(layer, lcdc_data.blendmode[layer], lcdc_data.alpha[layer]);
                        }
                }
                GLOBAL_INT_RESTORE();
        }

        /* If TE is enabled, immediately call callback */
        if (~hw_lcdc_get_mipi_cfg() & HW_LCDC_MIPI_CFG_TE_DIS) {
                hw_lcdc_call_callback(status, true);
                return;
        }

        switch (lcdc_data.phy) {
        case HW_LCDC_PHY_DUAL_SPI:
        case HW_LCDC_PHY_QUAD_SPI:
        case HW_LCDC_PHY_MIPI_DBIB:
        case HW_LCDC_PHY_MIPI_SPI3:
        case HW_LCDC_PHY_MIPI_SPI4:
                hw_lcdc_set_hold(false);
                hw_lcdc_set_mipi_cfg(hw_lcdc_get_mipi_cfg() & ~HW_LCDC_MIPI_CFG_FRC_CSX_0);

                break;
        default:
                break;
        }

        if (hw_lcdc_get_sticky_underflow_status()) {
                status = HW_LCDC_ERR_UNDERFLOW;
        }

        /* Do not clear interrupt callback in case of continuous refresh */
        hw_lcdc_call_callback(status, !HW_LCDC_REG_GETF(LCDC_MODE_REG, MODE_EN));

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

/** \} */

#endif /* dg_configUSE_HW_LCDC */

