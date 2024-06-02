/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup LCDC_ADAPTER LCD Controller Adapter
 *
 * \brief Liquid Crystal Display Controller Adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_lcdc.h
 *
 * @brief LCD controller adapter API
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_LCDC_H_
#define AD_LCDC_H_

#if dg_configLCDC_ADAPTER

#include <hw_lcdc.h>
#include <osal.h>
#include <hw_gpio.h>
#include <ad.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS
 *
 * \brief Controls whether LCDC adapter tries to automatically configure the system clocks
 */
#ifndef CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS
#define CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS       (1)
#endif

/**
 * \def HW_GPIO_FUNC_LCD
 *
 * \brief Defines a virtual LCD pin function that has to be used in the pin configuration list
 */
#define HW_GPIO_FUNC_LCD                        (HW_GPIO_FUNC_LAST)

/*
 * Data types definitions section
 */
/**
 * \brief LCDC adapter error codes
 */
typedef enum AD_LCDC_ERROR {
        AD_LCDC_ERROR_IO_CFG_INVALID            = -11,  //!< Invalid IO configuration
        AD_LCDC_ERROR_LLD_ERROR                 = -10,  //!< LLD error, use \ref ad_lcdc_get_lld_status to get error code
        AD_LCDC_ERROR_UNSUPPORTED               = -9,   //!< Operation not supported
        AD_LCDC_ERROR_PARAM_INVALID             = -8,   //!< Invalid parameter(s)
        AD_LCDC_ERROR_UNDERFLOW                 = -7,   //!< Underflow error during frame transfer
        AD_LCDC_ERROR_UNKNOWN                   = -6,   //!< Undefined error
        AD_LCDC_ERROR_TIMEOUT                   = -5,   //!< Event timeout error
        AD_LCDC_ERROR_SRC_CLOCKS                = -4,   //!< Source clock(s) error
        AD_LCDC_ERROR_CONTROLLER_BUSY           = -3,   //!< Controller is busy with another operation error
        AD_LCDC_ERROR_DRIVER_CONF_INVALID       = -2,   //!< Driver configuration is invalid error
        AD_LCDC_ERROR_HANDLE_INVALID            = -1,   //!< Device handle is not valid error
        AD_LCDC_ERROR_NONE                      = 0     //!< No error
} AD_LCDC_ERROR;

/**
 * \brief Tag enumeration used for the creation of command sequences which are used to initialize,
 * enable etc the LCD.
 */
typedef enum {
        LCDC_TAG_DELAY_US,                              //!< Delay using the hw_clk_delay_usec() function
        LCDC_TAG_DELAY_MS,                              //!< Delay using the OS_DELAY_MS() function
        LCDC_TAG_GPIO_SET_ACTIVE,                       //!< Set a GPIO active using hw_gpio_set_active() function
        LCDC_TAG_GPIO_SET_INACTIVE,                     //!< Set a GPIO inactive using hw_gpio_set_inactive() function
        LCDC_TAG_MIPI_CMD,                              //!< Send a MIPI DCS command to the LCD
        LCDC_TAG_MIPI_PARAM,                            //!< Send a MIPI DCS parameter to the LCD
        LCDC_TAG_MIPI_CMD_PARAM,                        //!< Send a MIPI DCS command with parameters to the LCD
        LCDC_TAG_GEN_CMD_PARAM,                         //!< Send a generic command with parameters to the LCD
        LCDC_TAG_JDI_CMD,                               //!< Send a JDI / Sharp command to the LCD
        LCDC_TAG_EXT_CLK,                               //!< Enable / disable the external clock generation
} LCDC_TAG;

/**
 * \brief LCDC handle returned by ad_lcdc_open()
 */
typedef void *ad_lcdc_handle_t;

/**
 * \brief Asynchronous callback function
 *
 * \param [in] status           Operation completion status
 * \param [in] user_data        Callback data of the corresponding asynchronous function
 */
typedef void (*ad_lcdc_user_cb)(AD_LCDC_ERROR status, void *user_data);

/**
 * \brief Function pointer used to provide a function to overload the default that configures LCD position
 *
 * \param [in] frame            Frame settings that the LCD should be configured
 */
typedef void (*ad_lcdc_mipi_set_position_callback)(const hw_lcdc_frame_t *frame);

/**
 * \brief LCD controller GPIO configuration
 */
typedef struct {
        HW_GPIO_POWER voltage_level;                    //!< Voltage level of the IOs of the device
        uint8_t io_cnt;                                 //!< Number of IOs in the \ref io_list list
        const ad_io_conf_t *io_list;                    //!< Array of IOs of the connected devices
} ad_lcdc_io_conf_t;

/**
 * \brief LCDC driver configuration
 *
 * Variable of this type keeps configuration needed to access LCDC controller.
 */
typedef struct ad_lcdc_driver_conf_t {
        hw_lcdc_config_t hw_init;                       //!< LCDC configuration passed to the LCDC LLD
        uint32_t *palette_lut;                          //!< Pointer to the palette array used for gamma correction
        ad_lcdc_mipi_set_position_callback set_position_cb; //!< If set, it overloads the default function that configures LCD position
        HW_LCDC_EXT_CLK ext_clk;                        //!< External clock frequency, \ref HW_LCDC_EXT_CLK
        bool te_enable;                                 //!< Tearing effect input availability from the LCD
        HW_LCDC_TE te_mode;                             //!< Tearing effect detection method
} ad_lcdc_driver_conf_t;

/**
 * \brief LCDC controller configuration
 *
 * Configuration of LCDC controller
 */
typedef struct {
        const ad_lcdc_io_conf_t *io;                    //!< I/O configuration
        const ad_lcdc_driver_conf_t *drv;               //!< Driver configuration
} ad_lcdc_controller_conf_t;

typedef struct ad_lcdc_device_data ad_lcdc_device_data;

/**
 * \name                Predefined command sequence
 *****************************************************************************************
 *
 * LCD controller adapter enables the creation and execution of pre-defined command sequences to
 * facilitate the LCD initialization and other common LCD actions (i.e. enable or disable of the
 * LCD).
 * The commands supported can be divided in the following:
 *      - Delays
 *      - GPIO manipulation
 *      - MIPI DCS commands with / without parameters
 *      - JDI / Sharp type commands
 *      - External clock manipulation
 *
 * To execute the command sequences, the function \ref ad_lcdc_execute_cmds() must be called
 * providing the memory space containing the commands and also the total size of the commands in
 * bytes.
 *
 * The following macros can be used to create the commands and their corresponding parameters.
 *
 * \{
 */
#define LCDC_DELAY_US(us)                      LCDC_TAG_DELAY_US, ((us) & 0xFF), ((us >> 8) & 0xFF)             //!< Delay for the specified amount of microseconds
#define LCDC_DELAY_MS(ms)                      LCDC_TAG_DELAY_MS, ((ms) & 0xFF), ((ms >> 8) & 0xFF)             //!< Delay for the specified amount of milliseconds
#define LCDC_GPIO_SET_ACTIVE(port, pin)        LCDC_TAG_GPIO_SET_ACTIVE, \
                                                       (((port) << HW_GPIO_PIN_BITS) | \
                                                       ((pin) & ((1 << HW_GPIO_PIN_BITS) - 1)))                 //!< Set the corresponding GPIO active
#define LCDC_GPIO_SET_INACTIVE(port, pin)      LCDC_TAG_GPIO_SET_INACTIVE, \
                                                       (((port) << HW_GPIO_PIN_BITS) | \
                                                       ((pin) & ((1 << HW_GPIO_PIN_BITS) - 1)))                 //!< Set the corresponding GPIO inactive
#define LCDC_MIPI_CMD(cmd)                     LCDC_TAG_MIPI_CMD, ((cmd) & 0xFF)                                //!< Send a MIPI DCS command to the LCD
#define LCDC_MIPI_DATA(data)                   LCDC_TAG_MIPI_PARAM, ((data) & 0xFF)                             //!< Send a MIPI DCS parameter to the LCD
#define LCDC_MIPI_CMD_DATA(cmd, ...)           LCDC_TAG_MIPI_CMD_PARAM, ((sizeof((int[]){0, \
                                                        ## __VA_ARGS__})/sizeof(int)) - 1), \
                                                        cmd, ## __VA_ARGS__                                     //!< Send a MIPI DCS command with parameters to the LCD
#define LCDC_GEN_CMD_DATA(cmd_len, ...)        LCDC_TAG_GEN_CMD_PARAM, cmd_len, ((sizeof((int[]){0, \
                                                       ## __VA_ARGS__})/sizeof(int)) - cmd_len - 1), \
                                                       ## __VA_ARGS__                                           //!< Send a generic command to the LCD
#define LCDC_MIPI_ENABLE()                     LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_EXIT_SLEEP_MODE),\
                                                       LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_SET_DISPLAY_ON)      //!< Enable the LCD over MIPI DCS
#define LCDC_MIPI_DISABLE()                    LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_SET_DISPLAY_OFF,\
                                                       LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_ENTER_SLEEP_MODE)    //!< Disable the LCD over MIPI DCS
#define LCDC_MIPI_SET_MODE(mode)               LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_SET_PIXEL_FORMAT,\
                                                       ((mode) & 0xFF))                                         //!< Set color mode of the LCD over MIPI DCS
#define LCDC_MIPI_SET_POSITION(sx, sy, ex, ey) LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_SET_COLUMN_ADDRESS, \
                                                       (((sx) >> 8) & 0xFF), ((sx) & 0xFF), \
                                                       (((ex) >> 8) & 0xFF), ((ex) & 0xFF)), \
                                               LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_SET_PAGE_ADDRESS, \
                                                       (((sy) >> 8) & 0xFF), ((sy) & 0xFF), \
                                                       (((ey) >> 8) & 0xFF), ((ey) & 0xFF))                     //!< Set screen area that is being updated over MIPI DCS
#define LCDC_MIPI_SET_PARTIAL(sx, sy, ex, ey)  LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_SET_PARTIAL_COLUMNS, \
                                                       (((sx) >> 8) & 0xFF), ((sx) & 0xFF), \
                                                       (((ex) >> 8) & 0xFF), ((ex) & 0xFF))), \
                                               LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_SET_PARTIAL_ROWS, \
                                                       (((sy) >> 8) & 0xFF), ((sy) & 0xFF), \
                                                       (((ey) >> 8) & 0xFF), ((ey) & 0xFF)))                    //!< Set screen area that is active over MIPI DCS
#define LCDC_MIPI_SW_RST()                     LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_SOFT_RESET)                  //!< Perform a SW reset of the LCD over MIPI DCS
#define LCDC_MIPI_EXIT_INVERT()                LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_EXIT_INVERT_MODE)            //!< Stop inversion mode of LCD over MIPI DCS
#define LCDC_MIPI_SET_ADDR_MODE(mode)          LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_SET_ADDRESS_MODE, \
                                                       ((mode) & 0xFF))                                         //!< Set the data mode of the LCD over MIPI DCS
#define LCDC_MIPI_SET_TEAR_ON(mode)            LCDC_MIPI_CMD_DATA(HW_LCDC_MIPI_DCS_SET_TEAR_ON,\
                                                       ((mode) & 0xFF))                                         //!< Set tear effect output sync of the LCD over MIPI DCS
#define LCDC_JDI_CMD(cmd)                      LCDC_TAG_JDI_CMD, ((cmd) & 0xFF)                                 //!< Send a JDI / Sharp command to the LCD
#define LCDC_JDI_BLINKOFF()                    LCDC_TAG_JDI_CMD, HW_LCDC_JDIS_CMD_BLINKOFF                      //!< Stop blinking of the LCD over JDI / Sharp interface
#define LCDC_JDI_BLINKBLACK()                  LCDC_TAG_JDI_CMD, HW_LCDC_JDIS_CMD_BLINKBLACK                    //!< Start blinking black of the LCD over JDI / Sharp interface
#define LCDC_JDI_BLINKWHITE()                  LCDC_TAG_JDI_CMD, HW_LCDC_JDIS_CMD_BLINKWHITE                    //!< Start blinking white of the LCD over JDI / Sharp interface
#define LCDC_JDI_BLINKINVERT()                 LCDC_TAG_JDI_CMD, HW_LCDC_JDIS_CMD_BLINKINVERT                   //!< Start blinking inverted of the LCD over JDI / Sharp interface
#define LCDC_JDI_CLEAR()                       LCDC_TAG_JDI_CMD, HW_LCDC_JDIS_CMD_CLEAR                         //!< Clear the LCD over JDI / Sharp interface
#define LCDC_EXT_CLK_SET(enable)               LCDC_TAG_EXT_CLK, ((enable) & 0xFF)                              //!< Enable / disable the external clock generation
/** \} */

/**
 * \brief Initialize adapter
 *
 * \note: It should ONLY be called by the system.
 */
void ad_lcdc_init(void);

/**
 * \brief Open LCDC controller
 *
 * This function:
 * - Acquires the resources needed for using the controller
 * - Configures the controller interface IOs
 * - Initializes the drivers associated with the controller
 *
 * \param [in] conf             Controller configuration
 *
 * \return !=0: handle that should be used in subsequent API calls, NULL: error
 *
 * \note The function will block until it acquires all controller resources
 *
 * \note If required by the device's selected frequency (\ref HW_LCDC_FREQ), this function tries to
 * set the system clock to corresponding clock. Function \ref ad_lcdc_close() has to be called to
 * undo the setting of the system clock frequency. If the clock is not successfully set, this
 * function returns a NULL pointer indicating the failure.
 *
 * \sa CONFIG_LCDC_AUTO_CONFIGURE_CLOCKS
 */
ad_lcdc_handle_t ad_lcdc_open(const ad_lcdc_controller_conf_t *conf);

/**
 * \brief Reconfigure LCD controller
 *
 * This function will apply a new LCDC driver configuration.
 *
 * \param [in] handle          Handle returned from ad_lcdc_open()
 * \param [in] conf            New driver configuration
 *
 * \sa ad_lcdc_open()
 *
 * \return 0: success, <0: error code
 */
 int ad_lcdc_reconfig(ad_lcdc_handle_t handle, const ad_lcdc_driver_conf_t *conf);

/**
 * \brief Close LCD controller
 *
 * This function:
 * - Aborts ongoing transactions
 * - De-initializes the drivers associated with the controller
 * - Resets controller interface IOs (as specified in ad_lcdc_open())
 * - Releases the controller resources
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] force            Force close even if controller is busy
 *
 * \sa ad_lcdc_open()
 *
 * \return 0: success, <0: error code
 *
 * \warning It is important to close the device when it is not needed since it will release the PLL
 * clock so it can be turned off and save power.
 */
int ad_lcdc_close(ad_lcdc_handle_t handle, bool force);

/**
* \brief Initialize controller pins to on / off IO configuration
*
* This function should be called for setting the pins to the correct level before external
* devices are powered up (e.g on system init). It does not need to be called before every
* ad_lcdc_open() call.
*
* \param [in] io                Controller IO configuration
* \param [in] state             On/off IO configuration
*
* \return 0: success, <0: error code
*/
int ad_lcdc_io_config (const ad_lcdc_io_conf_t *io, AD_IO_CONF_STATE state);

/**
 * \brief Execute a sequence of commands declared using the corresponding tags and macros
 * \sa LCDC_TAG
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] cmds             Command sequence
 * \param [in] len              Length in bytes of sequence
 *
 * \sa ad_lcdc_open()
 * \sa ad_lcdc_close()
 *
 * \return 0 on success, <0: error
 */
int ad_lcdc_execute_cmds(ad_lcdc_handle_t handle, const uint8_t *cmds, size_t len);

/**
 * \brief Send DCS command with parameters to the LCD
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] cmd              DCS command to be sent
 * \param [in] params           Command parameters to be sent
 * \param [in] param_len        Command parameters length in bytes
 *
 * \sa ad_lcdc_open()
 * \sa ad_lcdc_close()
 *
 * \return 0 on success, otherwise error
 */
int ad_lcdc_dcs_cmd_params(ad_lcdc_handle_t handle, HW_LCDC_MIPI_DCS cmd, const uint8_t *params, size_t param_len);

/**
 * \brief Execute a read operation to fetch data from the LCD
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] cmd              DCS read command to be sent
 * \param [out] data            Buffer for incoming data
 * \param [in] data_len         Length of data to be read
 * \param [in] dummy_ticks      Number of dummy ticks between command and data (if applicable)
 *
 * \sa ad_lcdc_open()
 * \sa ad_lcdc_close()
 *
 * \return >=0: length of bytes read, <0: error
 */
int ad_lcdc_dcs_read(ad_lcdc_handle_t handle, HW_LCDC_MIPI_DCS cmd, uint8_t *data, size_t data_len, size_t dummy_ticks);

/**
 * \brief Returns the LLD status of the completed operation
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 *
 * \sa AD_LCDC_ERROR_LLD_ERROR
 *
 * \return LLD status
 */
int ad_lcdc_get_lld_status(ad_lcdc_handle_t handle);

/**
 * \brief Set external clock generation state.
 *
 * This function should be called to enable/disable the generation of the external clock which is
 * used for the internal refresh of the LCD. This function sets the clock frequency declared,
 * enables COM power domain and configures latches of the configured pins.
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] enable           Enable / disable the clock generation
 *
 * \sa ad_lcdc_open()
 * \sa ad_lcdc_close()
 *
 * \return 0 on success, <0: error
 */
int ad_lcdc_set_external_clock(ad_lcdc_handle_t handle, bool enable);

/**
 * \brief Set screen dimensions that are updated by the controller
 *
 * If provided parameters are not valid, they are modified accordingly. Applied dimensions are
 * provided by modifying the input parameter.
 *
 * \warning LCD must support the partial update of the corresponding dimension
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in,out] frame        Frame dimensions to be updated
 *
 * \sa ad_lcdc_open()
 * \sa ad_lcdc_close()
 *
 * \return 0 on success, <0: error
 */
int ad_lcdc_set_partial_update(ad_lcdc_handle_t handle, hw_lcdc_frame_t *frame);

/**
 * \brief Exit screen partial update mode and set the complete screen to be updated by the
 * controller
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 *
 * \note Function is equivalent to calling \ref ad_lcdc_set_partial_update() with full screen frame
 * dimensions
 *
 * \sa ad_lcdc_open()
 * \sa ad_lcdc_close()
 *
 * \return 0 on success, <0: error
 */
int ad_lcdc_exit_partial_update(ad_lcdc_handle_t handle);

/**
 * \brief Set an offset of the first pixel of the display in the display's memory.
 *
 * Function enables the usage of displays that their first row / column does not correspond to the
 * driving IC's pixel memory first row / column.
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] offsetx          Horizontal offset of first pixel in display's memory
 * \param [in] offsety          Vertical offset of first pixel in display's memory
 *
 * \note Function has an effect only on MIPI interfaces.
 *
 * \sa ad_lcdc_open()
 * \sa ad_lcdc_close()
 *
 * \return 0 on success, <0: error
 */
int ad_lcdc_set_display_offset(ad_lcdc_handle_t handle, int16_t offsetx, int16_t offsety);

/**
 * \brief Set layer configuration
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] layer_no         Layer index of hardware layer
 * \param [in] enable           Set if layer is enabled (visible)
 * \param [in] layer            Layer parameters, i.e. size, color depth etc
 *
 * \sa ad_lcdc_open()
 * \sa ad_lcdc_close()
 *
 * \return 0 on success, <0: error
 */
int ad_lcdc_setup_layer(ad_lcdc_handle_t handle, HW_LCDC_LAYER layer_no, bool enable, const hw_lcdc_layer_t *layer);

/**
 * \brief Perform a screen contents update with the layer parameters
 *
 * This function is blocking. It may wait first for device access, then it waits until transaction
 * is completed. It has a timeout parameter to guard the transaction completion and not the
 * acquisition of the device.
 *
 * Example usage:
 * \code{.c}
 * {
 *   ad_lcdc_handle_t hndl = ad_lcdc_open(LCDC_DEVICE);
 *   if (hndl == NULL) {
 *     // Handle open error
 *   }
 *   ad_lcdc_setup_layer(hndl, 0, true, layer);
 *   while (1) {
 *     int ret;
 *     ret = ad_lcdc_draw_screen(hndl, OS_EVENT_FOREVER);
 *     OS_ASSERT(ret == 0);
 *     ...
 *   }
 * }
 * \endcode
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] timeout          Timeout of the transaction
 *
 * \sa ad_lcdc_open()
 * \sa ad_lcdc_close()
 *
 * \return 0 on success, <0: error
 */
int ad_lcdc_draw_screen(ad_lcdc_handle_t handle, OS_TICK_TIME timeout);

/**
 * \brief Perform a screen contents update with the layer parameters
 *
 * This function is not blocking. It may block only waiting for device access, then it sets up the
 * transaction and returns immediately.
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] cb               Callback to call after transaction is over (from ISR context)
 * \param [in] user_data        User data to pass to \p cb
 *
 * \sa ad_lcdc_open()
 * \sa ad_lcdc_close()
 *
 * \note The callback is called from within LCDC ISR and at that time resources are released.
 *
 * \warning Do not call this function consecutively without guaranteeing that the previous
 *          asynchronous transaction has completed.
 *
 * \warning After the callback is called, it is not guaranteed that the scheduler will give
 *          control to the task waiting for this transaction to complete. This is important to
 *          consider if more than one tasks are using this API.
 *
 * \return 0 on success, <0: error
 */
int ad_lcdc_draw_screen_async(ad_lcdc_handle_t handle, ad_lcdc_user_cb cb, void *user_data);

/**
 * \brief Initialize a continuous update of the LCD
 *
 * This function only sets up the continuous mode of the LCD update. In this mode, LCD controller
 * continuously updates the LCD even if the contents of the frame remain constant. In order to
 * update the screen contents, a callback function is provided by the application (with corresponding
 * user data) which is called every time the frame should be updated.
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 * \param [in] cb               Callback to call after transaction is over (from ISR context)
 * \param [in] user_data        User data to pass to \p cb
 *
 * \sa ad_lcdc_open()
 * \sa ad_lcdc_close()
 *
 * \note The callback is called from within LCDC ISR.
 *
 * \warning The continuous update mode ends only with a call to
 * \ref ad_lcdc_continuous_update_stop() and this is when the resources are released.
 *
 * \return 0 on success, <0: error
 */
int ad_lcdc_continuous_update_start(ad_lcdc_handle_t handle, ad_lcdc_user_cb cb, void *user_data);

/**
 * \brief Stop the continuous update of the LCD
 *
 * This function disables the continuous frame generation of the LCD controller and disables the
 * corresponding interrupt.
 *
 * \param [in] handle           Handle returned from ad_lcdc_open()
 *
 * \sa ad_lcdc_open()
 * \sa ad_lcdc_close()
 *
 * \return 0 on success, <0: error
 */
int ad_lcdc_continuous_update_stop(ad_lcdc_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* dg_configLCDC_ADAPTER */

#endif /* AD_LCDC_H_ */

/**
 * \}
 * \}
 */
