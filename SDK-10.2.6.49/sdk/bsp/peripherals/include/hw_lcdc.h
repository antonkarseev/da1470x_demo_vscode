/**
 * \addtogroup PLA_DRI_PER_ANALOG
 * \{
 * \addtogroup HW_LCD_CONTROLLER LCD Controller Driver
 * \{
 * \brief LCD Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_lcdc.h
 *
 * @brief Definition of API for the LCD Controller Low Level Driver.
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_LCDC_H
#define HW_LCDC_H


#if dg_configUSE_HW_LCDC

#include <sdk_defs.h>

/**
 * \brief Access LCDC register field mask
 *
 * \param [in] reg              Register
 * \param [in] field            Register field
 */
#define HW_LCDC_REG_MSK(reg, field)                     REG_MSK(LCDC, reg, field)

/**
 * \brief Access LCDC register field position
 *
 * \param [in] reg              Register
 * \param [in] field            Register field
 */
#define HW_LCDC_REG_POS(reg, field)                     REG_POS(LCDC, reg, field)

/**
 * \brief Set the value of an LCDC register field.
 *
 * \param [in] reg              Register
 * \param [in] field            Register field
 * \param [in] new_val          Value to write
 */
#define HW_LCDC_REG_SETF(reg, field, new_val)           REG_SETF(LCDC, reg, field, new_val)

/**
 * \brief Return the value of an LCDC register field.
 *
 * \param [in] reg              Register
 * \param [in] field            Register field
 */
#define HW_LCDC_REG_GETF(reg, field)                    REG_GETF(LCDC, reg, field)

/**
 * \brief Set the value of an LCDC register field to a variable
 *
 * \param [in] reg              Register
 * \param [in] field            Register field
 * \param [in,out] var          Variable to set
 * \param [in] val              Value to set
 */
#define HW_LCDC_REG_SET_FIELD(reg, field, var, val)     REG_SET_FIELD(LCDC, reg, field, var, val)

/**
 * \brief Get the value of an LCDC register field from a variable
 *
 * \param [in] reg              Register
 * \param [in] field            Register field
 * \param [in] var              Variable to get
 */
#define HW_LCDC_REG_GET_FIELD(reg, field, var)          REG_GET_FIELD(LCDC, reg, field, var)

/**
 * \brief Set the value of an LCDC layer register
 *
 * \param [in] reg              Register
 * \param [in] layer            Layer index
 * \param [in] val              Value to set
 */
#define HW_LCDC_SET_LAYER_REG(reg, layer, val)          (*REG_GET_ADDR_INDEXED(LCDC, reg, 0x20, layer) = val)

/**
 * \brief Get the value of an LCDC layer register
 *
 * \param [in] reg              Register
 * \param [in] layer            Layer index
 */
#define HW_LCDC_GET_LAYER_REG(reg, layer)               (*REG_GET_ADDR_INDEXED(LCDC, reg, 0x20, layer))

/**
 * \brief Set the value of an LCDC layer register field.
 *
 * \param [in] reg              Register
 * \param [in] field            Register field
 * \param [in] new_val          Value to write
 * \param [in] layer            Layer index
 */
#define HW_LCDC_SETF_LAYER_REG(reg, field, new_val, layer) \
        (HW_LCDC_SET_LAYER_REG(reg, layer,(HW_LCDC_GET_LAYER_REG(reg, layer) & ~HW_LCDC_REG_MSK(reg, field)) | \
                (HW_LCDC_REG_MSK(reg, field) & ((new_val) << HW_LCDC_REG_POS(reg, field)))))

/**
 * \brief Return the value of an LCDC layer register field.
 *
 * \param [in] reg              Register
 * \param [in] field            Register field
 * \param [in] layer            Layer index
 */
#define HW_LCDC_GETF_LAYER_REG(reg, field, layer) \
        ((HW_LCDC_GET_LAYER_REG(reg, layer) & HW_LCDC_REG_MSK(reg, field)) >> HW_LCDC_REG_POS(reg, field))

/**
 * \brief Number of palette entries
 */
#define HW_LCDC_PALETTE_ENTRIES                                 (256U)

/**
 * \brief Macro to construct a palette entry
 *
 * \param [in] r                Red color channel
 * \param [in] g                Green color channel
 * \param [in] b                Blue color channel
 */
#define HW_LCDC_PALETTE_ENTRY(r, g, b)                          ((((r) << HW_LCDC_REG_POS(LCDC_PALETTE_BASE, PALLETE_R)) & HW_LCDC_REG_MSK(LCDC_PALETTE_BASE, PALLETE_R)) | \
                                                                 (((g) << HW_LCDC_REG_POS(LCDC_PALETTE_BASE, PALLETE_G)) & HW_LCDC_REG_MSK(LCDC_PALETTE_BASE, PALLETE_G)) | \
                                                                 (((b) << HW_LCDC_REG_POS(LCDC_PALETTE_BASE, PALLETE_B)) & HW_LCDC_REG_MSK(LCDC_PALETTE_BASE, PALLETE_B)))

/*
 * Definitions for registers with overlapping fields
 */
#define LCDC_LCDC_FMTCTRL_REG_DBIB_GE_Pos                       (30UL)          /*!< DBIB_GE (Bit 30) */
#define LCDC_LCDC_FMTCTRL_REG_DBIB_GE_Msk                       (0xC0000000UL)  /*!< DBIB_GE (Bitfield-Mask) */
#define LCDC_LCDC_FMTCTRL_REG_DBIB_READ_C_Pos                   (16UL)          /*!< DBIB read cycles (Bit 16) */
#define LCDC_LCDC_FMTCTRL_REG_DBIB_READ_C_Msk                   (0x3F0000UL)    /*!< DBIB read cycles (Bitfield-Mask) */
#define LCDC_LCDC_FMTCTRL_REG_DBIB_CT_Pos                       (0UL)           /*!< DBIB_CT (Bit 8) */
#define LCDC_LCDC_FMTCTRL_REG_DBIB_CT_Msk                       (0xFFFFUL)      /*!< DBIB_CT (Bitfield-Mask) */
#define LCDC_LCDC_FMTCTRL_REG_JDIP_DPI_MUTE_Pos                 (31UL)          /*!< JDI parallel mute DPI outputs (Bit 31) */
#define LCDC_LCDC_FMTCTRL_REG_JDIP_DPI_MUTE_Msk                 (0x80000000UL)  /*!< JDI parallel mute DPI outputs (Bitfield-Mask) */
#define LCDC_LCDC_FMTCTRL_REG_JDIP_DPI_MASK_READY_Pos           (30UL)          /*!< JDI parallel mask DPI ready input (Bit 30) */
#define LCDC_LCDC_FMTCTRL_REG_JDIP_DPI_MASK_READY_Msk           (0x40000000UL)  /*!< JDI parallel mask DPI ready input (Bitfield-Mask) */
#define LCDC_LCDC_FMTCTRL_REG_JDIP_HST_WIDTH_Pos                (26UL)          /*!< JDI parallel HST width (Bit 26) */
#define LCDC_LCDC_FMTCTRL_REG_JDIP_HST_WIDTH_Msk                (0x1C000000UL)  /*!< JDI parallel HST width (Bitfield-Mask) */
#define LCDC_LCDC_FMTCTRL_REG_JDIP_HST_OFFSET_Pos               (23UL)          /*!< JDI parallel HST offset (Bit 23) */
#define LCDC_LCDC_FMTCTRL_REG_JDIP_HST_OFFSET_Msk               (0x3800000UL)   /*!< JDI parallel HST offset (Bitfield-Mask) */
#define LCDC_LCDC_FMTCTRL_REG_JDIP_VST_WIDTH_Pos                (13UL)          /*!< JDI parallel VST width (Bit 13) */
#define LCDC_LCDC_FMTCTRL_REG_JDIP_VST_WIDTH_Msk                (0x7FE000UL)    /*!< JDI parallel VST width (Bitfield-Mask) */
#define LCDC_LCDC_FMTCTRL_REG_JDIP_VST_OFFSET_Pos               (3UL)           /*!< JDI parallel VST offset (Bit 3) */
#define LCDC_LCDC_FMTCTRL_REG_JDIP_VST_OFFSET_Msk               (0x1FF8UL)      /*!< JDI parallel VST offset (Bitfield-Mask) */

#define LCDC_LCDC_FMTCTRL_2_REG_DBIB_BLX_Pos                    (0UL)           /*!< DBIB X blanking period length (Bit 0) */
#define LCDC_LCDC_FMTCTRL_2_REG_DBIB_BLX_Msk                    (0xFFFFUL)      /*!< DBIB X blanking period length (Bitfield-Mask) */
#define LCDC_LCDC_FMTCTRL_2_REG_JDIP_XRST_OFFSET_Pos            (20UL)          /*!< JDI parallel XRST offset (Bit 20) */
#define LCDC_LCDC_FMTCTRL_2_REG_JDIP_XRST_OFFSET_Msk            (0x3FF00000UL)  /*!< JDI parallel XRST offset (Bitfield-Mask) */
#define LCDC_LCDC_FMTCTRL_2_REG_JDIP_ENB_WIDTH_Pos              (10UL)          /*!< JDI parallel ENB width (Bit 10) */
#define LCDC_LCDC_FMTCTRL_2_REG_JDIP_ENB_WIDTH_Msk              (0xFFC00UL)     /*!< JDI parallel ENB width (Bitfield-Mask) */
#define LCDC_LCDC_FMTCTRL_2_REG_JDIP_ENB_OFFSET_Pos             (0UL)           /*!< JDI parallel ENB offset (Bit 0) */
#define LCDC_LCDC_FMTCTRL_2_REG_JDIP_ENB_OFFSET_Msk             (0x3FFUL)       /*!< JDI parallel ENB offset (Bitfield-Mask) */

#define LCDC_LCDC_DBIB_RDAT_REG_DBIB_READ_C_Pos                 (30UL)          /*!< Number of read cycles (Bit 30) */
#define LCDC_LCDC_DBIB_RDAT_REG_DBIB_READ_C_Msk                 (0xC0000000UL)  /*!< Number of read cycles (Bitfield-Mask) */

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief MIPI Display Bus Interface command type
 */
typedef enum {
        HW_LCDC_MIPI_CMD,        //!< New command to the LCD
        HW_LCDC_MIPI_CMD_FRAME,  //!< New command to the LCD to update frame contents
        HW_LCDC_MIPI_STORE_BADDR,//!< Store value to the line register
        HW_LCDC_MIPI_DATA,       //!< Additional data to a command
        HW_LCDC_MIPI_READ,       //!< Read command
} HW_LCDC_MIPI;

/**
 * \brief LCD controller MIPI configuration type
 */
typedef enum {
        HW_LCDC_MIPI_CFG_DBI_EN        = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DBIB_INTERFACE_EN),    //!< Enable DBI interface
        HW_LCDC_MIPI_CFG_FRC_CSX_0     = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DBIB_CSX_CFG_EN),      //!< Force Chip Select output (to 0)
        HW_LCDC_MIPI_CFG_SPI_CSX_V     = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DBIB_CSX_CFG),         //!< Invert Chip Select control
        HW_LCDC_MIPI_CFG_FRC_CSX_1     = HW_LCDC_MIPI_CFG_FRC_CSX_0 | HW_LCDC_MIPI_CFG_SPI_CSX_V,  //!< Force Chip Select output (to 1)
        HW_LCDC_MIPI_CFG_TE_DIS        = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DBIB_TE_DISABLE),      //!< Disable sampling of tearing effect signal
        HW_LCDC_MIPI_CFG_SPIDC_DQSPI   = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, SPI_DC_AS_SPI_SD1),    //!< Enable usage of SPI_DC as SPI_SD1
        HW_LCDC_MIPI_CFG_RSTN_DBI_SPI  = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DBIB_FORCE_IDLE),      //!< Force DBI interface to idle state
        HW_LCDC_MIPI_CFG_RESX          = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DBIB_RESX_OUT_EN),     //!< Enable reset signal of MIPI
        HW_LCDC_MIPI_CFG_SPIX_REV      = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, SUB_PIXEL_REVERSE),    //!< Reverse sub pixel order
        HW_LCDC_MIPI_CFG_SPI3          = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, SPI3_EN),              //!< Enable SPI3 interface
        HW_LCDC_MIPI_CFG_SPI4          = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, SPI4_EN),              //!< Enable SPI4 interface
        HW_LCDC_MIPI_CFG_EN_STALL      = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DBIB_BACK_PRESSURE_EN),//!< Enable back pressure for DBI interface
        HW_LCDC_MIPI_CFG_SPI_CPHA      = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, SPI_CLK_PHASE),        //!< Phase of SPI clock
        HW_LCDC_MIPI_CFG_SPI_CPOL      = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, SPI_CLK_POLARITY),     //!< Polarity of SPI clock
        HW_LCDC_MIPI_CFG_SPI_JDI       = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, SPID_JDI),             //!< Enable line addressing between lines
        HW_LCDC_MIPI_CFG_SPI_HOLD      = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, CMD_DATA_AS_HEADER),   //!< Enable the hold of commands to bind commands and data
        HW_LCDC_MIPI_CFG_INV_ADDR      = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, BIT_ORDER_ADDR_INVERT),//!< Enable horizontal line address inversion (MSB to LSB)
        HW_LCDC_MIPI_CFG_SCAN_ADDR     = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, SPI_2BYTE_ADDR),       //!< 2 byte address is sent with each line
        HW_LCDC_MIPI_CFG_PIXCLK_OUT_EN = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, PIX_CLK_AT_DBIB_CLK),  //!< Expose pixel generation clock on the DBIB_CLK
        HW_LCDC_MIPI_CFG_EXT_CTRL      = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, EXT_CTRL_EN),          //!< Enable external control
        HW_LCDC_MIPI_CFG_BLANKING_EN   = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, HORIZONTAL_BLANK_EN),  //!< Enable horizontal blanking
        HW_LCDC_MIPI_CFG_DSPI_SPIX     = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DUAL_SPI_SUBPIXEL_EXTRACT_EN),//!< Enable Dual SPI subpixel transaction
        HW_LCDC_MIPI_CFG_QSPI          = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, QUAD_SPI_EN),          //!< Enable Quad SPI
        HW_LCDC_MIPI_CFG_DSPI          = HW_LCDC_REG_MSK(LCDC_DBIB_CFG_REG, DUAL_SPI_EN),          //!< Enable Dual SPI
} HW_LCDC_MIPI_CFG;

/**
 * \brief LCD controller mode configuration type
 */
typedef enum {
        HW_LCDC_MODE_DISABLE       = 0,                                                     //!< Disable mode
        HW_LCDC_MODE_ENABLE        = HW_LCDC_REG_MSK(LCDC_MODE_REG, MODE_EN),               //!< Enable continuous mode
        HW_LCDC_MODE_NEG_V         = HW_LCDC_REG_MSK(LCDC_MODE_REG, VSYNC_POL),             //!< Negative VSYNC polarity
        HW_LCDC_MODE_NEG_H         = HW_LCDC_REG_MSK(LCDC_MODE_REG, HSYNC_POL),             //!< Negative HSYNC polarity
        HW_LCDC_MODE_NEG_DE        = HW_LCDC_REG_MSK(LCDC_MODE_REG, DE_POL),                //!< Negative DE polarity
        HW_LCDC_MODE_SINGLEV       = HW_LCDC_REG_MSK(LCDC_MODE_REG, VSYNC_SCPL),            //!< VSYNC for a single cycle per line
        HW_LCDC_MODE_BLANK         = HW_LCDC_REG_MSK(LCDC_MODE_REG, FORCE_BLANK),           //!< Force output to blank
        HW_LCDC_MODE_ONE_FRAME     = HW_LCDC_REG_MSK(LCDC_MODE_REG, SFRAME_UPD),            //!< Single frame update
        HW_LCDC_MODE_FORMAT_CLK    = HW_LCDC_REG_MSK(LCDC_MODE_REG, PIXCLKOUT_SEL),         //!< Select pixel clock source
        HW_LCDC_MODE_INVPIXCLK     = HW_LCDC_REG_MSK(LCDC_MODE_REG, PIXCLKOUT_POL),         //!< Pixel clock out polarity
        HW_LCDC_MODE_PALETTE       = HW_LCDC_REG_MSK(LCDC_MODE_REG, GLOBAL_GAMMA_EN),       //!< Enable global gamma correction
        HW_LCDC_MODE_MIPI_OFF      = HW_LCDC_REG_MSK(LCDC_MODE_REG, DBIB_OFF),              //!< MIPI off
        HW_LCDC_MODE_UDERRUN_PREV  = HW_LCDC_REG_MSK(LCDC_MODE_REG, UNDERRUN_PREVENTION_EN),/*!< Enable underrun prevention
                                                                                                 for interfaces that support this */
        HW_LCDC_MODE_OUTP_OFF      = HW_LCDC_REG_MSK(LCDC_MODE_REG, FORM_OFF),              //!< Formating off
        HW_LCDC_MODE_SCANDOUBLE    = HW_LCDC_REG_MSK(LCDC_MODE_REG, DSCAN),                 //!< Enable double horizontal scan
        HW_LCDC_MODE_TESTMODE      = HW_LCDC_REG_MSK(LCDC_MODE_REG, TMODE),                 //!< Enable test mode

        HW_LCDC_MODE_P_RGB         = 0 << HW_LCDC_REG_POS(LCDC_MODE_REG, OUT_MODE),         //!< Parallel RGB
        HW_LCDC_MODE_JDIMIP        = 8 << HW_LCDC_REG_POS(LCDC_MODE_REG, OUT_MODE),         //!< JDI MIP
} HW_LCDC_MODE;

/**
 * \brief MIPI Display Command Set
 */
typedef enum {
        HW_LCDC_MIPI_DCS_NOP                     = 0x00,//!< No Operation
        HW_LCDC_MIPI_DCS_SOFT_RESET              = 0x01,//!< Software Reset
        HW_LCDC_MIPI_DCS_GET_COMPRESSION_MODE    = 0x03,//!< Get the current compression mode
        HW_LCDC_MIPI_DCS_GET_RED_CHANNEL         = 0x06,//!< Get the red component of the pixel at (0, 0).
        HW_LCDC_MIPI_DCS_GET_GREEN_CHANNEL       = 0x07,//!< Get the green component of the pixel at (0, 0).
        HW_LCDC_MIPI_DCS_GET_BLUE_CHANNEL        = 0x08,//!< Get the blue component of the pixel at (0, 0).
        HW_LCDC_MIPI_DCS_GET_POWER_MODE          = 0x0A,//!< Get the current power mode.
        HW_LCDC_MIPI_DCS_GET_ADDRESS_MODE        = 0x0B,/*!< Get the data order for transfers from the Host
                                                             to the display module and from the frame memory
                                                             to the display device. */
        HW_LCDC_MIPI_DCS_GET_PIXEL_FORMAT        = 0x0C,//!< Get the current pixel format.
        HW_LCDC_MIPI_DCS_GET_DISPLAY_MODE        = 0x0D,//!< Get the current display mode from the peripheral.
        HW_LCDC_MIPI_DCS_GET_SIGNAL_MODE         = 0x0E,//!< Get display module signaling mode.
        HW_LCDC_MIPI_DCS_GET_DIAGNOSTIC_RESULT   = 0x0F,//!< Get Peripheral Self-Diagnostic Result
        HW_LCDC_MIPI_DCS_ENTER_SLEEP_MODE        = 0x10,//!< Power for the display panel is off.
        HW_LCDC_MIPI_DCS_EXIT_SLEEP_MODE         = 0x11,//!< Power for the display panel is on.
        HW_LCDC_MIPI_DCS_ENTER_PARTIAL_MODE      = 0x12,//!< Part of the display area is used for image display.
        HW_LCDC_MIPI_DCS_ENTER_NORMAL_MODE       = 0x13,//!< The whole display area is used for image display.
        HW_LCDC_MIPI_DCS_EXIT_INVERT_MODE        = 0x20,//!< Displayed image colors are not inverted.
        HW_LCDC_MIPI_DCS_ENTER_INVERT_MODE       = 0x21,//!< Displayed image colors are inverted.
        HW_LCDC_MIPI_DCS_SET_GAMMA_CURVE         = 0x26,//!< Selects the gamma curve used by the display device.
        HW_LCDC_MIPI_DCS_SET_DISPLAY_OFF         = 0x28,//!< Blanks the display device.
        HW_LCDC_MIPI_DCS_SET_DISPLAY_ON          = 0x29,//!< Show the image on the display device.
        HW_LCDC_MIPI_DCS_SET_COLUMN_ADDRESS      = 0x2A,//!< Set the column extent.
        HW_LCDC_MIPI_DCS_SET_PAGE_ADDRESS        = 0x2B,//!< Set the page extent.
        HW_LCDC_MIPI_DCS_WRITE_MEMORY_START      = 0x2C,/*!< Transfer image data from the Host Processor to the
                                                             peripheral starting at the location provided by
                                                             HW_LCDC_MIPI_DCS_SET_COLUMN_ADDRESS and
                                                             HW_LCDC_MIPI_DCS_SET_PAGE_ADDRESS. */
        HW_LCDC_MIPI_DCS_WRITE_LUT               = 0x2D,//!< Fills the peripheral look-up table with the provided data.
        HW_LCDC_MIPI_DCS_READ_MEMORY_START       = 0x2E,/*!< Transfer image data from the peripheral to the Host
                                                             Processor interface starting at the location provided
                                                             by HW_LCDC_MIPI_DCS_SET_COLUMN_ADDRESS and
                                                             HW_LCDC_MIPI_DCS_SET_PAGE_ADDRESS. */
        HW_LCDC_MIPI_DCS_SET_PARTIAL_ROWS        = 0x30,/*!< Defines the number of rows in the partial display area
                                                             on the display device. */
        HW_LCDC_MIPI_DCS_SET_PARTIAL_COLUMNS     = 0x31,/*!< Defines the number of columns in the partial display
                                                             area on the display device. */
        HW_LCDC_MIPI_DCS_SET_SCROLL_AREA         = 0x33,/*!< Defines the vertical scrolling and fixed area on
                                                             display device. */
        HW_LCDC_MIPI_DCS_SET_TEAR_OFF            = 0x34,/*!< Synchronization information is not sent from the display
                                                             module to the host processor. */
        HW_LCDC_MIPI_DCS_SET_TEAR_ON             = 0x35,/*!< Synchronization information is sent from the display
                                                             module to the host processor at the start of VFP. */
        HW_LCDC_MIPI_DCS_SET_ADDRESS_MODE        = 0x36,/*!< Set the data order for transfers from the Host to the
                                                             display module and from the frame memory to the display
                                                             device. */
        HW_LCDC_MIPI_DCS_SET_SCROLL_START        = 0x37,//!< Defines the vertical scrolling starting point.
        HW_LCDC_MIPI_DCS_EXIT_IDLE_MODE          = 0x38,//!< Full color depth is used on the display panel.
        HW_LCDC_MIPI_DCS_ENTER_IDLE_MODE         = 0x39,//!< Reduced color depth is used on the display panel.
        HW_LCDC_MIPI_DCS_SET_PIXEL_FORMAT        = 0x3A,//!< Defines how many bits per pixel are used in the interface.
        HW_LCDC_MIPI_DCS_WRITE_MEMORY_CONTINUE   = 0x3C,/*!< Transfer image information from the Host Processor interface
                                                             to the peripheral from the last written location. */
        HW_LCDC_MIPI_DCS_SET_3D_CONTROL          = 0x3D,//!< 3D is used on the display panel
        HW_LCDC_MIPI_DCS_READ_MEMORY_CONTINUE    = 0x3E,/*!< Read image data from the peripheral continuing after the
                                                             last HW_LCDC_MIPI_DCS_READ_MEMORY_CONTINUE or
                                                             HW_LCDC_MIPI_DCS_READ_MEMORY_START. */
        HW_LCDC_MIPI_DCS_GET_3D_CONTROL          = 0x3F,//!< Get display module 3D mode
        HW_LCDC_MIPI_DCS_SET_VSYNC_TIMING        = 0x40,//!< Set VSYNC timing
        HW_LCDC_MIPI_DCS_SET_TEAR_SCANLINE       = 0x44,/*!< Synchronization information is sent from the display module
                                                             to the host processor when the display device refresh
                                                             reaches the provided scan line. */
        HW_LCDC_MIPI_DCS_GET_SCANLINE            = 0x45,//!< Get the current scan line.
        HW_LCDC_MIPI_DCS_SET_DISPLAY_BRIGHTNESS  = 0x51,//!< Set the display brightness value
        HW_LCDC_MIPI_DCS_GET_DISPLAY_BRIGHTNESS  = 0x52,//!< Get the display brightness value
        HW_LCDC_MIPI_DCS_WRITE_CONTROL_DISPLAY   = 0x53,//!< Set the display control
        HW_LCDC_MIPI_DCS_GET_CONTROL_DISPLAY     = 0x54,//!< Get the display control
        HW_LCDC_MIPI_DCS_WRITE_POWER_SAVE        = 0x55,//!< Set the display power save
        HW_LCDC_MIPI_DCS_GET_POWER_SAVE          = 0x56,//!< Get the display power save
        HW_LCDC_MIPI_DCS_SET_CABC_MIN_BRIGHTNESS = 0x5E,//!< Set the content adaptive brightness control minimum brightness
        HW_LCDC_MIPI_DCS_GET_CABC_MIN_BRIGHTNESS = 0x5F,//!< Get the content adaptive brightness control minimum brightness
        HW_LCDC_MIPI_DCS_READ_DDB_START          = 0xA1,//!< Read the DDB from the provided location.
        HW_LCDC_MIPI_DCS_READ_DDB_CONTINUE       = 0xA8,//!< Continue reading the DDB from the last read location.
} HW_LCDC_MIPI_DCS;

/**
 * \brief Output color mode/format of the LCD controller
 */
typedef enum {
        HW_LCDC_OCM_8RGB111_1 = 0x01 | (0U << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_DATA_ORDER)), //!< 0 0 R G B R' G' B'
        HW_LCDC_OCM_8RGB111_2 = 0x01 | (2U << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_DATA_ORDER)), //!< R G B 0 R' G' B' 0
        HW_LCDC_OCM_8RGB111_3 = 0x01 | (1U << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_DATA_ORDER)), //!< 0 R G B 0 R' G' B'
        HW_LCDC_OCM_RGB111    = 0x01 | (4U << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_DATA_ORDER)), //!< R G B  R' G' B' ...
        HW_LCDC_OCM_L1        = 0x01 | (3U << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_DATA_ORDER)), //!< D D' D'' ...
        HW_LCDC_OCM_8RGB332   = 0x02,                                                               //!< R[2-0]G[2-0]B[1-0]
        HW_LCDC_OCM_8RGB444   = 0x03,                                                               //!< R[3-0]G[3-0] - B[3-0]R'[3-0] - G'[3-0]B'[3-0]
        HW_LCDC_OCM_8RGB565   = 0x05,                                                               //!< R[4-0]G[5-3] - G[2-0]B[4-0]
        HW_LCDC_OCM_8RGB666   = 0x06,                                                               //!< R[5-0]00 - G[5-0]00 - B[5-0]00
        HW_LCDC_OCM_8RGB666_P = 0x06 | (1U << HW_LCDC_REG_POS(LCDC_DBIB_CFG_REG, DBIB_DATA_ORDER)), //!< R[5-0] - G[5-0] - B[5-0], packed
        HW_LCDC_OCM_8RGB888   = 0x07,                                                               //!< R[7-0] - G[7-0] - B[7-0]

        /**
         * JDI parallel only
         *
         * R1 line: R[1]  R''[1]  ... R[0]  R''[0]  ...
         * R2 line: R'[1] R'''[1] ... R'[0] R'''[0] ...
         * G1 line: G[1]  G''[1]  ... G[0]  G''[0]  ...
         * G2 line: G'[1] G'''[1] ... G'[0] G'''[0] ...
         * B1 line: B[1]  B''[1]  ... B[0]  B''[0]  ...
         * B2 line: B'[1] B'''[1] ... B'[0] B'''[0] ...
         */
        HW_LCDC_OCM_RGB222    = 0x00,
} HW_LCDC_OUTPUT_COLOR_MODE;

/**
 * \brief Layer color format/mode
 */
typedef enum {
        HW_LCDC_LCM_RGB332   = 0x04,//!< R[2-0]G[2-0]B[1-0]
        HW_LCDC_LCM_RGB565   = 0x05,//!< R[4-0]G[5-0]B[4-0]
        HW_LCDC_LCM_RGBA5551 = 0x01,//!< R[4-0]G[4-0]B[4-0]A0
        HW_LCDC_LCM_RGBA4444 = 0x15,//!< R[3-0]G[3-0]B[3-0]A[3-0]
        HW_LCDC_LCM_ARGB4444 = 0x18,//!< A[3-0]R[3-0]G[3-0]B[3-0]
        HW_LCDC_LCM_RGB888   = 0x0b,//!< R[7-0]G[7-0]B[7-0]
        HW_LCDC_LCM_ABGR8888 = 0x0d,//!< A[7-0]B[7-0]G[7-0]R[7-0]
        HW_LCDC_LCM_BGRA8888 = 0x0e,//!< B[7-0]G[7-0]R[7-0]A[7-0]
        HW_LCDC_LCM_RGBA8888 = 0x02,//!< R[7-0]G[7-0]B[7-0]A[7-0]
        HW_LCDC_LCM_ARGB8888 = 0x06,//!< A[7-0]R[7-0]G[7-0]B[7-0]
} HW_LCDC_LAYER_COLOR_MODE;

/**
 * \brief DBI interface configuration
 */
typedef enum {
        HW_LCDC_DBI_INTERFACE_WIDTH_DBIB_8 = 0x00,//!< DBI outputs DBIB of 8 bits width
        HW_LCDC_DBI_INTERFACE_WIDTH_SPI    = 0x03,//!< DBI outputs DBIC types (SPI3 / SPI4)
        HW_LCDC_DBI_INTERFACE_WIDTH_DSPI   = 0x04,//!< DBI outputs Dual SPI
        HW_LCDC_DBI_INTERFACE_WIDTH_QSPI   = 0x05,//!< DBI outputs Quad SPI
        HW_LCDC_DBI_INTERFACE_WIDTH_GPI    = 0x06,//!< DBI outputs GPI
} HW_LCDC_DBI_INTERFACE_WIDTH;

/**
 * \brief Dual SPI flavors
 */
typedef enum {
        HW_LCDC_DSPI_OPT_1P1T2 = 0x00,//!< 1 pixel is sent in 1 transmission over 2 lines
        HW_LCDC_DSPI_OPT_2P3T2 = 0x01,//!< 2 pixels are sent in 3 transmissions over 2 lines
} HW_LCDC_DSPI_OPT;

/**
 * \brief MIPI DBI command width
 */
typedef enum {
        HW_LCDC_CMD_WIDTH_8  = 0x00,//!< 8bit width
        HW_LCDC_CMD_WIDTH_16 = 0x01,//!< 16bit width
        HW_LCDC_CMD_WIDTH_24 = 0x02,//!< 24bit width
        HW_LCDC_CMD_WIDTH_32 = 0x03,//!< 32bit width
} HW_LCDC_CMD_WIDTH;

/**
 * \brief QSPI mode of operation
 *
 * LCDC transmits command, address and data either in single mode (SSS) or in single for command and
 * address and quad for data (SSQ). The latter (SSQ) is only available for frame data, i.e. DMA.
 */
typedef enum {
        HW_LCDC_QSPI_MODE_AUTO,         //!< SSS for parameters, except of pixels which are in SSQ
        HW_LCDC_QSPI_MODE_FORCE_SINGLE, //!< Everything is transmitted in SSS
} HW_LCDC_QSPI_MODE;

/**
 * \brief Dithering options
 */
typedef enum {
        HW_LCDC_DITHER_OFF = 0x00,//!< Dithering disabled
        HW_LCDC_DITHER_15  = 0x03,//!< 15 bit dithering
        HW_LCDC_DITHER_16  = 0x02,//!< 16 bit dithering
        HW_LCDC_DITHER_18  = 0x01,//!< 18 bit dithering
} HW_LCDC_DITHER;

/**
 * \brief Status of operation
 */
typedef enum {
        HW_LCDC_ERR_UNSUPPORTED   = -4,//!< Operation not supported
        HW_LCDC_ERR_PARAM_INVALID = -3,//!< Invalid parameter(s)
        HW_LCDC_ERR_UNDERFLOW     = -2,//!< FIFO underflow during frame transfer
        HW_LCDC_ERR_CONF_INVALID  = -1,//!< Wrong configuration
        HW_LCDC_ERR_NONE          =  0,//!< Operation completed successfully
} HW_LCDC_ERR;

/**
 * \brief Physical connection type enumeration
 */
typedef enum {
        HW_LCDC_PHY_NONE,            //!< No physical connection - Disable LCDC
        HW_LCDC_PHY_MIPI_DBIB,       //!< DBI type B parallel connection
        HW_LCDC_PHY_QUAD_SPI,        //!< QUAD SPI connection
        HW_LCDC_PHY_DUAL_SPI,        //!< Dual SPI connection
        HW_LCDC_PHY_MIPI_SPI3,       //!< SPI connection with 3 wires (DCX as an extra bit)
        HW_LCDC_PHY_MIPI_SPI4,       //!< SPI connection with 4 wires (DCX as an extra line)
        HW_LCDC_PHY_JDI_SPI,         //!< JDI serial connection
        HW_LCDC_PHY_SHARP_SPI,       //!< Sharp serial connection
        HW_LCDC_PHY_JDI_PARALLEL,    //!< JDI parallel connection
        HW_LCDC_PHY_DPI,             //!< DPI parallel connection
} HW_LCDC_PHY;

/**
 * \def HW_LCDC_CLK_DIV_MSK
 *
 * \brief Macro to define the interface (secondary) clock divider mask
 */
#define HW_LCDC_CLK_DIV_MSK             ( HW_LCDC_REG_MSK(LCDC_CLKCTRL_REG, SEC_CLK_DIV) >> \
                                          HW_LCDC_REG_POS(LCDC_CLKCTRL_REG, SEC_CLK_DIV)      )

/**
 * \def HW_LCDC_CLK_PLL_BIT
 *
 * \brief Macro to define the bit that indicates if system PLL clock is required to achieve the
 * required frequency
 */
#define HW_LCDC_CLK_PLL_BIT             (1U << 31)

/**
 * \def HW_LCDC_CLK_RCHS_BIT
 *
 * \brief Macro to define the bit that indicates if system RCHS clock is required to achieve the
 * required frequency
 */
#define HW_LCDC_CLK_RCHS_BIT            (1U << 30)

/**
 * \def HW_LCDC_DIV
 *
 * \brief Macro to calculate the LCDC divider to produce a frequency using the provided source clock
 *
 * Due to physical restrictions only the following ranges of frequencies are valid:
 *
 *  Output (interface) clock  |  Source DIVN (32MHz)  |  Source 96MHz      |  Source 160MHz
 * -------------------------- | --------------------- | ------------------ |------------------
 *  DPI Parallel              |  32 MHz - 1 MHz       |  96 MHz - 3 MHz    |  160 MHz - 5 MHz
 *  JDI Parallel              |  16 MHz - 0.5 MHz     |  48 MHz - 1.5 MHz  |   80 MHz - 2.5 MHz
 *  Serial                    |  16 MHz - 0.5 MHz     |  48 MHz - 1.5 MHz  |   80 MHz - 2.5 MHz
 *
 *  In case of JDI parallel and serial interfaces the divider's value is automatically adapted
 *  (divided by 2) to produce the correct frequency
 *
 * \param [in] hz               Frequency in Hz
 *
 * \note If the requested frequency is not supported (i.e. there is no LCDC divider to produce the
 * exact frequency) the next available frequency will be selected.
 *
 * \warning The application or the adapter (if used) is responsible to turn on the PLL if needed and
 * maintain it as long as needed.
 */
#define HW_LCDC_DIV(hz)                                                                                    \
        ((!(dg_configDIVN_FREQ % (hz)))     ? ((dg_configDIVN_FREQ / (hz))) :                              \
         (!(dg_configRCHS_96M_FREQ % (hz))) ? (((dg_configRCHS_96M_FREQ / (hz))) | HW_LCDC_CLK_RCHS_BIT) : \
                                              (((dg_configPLL160M_FREQ / (hz))) | HW_LCDC_CLK_PLL_BIT))

/**
 * \def HW_LCDC_EXT_CLK_DIV
 *
 * \brief Macro to calculate the LCDC external clock divider in order to produce the provided
 * frequency. Divider range is [1, 2048] resulting in a range of output frequency of [0.5, 1024] Hz
 * when LP clock is at 32768KHz.
 *
 * \param [in] hz                       Frequency in tenths (0.1Hz)
 */
#define HW_LCDC_EXT_CLK_DIV(hz)         ((dg_configRC32K_FREQ * 10U / ((hz) * 32U)) - 1U)

/**
 * \brief LCD interface frequency
 *
 * It controls the interface clock divisor (\ref hw_lcdc_set_iface_clk()) and the requirement of a
 * PLL system clock by setting the flag bit LCDC_CLK_PLL_BIT.
 *
 * \note Custom values can also be entered to produce frequencies between the predefined ones. To
 * set such a frequency, the format (divisor | flag) has to be followed.
 *
 * \warning The application or the adapter (if used) is responsible to turn on the PLL if needed and
 * maintain it as long as needed.
 *
 * \sa hw_lcdc_set_iface_clk()
 */
typedef enum {
        LCDC_FREQ_80MHz   = HW_LCDC_DIV(80000000U),//!< LCD interface frequency at 80MHz
        LCDC_FREQ_48MHz   = HW_LCDC_DIV(48000000U),//!< LCD interface frequency at 48MHz
        LCDC_FREQ_40MHz   = HW_LCDC_DIV(40000000U),//!< LCD interface frequency at 40MHz
        LCDC_FREQ_26_7MHz = HW_LCDC_DIV(26700000U),//!< LCD interface frequency at 26.7MHz
        LCDC_FREQ_24MHz   = HW_LCDC_DIV(24000000U),//!< LCD interface frequency at 24MHz
        LCDC_FREQ_20MHz   = HW_LCDC_DIV(20000000U),//!< LCD interface frequency at 20MHz
        LCDC_FREQ_16MHz   = HW_LCDC_DIV(16000000U),//!< LCD interface frequency at 16MHz
        LCDC_FREQ_12MHz   = HW_LCDC_DIV(12000000U),//!< LCD interface frequency at 12MHz
        LCDC_FREQ_9_6MHz  = HW_LCDC_DIV( 9600000U),//!< LCD interface frequency at 9.6MHz
        LCDC_FREQ_8MHz    = HW_LCDC_DIV( 8000000U),//!< LCD interface frequency at 8MHz
        LCDC_FREQ_6MHz    = HW_LCDC_DIV( 6000000U),//!< LCD interface frequency at 6MHz
        LCDC_FREQ_4_8MHz  = HW_LCDC_DIV( 4800000U),//!< LCD interface frequency at 4.8MHz
        LCDC_FREQ_4MHz    = HW_LCDC_DIV( 4000000U),//!< LCD interface frequency at 4MHz
        LCDC_FREQ_3_2MHz  = HW_LCDC_DIV( 3200000U),//!< LCD interface frequency at 3.2MHz
        LCDC_FREQ_3MHz    = HW_LCDC_DIV( 3000000U),//!< LCD interface frequency at 3MHz
        LCDC_FREQ_2MHz    = HW_LCDC_DIV( 2000000U),//!< LCD interface frequency at 2MHz
        LCDC_FREQ_1_6MHz  = HW_LCDC_DIV( 1600000U),//!< LCD interface frequency at 1.6MHz
        LCDC_FREQ_1MHz    = HW_LCDC_DIV( 1000000U),//!< LCD interface frequency at 1MHz
        LCDC_FREQ_0_8MHz  = HW_LCDC_DIV(  800000U),//!< LCD interface frequency at 800Hz
        LCDC_FREQ_0_5MHz  = HW_LCDC_DIV(  500000U),//!< LCD interface frequency at 500Hz
} HW_LCDC_FREQ;


/**
 * \brief Parallel connection type
 */
typedef enum {
        HW_LCDC_GPIO_IF_JDI  = 0,//!< JDI parallel connection
        HW_LCDC_GPIO_IF_DPI  = 1,//!< DPI parallel connection
        HW_LCDC_GPIO_IF_DBIB = 2,//!< DBI-B parallel connection
        HW_LCDC_GPIO_IF_SPI  = 3,//!< Serial connection
        HW_LCDC_GPIO_IF_GPI  = 4,//!< GPI connection
} HW_LCDC_GPIO_IF;

/**
 * \brief Tearing effect detection method
 */
typedef enum {
        HW_LCDC_TE_POL_LOW  = 0,//!< Detected low TE signal
        HW_LCDC_TE_POL_HIGH = 1,//!< Detected high TE signal
} HW_LCDC_TE;

/**
 * \brief Layer FIFO input threshold
 */
typedef enum {
        HW_LCDC_FIFO_THR_HALF         = 0x00,//!< DMA is triggered when FIFO is below half (Default)
        HW_LCDC_FIFO_THR_2_BURST_SIZE = 0x01,//!< DMA is triggered when FIFO can fit at least 2 bursts
        HW_LCDC_FIFO_THR_4_BURST_SIZE = 0x02,//!< DMA is triggered when FIFO can fit at least 4 bursts
        HW_LCDC_FIFO_THR_8_BURST_SIZE = 0x03,//!< DMA is triggered when FIFO can fit at least 8 bursts
} HW_LCDC_FIFO_THR;

/**
 * \brief Layer burst length in beats
 */
typedef enum {
        HW_LCDC_BURST_LEN_8_BEATS  = 0x1,//!< 8 beats burst length
        HW_LCDC_BURST_LEN_16_BEATS = 0x0,//!< 16 beats burst length
} HW_LCDC_BURST_LEN;

/**
 * Layer number
 */
typedef enum {
        HW_LCDC_LAYER_0,  //!< Layer 0 - Background layer
        HW_LCDC_LAYER_1,  //!< Layer 1 - Foreground layer
        HW_LCDC_LAYER_MAX,//!< Count of available layers
} HW_LCDC_LAYER;

/**
 * \brief Blend factors configuration (\ref HW_LCDC_BLEND_MODE)
 *
 * Resulting color follows the equation C = Cs * Fs + Cd * Fd, where Fs and Fd are configured by the
 * source (s) and destination (d) blend factors
 */
typedef enum {
        HW_LCDC_BF_ZERO            = 0x0,//!< Blend black (Fs/d = 0)
        HW_LCDC_BF_ONE             = 0x1,//!< Blend white (Fs/d = 1)
        HW_LCDC_BF_SRCALPHA        = 0x2,//!< Blend alpha source (Fs/d = as)
        HW_LCDC_BF_GLBALPHA        = 0x3,//!< Blend alpha global (Fs/d = ag)
        HW_LCDC_BF_SRCGBLALPHA     = 0x4,//!< Blend alpha source and alpha global (Fs/d = as * ag)
        HW_LCDC_BF_INVSRCALPHA     = 0x5,//!< Blend inverted source (Fs/d = 1-as)
        HW_LCDC_BF_INVGBLALPHA     = 0x6,//!< Blend inverted global (Fs/d = 1-ag)
        HW_LCDC_BF_INVSRCGBLALPHA  = 0x7,//!< Blend inverted source and inverted global (Fs/d = 1 - as * ag)
        HW_LCDC_BF_DSTALPHA        = 0xA,//!< Blend alpha destination (Fs/d = ad)
        HW_LCDC_BF_INVDSTALPHA     = 0xB,//!< Blend inverted destination (Fs/d = 1 - ad)
} HW_LCDC_BLEND_FACTORS;

/**
 * \def HW_LCDC_BLENDMODE
 *
 * \brief Macro to create various blend modes by combining a source and a destination blend factor
 */
#define HW_LCDC_BLENDMODE(src, dst)     ((HW_LCDC_BF_ ## src) | ((HW_LCDC_BF_ ## dst) << 4))

/**
 * \brief Blend modes configure how each layer is blended with the previous one(s)
 */
typedef enum {
        HW_LCDC_BL_SIMPLE     = HW_LCDC_BLENDMODE(SRCALPHA    ,INVSRCALPHA),   //!< Sa * Sa + Da * (1 - Sa)
        HW_LCDC_BL_CLEAR      = HW_LCDC_BLENDMODE(ZERO        ,ZERO       ),   //!< 0
        HW_LCDC_BL_SRC        = HW_LCDC_BLENDMODE(ONE         ,ZERO       ),   //!< Sa
        HW_LCDC_BL_SRC_OVER   = HW_LCDC_BLENDMODE(ONE         ,INVSRCALPHA),   //!< Sa + Da * (1 - Sa)
        HW_LCDC_BL_DST_OVER   = HW_LCDC_BLENDMODE(INVDSTALPHA ,ONE        ),   //!< Sa * (1 - Da) + Da
        HW_LCDC_BL_SRC_IN     = HW_LCDC_BLENDMODE(DSTALPHA    ,ZERO       ),   //!< Sa * Da
        HW_LCDC_BL_DST_IN     = HW_LCDC_BLENDMODE(ZERO        ,SRCALPHA   ),   //!< Da * Sa
        HW_LCDC_BL_SRC_OUT    = HW_LCDC_BLENDMODE(INVDSTALPHA ,ZERO       ),   //!< Sa * (1 - Da)
        HW_LCDC_BL_DST_OUT    = HW_LCDC_BLENDMODE(ZERO        ,INVSRCALPHA),   //!< Da * (1 - Sa)
        HW_LCDC_BL_SRC_ATOP   = HW_LCDC_BLENDMODE(DSTALPHA    ,INVSRCALPHA),   //!< Sa * Da + Da * (1 - Sa)
        HW_LCDC_BL_DST_ATOP   = HW_LCDC_BLENDMODE(INVDSTALPHA ,SRCALPHA   ),   //!< Sa * (1 - Da) + Da * Sa
        HW_LCDC_BL_ADD        = HW_LCDC_BLENDMODE(ONE         ,ONE        ),   //!< Sa + Da
        HW_LCDC_BL_XOR        = HW_LCDC_BLENDMODE(INVDSTALPHA ,INVSRCALPHA),   //!< Sa * (1 - Da) + Da * (1 - Sa)
} HW_LCDC_BLEND_MODE;

/**
 * \brief DMA pre-fetch level
 *
 * LCD controller waits until at least the specified amount of data have been received in FIFO
 * before the transmission of the frame starts.
 */
typedef enum {
        HW_LCDC_FIFO_PREFETCH_LVL_DISABLED  = 0x00,//!< No wait, controller starts immediately sending data
        HW_LCDC_FIFO_PREFETCH_LVL_1         = 0x01,//!< Wait until at least 52 bytes have been received
        HW_LCDC_FIFO_PREFETCH_LVL_2         = 0x02,//!< Wait until at least 84 bytes have been received
        HW_LCDC_FIFO_PREFETCH_LVL_3         = 0x03,//!< Wait until at least 116 bytes have been received
        HW_LCDC_FIFO_PREFETCH_LVL_4         = 0x04,//!< Wait until at least 108 bytes have been received
} HW_LCDC_FIFO_PREFETCH_LVL;

/**
 * \brief Chip select mode of operation
 *
 * In auto modes, chip select is handled automatically by the LCD controller
 */
typedef enum {
        HW_LCDC_SCS_AUTO,    //!< Chip select is low when enabled
        HW_LCDC_SCS_AUTO_INV,//!< Chip select is high when enabled
        HW_LCDC_SCS_HIGH,    //!< Chip select is forced to high
        HW_LCDC_SCS_LOW,     //!< Chip select is forced to low
} HW_LCDC_SCS_CFG;

/**
 * \brief JDI/Sharp serial commands enumeration
 *
 * \note Each LCD may adopt only a part of the functionality and the respective commands. Please,
 * reference to the specific LCD documentation for the supported commands.
 */
typedef enum {
        HW_LCDC_JDIS_CMD_NOP           = 0x00,//!< No operation
        HW_LCDC_JDIS_CMD_BLINKOFF      = 0x00,//!< Stop LCD blinking
        HW_LCDC_JDIS_CMD_BLINKBLACK    = 0x10,//!< Blink display with black color
        HW_LCDC_JDIS_CMD_BLINKWHITE    = 0x18,//!< Blink display with white color
        HW_LCDC_JDIS_CMD_BLINKINVERT   = 0x14,//!< Blink display with inverted colors
        HW_LCDC_JDIS_CMD_CLEAR         = 0x20,//!< Clear display memory
        HW_LCDC_JDIS_CMD_UPDATE_NATIVE = 0x80,//!< Update display in native color mode
        HW_LCDC_JDIS_CMD_UPDATE_1BIT   = 0x88,//!< Update display in 1 bit color mode (b&w)
        HW_LCDC_JDIS_CMD_UPDATE_4BIT   = 0x90,//!< Update display in 4 bit color mode
} HW_LCDC_JDIS_CMD;

/**
 * \brief LCD external clock frequency
 */
typedef enum {
        HW_LCDC_EXT_CLK_OFF    = 0,                         //!< Clock is off (Default)
        HW_LCDC_EXT_CLK_1HZ    = HW_LCDC_EXT_CLK_DIV(10),   //!< Clock frequency at 1Hz
        HW_LCDC_EXT_CLK_62_5HZ = HW_LCDC_EXT_CLK_DIV(625),  //!< Clock frequency at 62.5Hz
        HW_LCDC_EXT_CLK_125HZ  = HW_LCDC_EXT_CLK_DIV(1250), //!< Clock frequency at 125Hz
} HW_LCDC_EXT_CLK;

/**
 * \brief Structure that holds a frame's dimensions
 */
typedef struct {
        uint16_t startx;//!< Start column of the frame
        uint16_t starty;//!< Start row of the frame
        uint16_t endx;  //!< End column of the frame
        uint16_t endy;  //!< End row of the frame
} hw_lcdc_frame_t;

/**
 * \brief Structure that holds the display timing parameters in pixels
 */
typedef struct {
        uint16_t resx;//!< Horizontal resolution of the screen
        uint16_t resy;//!< Vertical resolution of the screen
        uint16_t fpx; //!< Horizontal front porch
        uint16_t fpy; //!< Vertical front porch
        uint16_t bpx; //!< Horizontal back porch
        uint16_t bpy; //!< Vertical back porch
        uint16_t blx; //!< Horizontal blanking
        uint16_t bly; //!< Vertical blanking
} hw_lcdc_display_t;

/**
 * \brief Structure that holds the layer parameters (input of the LCD controller)
 */
typedef struct {
        uint32_t baseaddr;                         //!< Base address where the input frame resides in memory
        int32_t  stride;                           //!< Line to line distance in bytes of frame in memory
        int16_t  startx;                           /*!< Horizontal coordinate of the top-left corner of the
                                                        layer. (0,0) is the top-left corner of the screen */
        int16_t  starty;                           /*!< Vertical coordinate of the top-left corner of the
                                                        layer. (0,0) is the top-left corner of the screen */
        uint16_t resx;                             //!< Horizontal resolution of layer in pixels
        uint16_t resy;                             //!< Vertical resolution of layer in pixels
        HW_LCDC_LAYER_COLOR_MODE format;           //!< Color mode format of the layer, \ref HW_LCDC_LAYER_COLOR_MODE
        HW_LCDC_BLEND_MODE blendmode;              //!< Blend mode of the layer with its underlying image, /ref HW_LCDC_BLEND_MODE
        HW_LCDC_FIFO_PREFETCH_LVL dma_prefetch_lvl;//!< DMA pre-fetch level as defined in \ref HW_LCDC_FIFO_PREFETCH_LVL
        uint8_t alpha;                             //!< Global alpha value (combined with \ref blendmode)
} hw_lcdc_layer_t;

/**
 * \brief MIPI DBIC (SPI) specific configuration parameters
 */
typedef struct {
        const uint8_t *write_memory_cmd;//!< Can be used to set a custom command for the transfer of pixels
        uint8_t write_memory_cmd_len;   //!< Length of custom write memory command. If set to 0, default command is sent
        bool si_on_so;                  //!< Set to true if SI is in the same pin with SO
} hw_lcdc_spi_t;

/**
 * \brief MIPI DBIB specific configuration parameters
 */
typedef struct {
        const uint8_t *write_memory_cmd;//!< Can be used to set a custom command for the transfer of pixels
        uint8_t write_memory_cmd_len;   //!< Length of custom write memory command. If set to 0, default command is sent
} hw_lcdc_dbib_t;

/**
 * \brief Dual SPI specific configuration parameters
 */
typedef struct {
        const uint8_t *write_memory_cmd;//!< Can be used to set a custom command for the transfer of pixels
        uint8_t write_memory_cmd_len;   //!< Length of custom write memory command. If set to 0, default command is sent
        HW_LCDC_DSPI_OPT option;        //!< Dual SPI flavor, \ref HW_LCDC_DSPI_OPT
        bool spi3;                      //!< Select if command is send in SPI3 or SPI4 mode
        bool si_on_so;                  //!< Set to true if SI is in the same pin with SO
} hw_lcdc_dspi_t;

/**
 * \brief Quad SPI specific configuration parameters
 */
typedef struct {
        const uint8_t *write_memory_cmd; //!< Can be used to set a custom command for the transfer of pixels
        uint8_t write_memory_cmd_len;    //!< Length of custom write memory command. If set to 0, default command is sent
        HW_LCDC_CMD_WIDTH cmd_width;     //!< Command width (transmitted in the address field), \ref HW_LCDC_CMD_WIDTH
        uint8_t sss_write_cmd;           //!< Byte value that indicates a write command in single mode
        uint8_t sss_read_cmd;            //!< Byte value that indicates a read command in single mode
        uint8_t ssq_write_cmd;           //!< Byte value that indicates a write in quad mode
        bool si_on_so;                   //!< Set to true if SI is in the same pin with SO
} hw_lcdc_qspi_t;


/**
 * \brief Structure that holds the JDI parallel timings information
 *
 * Horizontal values are counted in HCK quarters and vertical values in VCK halves
 */
typedef struct {
        uint16_t fpx; //!< Horizontal front porch
        uint16_t fpy; //!< Vertical front porch
        uint16_t bpx; //!< Horizontal back porch
        uint16_t bpy; //!< Vertical back porch
        uint16_t blx; //!< Horizontal blanking
        uint16_t bly; //!< Vertical blanking
} hw_lcdc_jdi_par_t;

/**
 * \brief Structure that holds the display timing parameters in pixels
 */
typedef struct {
        uint16_t fpx; //!< Horizontal front porch
        uint16_t fpy; //!< Vertical front porch
        uint16_t bpx; //!< Horizontal back porch
        uint16_t bpy; //!< Vertical back porch
        uint16_t blx; //!< Horizontal blanking
        uint16_t bly; //!< Vertical blanking
        bool enable_dpi_ready;
} hw_lcdc_dpi_t;

/**
 * \brief LCD Controller configuration
 */
typedef struct {
        HW_LCDC_PHY phy_type;                     //!< Physical connection type as defined in \ref HW_LCDC_PHY
        HW_LCDC_OUTPUT_COLOR_MODE format;         //!< Output color mode/format of the LCD controller as in \ref HW_LCDC_OUTPUT_COLOR_MODE
        uint16_t resx;                            //!< Horizontal resolution of LCD
        uint16_t resy;                            //!< Vertical resolution of LCD
        union {
                hw_lcdc_spi_t spi;                //!< DBI-C (SPI3/4) interface specific configuration
                hw_lcdc_dbib_t dbib;              //!< DBI-B interface specific configuration
                hw_lcdc_dspi_t dspi;              //!< Dual SPI interface specific configuration
                hw_lcdc_qspi_t qspi;              //!< Quad SPI interface specific configuration
                hw_lcdc_jdi_par_t jdi_par;        //!< JDI parallel interface specific configuration
                hw_lcdc_dpi_t dpi;                //!< DPI interface specific configuration
        } iface_conf;                             //!< Interface specific configuration
        HW_LCDC_MIPI_CFG cfg_extra_flags;         //!< Extra configuration flags to be applied in register LCDC_DBIB_CFG_REG, see also \ref hw_lcdc_set_mipi_cfg()
        HW_LCDC_MODE mode;                        //!< Mode configuration flags (\ref HW_LCDC_MODE)
        union {
                struct {
                        HW_LCDC_FREQ write_freq;  //!< Frequency of the interface as provided by \ref HW_LCDC_FREQ or \ref HW_LCDC_DIV
                        HW_LCDC_FREQ read_freq;   //!< Frequency of the interface as provided by \ref HW_LCDC_FREQ or \ref HW_LCDC_DIV
                };
        };
        HW_LCDC_DITHER dither;                    //!< Dithering configuration, \ref HW_LCDC_DITHER
} hw_lcdc_config_t;

/**
 * \brief Callback function to be called when an interrupt event occurs
 *
 * \param [in] status           Indication if an error has occurred in the last transfer
 * \param [in] user_data        User defined data to be passed
 *
 * \sa hw_lcdc_set_callback()
 */
typedef void (*hw_lcdc_callback)(HW_LCDC_ERR status, void *user_data);


/*
 * API FUNCTION DECLARATIONS
 *****************************************************************************************
 */
/**
 * \name                Register functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Returns the HW configuration (features) of LCDC
 *
 * \return HW features
 */
__STATIC_INLINE uint32_t hw_lcdc_get_hw_config(void)
{
        return LCDC->LCDC_CONF_REG;
}

/**
 * \brief Returns HW supported color modes
 *
 * \return HW color modes
 */
__STATIC_INLINE uint32_t hw_lcdc_get_color_modes(void)
{
        return LCDC->LCDC_COLMOD_REG;
}

/**
 * \brief Set the mode register with corresponding value(s)
 *
 * \param [in] mode             Flag(s) to be set (\ref HW_LCDC_MODE)
 */
__STATIC_INLINE void hw_lcdc_set_mode(HW_LCDC_MODE mode)
{
        LCDC->LCDC_MODE_REG = mode;
}

/**
 * \brief Set clock divider of the LCDC which controls the internal pixel pipeline clock
 *
 * \note Source clock of this divider is the format pipeline clock. The period of the generated
 * clock is defined as : LCDC_CLK_DIV x period_of_format_clk. A zero value gives division by one.
 *
 * \note Preferably set this divider to 1 (maximum frequency) which is also the default value
 *
 * \param [in] div              Clock divider
 */
__STATIC_INLINE void hw_lcdc_set_pixel_clk(uint8_t div)
{
        HW_LCDC_REG_SETF(LCDC_CLKCTRL_REG, CLK_DIV, div);
}

/**
 * \brief Set (secondary) clock divider of LCDC which controls the interface/format clock
 *
 * \note Source clock of this divider is the main clock of LCD controller. The period of the
 * generated clock is defined as : (LCDC_SEC_CLK_DIV + 1) x period_of_main_clock.
 *
 * \note Output clock of the serial interfaces is further divided by 2
 *
 * \param [in] div              Clock divider
 */
__STATIC_INLINE void hw_lcdc_set_iface_clk(uint8_t div)
{
        HW_LCDC_REG_SETF(LCDC_CLKCTRL_REG, SEC_CLK_DIV, div);
}

/**
 * \brief Configure the number of bits for the read operation
 *
 * \param [in] bits             Number of bits to be read
 */
__STATIC_INLINE void hw_lcdc_set_read_cycles(uint8_t bits)
{
        switch (bits) {
        case 8:
                HW_LCDC_REG_SETF(LCDC_DBIB_RDAT_REG, DBIB_READ_C, 0x00);
                break;
        case 16:
                HW_LCDC_REG_SETF(LCDC_DBIB_RDAT_REG, DBIB_READ_C, 0x01);
                break;
        case 24:
                HW_LCDC_REG_SETF(LCDC_DBIB_RDAT_REG, DBIB_READ_C, 0x02);
                break;
        default:
                bits = MIN(bits - 1, 0x3F);
                HW_LCDC_REG_SETF(LCDC_FMTCTRL_REG, DBIB_READ_C, bits);
                HW_LCDC_REG_SETF(LCDC_DBIB_RDAT_REG, DBIB_READ_C, 0x03);
                break;
        }
}

/**
 * \brief Receive data already read by \ref hw_lcdc_set_read_cycles()
 *
 * \sa hw_lcdc_set_read_cycles()
 *
 * \return Data read from the LCD
 */
__STATIC_INLINE uint32_t hw_lcdc_get_read_data(void)
{
        return HW_LCDC_REG_GETF(LCDC_DBIB_RDAT_REG, DBIB_RDAT);
}

/**
 * \brief Configures the frequency of the external clock produced for the LCD internal refresh.
 *
 * \param [in] div              Clock divider as defined in \ref HW_LCDC_EXT_CLK
 */
__STATIC_INLINE void hw_lcdc_set_external_clk(HW_LCDC_EXT_CLK div)
{
        if (div == HW_LCDC_EXT_CLK_OFF) {
                REG_CLR_BIT(CRG_TOP, LCD_EXT_CTRL_REG, LCD_EXT_CLK_EN);
        } else {
                div = MIN(div, REG_MSK(CRG_TOP, LCD_EXT_CTRL_REG, LCD_EXT_CNT_RELOAD));
                /* T = (slp_clk) * 32 * (LCD_EXT_CNT_RELOAD + 1) */
                uint32_t lcd_ext_ctrl_reg = CRG_TOP->LCD_EXT_CTRL_REG;
                REG_SET_FIELD(CRG_TOP, LCD_EXT_CTRL_REG, LCD_EXT_CLK_EN, lcd_ext_ctrl_reg, 1);
                REG_SET_FIELD(CRG_TOP, LCD_EXT_CTRL_REG, LCD_EXT_CNT_RELOAD, lcd_ext_ctrl_reg, div);
                CRG_TOP->LCD_EXT_CTRL_REG = lcd_ext_ctrl_reg;
        }
}

/**
 * \brief Gets the configured frequency of the external clock produced for the LCD internal refresh
 *
 * \return Frequency configured, \ref HW_LCDC_EXT_CLK
 */
__STATIC_INLINE HW_LCDC_EXT_CLK hw_lcdc_get_external_clk(void)
{
        if (!REG_GETF(CRG_TOP, LCD_EXT_CTRL_REG, LCD_EXT_CLK_EN)) {
                return HW_LCDC_EXT_CLK_OFF;
        } else {
                return REG_GETF(CRG_TOP, LCD_EXT_CTRL_REG, LCD_EXT_CNT_RELOAD);
        }
}

/**
 * \brief Set layer FIFO parameters
 *
 * FIFO threshold controls at which threshold a DMA request is triggered and FIFO burst length
 * controls the amount of data DMA will try to fetch in a single burst transaction.
 *
 * \param [in] layer            Layer index
 * \param [in] fifo_thr         FIFO input threshold (\ref HW_LCDC_FIFO_THR)
 * \param [in] burst_len        Burst length (\ref HW_LCDC_BURST_LEN)
 */
__STATIC_INLINE void hw_lcdc_set_layer_fifo_params(HW_LCDC_LAYER layer, HW_LCDC_FIFO_THR fifo_thr, HW_LCDC_BURST_LEN burst_len)
{
        ASSERT_ERROR(layer < HW_LCDC_LAYER_MAX);
        uint32_t lcdc_layer_stride_reg = HW_LCDC_GET_LAYER_REG(LCDC_LAYER0_STRIDE_REG, layer);
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_STRIDE_REG, L0_FIFO_THR, lcdc_layer_stride_reg, fifo_thr);
        HW_LCDC_REG_SET_FIELD(LCDC_LAYER0_STRIDE_REG, L0_NO_16BEAT_BURST, lcdc_layer_stride_reg, burst_len);
        HW_LCDC_SET_LAYER_REG(LCDC_LAYER0_STRIDE_REG, layer, lcdc_layer_stride_reg);
}

/**
 * \brief Get layer FIFO maximum burst length
 *
 * \param [in] layer            Layer index
 *
 * \return burst length
 */
__STATIC_INLINE HW_LCDC_BURST_LEN hw_lcdc_get_layer_burst_len(HW_LCDC_LAYER layer)
{
        ASSERT_ERROR(layer < HW_LCDC_LAYER_MAX);
        return HW_LCDC_GETF_LAYER_REG(LCDC_LAYER0_STRIDE_REG, L0_NO_16BEAT_BURST, layer);
}

/**
 * \brief Control if the palette (gamma correction) will be used from the LCDC output.
 *
 * \sa hw_lcdc_set_palette
 *
 * \param [in] enable           Enable or disable palette
 */
__STATIC_INLINE void hw_lcdc_set_palette_state(bool enable)
{
        HW_LCDC_MODE mode = LCDC->LCDC_MODE_REG & ~HW_LCDC_MODE_PALETTE;

        hw_lcdc_set_mode(mode | (enable ? HW_LCDC_MODE_PALETTE : 0));
}

/**
 * \brief Set the contents of the 256 RGB888 entries of the palette LUT.
 *
 * \param [in] index            Index of the (first) palette entry to set
 * \param [in] color            Array of RGB values for the LUT, \ref HW_LCDC_PALETTE_ENTRY
 * \param [in] color_num        Number of entries to set
 */
__STATIC_INLINE void hw_lcdc_set_palette(int index, const uint32_t *color, int color_num)
{
        for (int i = 0; i < color_num && index < HW_LCDC_PALETTE_ENTRIES; i++, index++) {
                volatile uint32_t *lcdc_palette_reg = REG_GET_ADDR_INDEXED(LCDC, LCDC_PALETTE_BASE, sizeof(uint32_t), index);
                *lcdc_palette_reg = color[i];
        }
}


/**
 * \brief Forces the output of the LCDC to be blank
 *
 * \param [in] enable           Enable or disable
 */
__STATIC_INLINE void hw_lcdc_force_blank(bool enable)
{
        HW_LCDC_MODE mode = LCDC->LCDC_MODE_REG & ~HW_LCDC_MODE_BLANK;

        hw_lcdc_set_mode(mode | (enable ? HW_LCDC_MODE_BLANK : 0));
}

/**
 * \brief Set display background color
 *
 * \param [in] red              Red color used as background
 * \param [in] green            Green color used as background
 * \param [in] blue             Blue color used as background
 * \param [in] alpha            Alpha used as background
 */
__STATIC_INLINE void hw_lcdc_set_bg_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
        uint32_t lcdc_bgcolor_reg = 0;
        HW_LCDC_REG_SET_FIELD(LCDC_BGCOLOR_REG, BG_RED, lcdc_bgcolor_reg, red);
        HW_LCDC_REG_SET_FIELD(LCDC_BGCOLOR_REG, BG_GREEN, lcdc_bgcolor_reg, green);
        HW_LCDC_REG_SET_FIELD(LCDC_BGCOLOR_REG, BG_BLUE, lcdc_bgcolor_reg, blue);
        HW_LCDC_REG_SET_FIELD(LCDC_BGCOLOR_REG, BG_ALPHA, lcdc_bgcolor_reg, alpha);
        LCDC->LCDC_BGCOLOR_REG = lcdc_bgcolor_reg;
}

/**
 * \brief Set interface type
 *
 * \param [in] iface            Selected interface type (\ref HW_LCDC_GPIO_IF)
 */
__STATIC_INLINE void hw_lcdc_set_iface(HW_LCDC_GPIO_IF iface)
{
        uint32_t lcdc_gpio_reg = LCDC->LCDC_GPIO_REG;
        HW_LCDC_REG_SET_FIELD(LCDC_GPIO_REG, GPIO_OUTPUT_MODE, lcdc_gpio_reg, iface);
        HW_LCDC_REG_SET_FIELD(LCDC_GPIO_REG, GPIO_OUTPUT_EN, lcdc_gpio_reg,
                iface == HW_LCDC_GPIO_IF_GPI ? 0 : 1);
        LCDC->LCDC_GPIO_REG = lcdc_gpio_reg;
}

/**
 * \brief Get interface type
 *
 * \return Selected interface type (\ref HW_LCDC_GPIO_IF)
 */
__STATIC_INLINE HW_LCDC_GPIO_IF hw_lcdc_get_iface(void)
{
        uint32_t lcdc_gpio_reg = LCDC->LCDC_GPIO_REG;
        if (HW_LCDC_REG_GET_FIELD(LCDC_GPIO_REG, GPIO_OUTPUT_EN, lcdc_gpio_reg)) {
                return HW_LCDC_REG_GET_FIELD(LCDC_GPIO_REG, GPIO_OUTPUT_MODE, lcdc_gpio_reg);
        } else {
                return HW_LCDC_GPIO_IF_GPI;
        }
}
/**
 * \brief Returns the sticky underflow status bit and clears it before exiting
 *
 * An underflow can occur if an LCDC DMA transaction has been initiated and required layer data is
 * not available in the required rate. Possible causes may be that bus/memory are either slow or
 * occupied by another master. Condition can be affected by the DMA level of the layer
 *
 * \sa hw_lcdc_set_layer_offset_dma_prefetch
 *
 * \note Any write access to register LCDC_INTERRUPT_REG will clear status. As a result function
 * \ref hw_lcdc_get_sticky_underflow_status() must be called before.
 *
 * \return If an underflow has occurred
 * \retval true  Underflow has occurred
 * \retval false Underflow has not occurred
 */
__STATIC_INLINE bool hw_lcdc_get_sticky_underflow_status(void)
{
        bool underflow;
        underflow = HW_LCDC_REG_GETF(LCDC_STATUS_REG, STICKY_UNDERFLOW) ? true : false;
        if (underflow) {
                /* Clean sticky bit by writing the interrupt register */
                uint32_t lcdc_interrupt_reg = LCDC->LCDC_INTERRUPT_REG;
                LCDC->LCDC_INTERRUPT_REG = lcdc_interrupt_reg;
        }
        return underflow;
}

/**
 * \brief Detect if LCD controller is active / inactive
 *
 * \return If LCDC is busy
 * \retval true  LCD controller is active
 * \retval false LCD controller is idle
 */
__STATIC_INLINE bool hw_lcdc_is_busy(void)
{
        uint32_t lcdc_status_reg = LCDC->LCDC_STATUS_REG;
        if (lcdc_status_reg & ( HW_LCDC_REG_MSK(LCDC_STATUS_REG, DBI_SPI_CS)
                              | HW_LCDC_REG_MSK(LCDC_STATUS_REG, DBIB_OUT_TRANS_PENDING)
                              | HW_LCDC_REG_MSK(LCDC_STATUS_REG, DBIB_CMD_PENDING)
                              | HW_LCDC_REG_MSK(LCDC_STATUS_REG, DBIB_DATA_PENDING)
                              | HW_LCDC_REG_MSK(LCDC_STATUS_REG, STAT_DE)
                              | HW_LCDC_REG_MSK(LCDC_STATUS_REG, STAT_ACTIVE))) {
                return true;
        }
        return false;
}

/**
 * \brief Get LCD Controller ID
 *
 * \return ID of the LCD controller
 */
__STATIC_INLINE uint32_t hw_lcdc_get_id(void)
{
        return HW_LCDC_REG_GETF(LCDC_IDREG_REG, LCDC_ID);
}

/** \} */

/**
 * \name                Display controller functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Return the pixel size in bits of an input (layer) color format
 *
 * \param [in] format           Input color mode
 *
 * \return Pixel bit size
 */
__STATIC_INLINE uint8_t hw_lcdc_lcm_size_bits(HW_LCDC_LAYER_COLOR_MODE format)
{
        switch (format) {
        case HW_LCDC_LCM_RGBA8888:
        case HW_LCDC_LCM_ARGB8888:
        case HW_LCDC_LCM_ABGR8888:
        case HW_LCDC_LCM_BGRA8888:
                return 32;
        case HW_LCDC_LCM_RGB888:
                return 24;
        case HW_LCDC_LCM_RGBA4444:
        case HW_LCDC_LCM_ARGB4444:
        case HW_LCDC_LCM_RGBA5551:
        case HW_LCDC_LCM_RGB565:
                return 16;
        case HW_LCDC_LCM_RGB332:
                return 8;
        default:
                ASSERT_WARNING(0);
                return 0;
        }
}

/**
 * \brief Return the pixel size in bits of an output color mode
 *
 * \param [in] format           Output color mode
 *
 * \return Pixel bit size
 */
__STATIC_INLINE uint8_t hw_lcdc_ocm_size_bits(HW_LCDC_OUTPUT_COLOR_MODE format)
{
        switch (format) {
        case HW_LCDC_OCM_L1:
                return 1;
        case HW_LCDC_OCM_RGB111:
                return 3;
        case HW_LCDC_OCM_8RGB111_1:
        case HW_LCDC_OCM_8RGB111_2:
        case HW_LCDC_OCM_8RGB111_3:
                return 4;
        case HW_LCDC_OCM_RGB222:
                return 6;
        case HW_LCDC_OCM_8RGB332:
                return 8;
        case HW_LCDC_OCM_8RGB444:
                return 12;
        case HW_LCDC_OCM_8RGB565:
                return 16;
        case HW_LCDC_OCM_8RGB666_P:
                return 18;
        case HW_LCDC_OCM_8RGB666:
        case HW_LCDC_OCM_8RGB888:
                return 24;
        default:
                ASSERT_WARNING(0);
                return 0;
        }
}

/**
 * \brief Return the pixel size in bytes of an input (layer) color format
 *
 * \param [in] format           Input color mode
 *
 * \return Pixel byte size
 */
__STATIC_INLINE uint8_t hw_lcdc_lcm_size(HW_LCDC_LAYER_COLOR_MODE format)
{
        return (hw_lcdc_lcm_size_bits(format) + 7) >> 3;
}

/**
 * \brief Calculate the minimum stride size required for the provided parameters
 *
 * \param [in] format           Format (color) mode
 * \param [in] width            Width of the display
 *
 * \return Stride size in bytes of the mode
 */
uint32_t hw_lcdc_stride_size(HW_LCDC_LAYER_COLOR_MODE format, uint16_t width);

/**
 * \brief Initializes the LCD Controller module and driver
 *
 * \param [in] cfg              Configuration parameters
 *
 * \return Operation status
 */
HW_LCDC_ERR hw_lcdc_init(const hw_lcdc_config_t *cfg);

/**
 * \brief Sets the frame generator's timing properties such as resolution, blanking and porches
 *
 * \param [in] params           Timing parameters
 */
void hw_lcdc_set_lcd_timing(const hw_lcdc_display_t *params);

/**
 * \brief Set the update region of the screen (screen must support partial update)
 *
 * If provided parameters are not valid, they are modified accordingly.
 *
 * \param [in,out] frame        Frame parameters
 */
void hw_lcdc_set_update_region(hw_lcdc_frame_t *frame);

/**
 * \brief Checks if provided layer settings can be supported by the configured prefetch level
 *
 * \param [in] layer            Layer attributes structure
 * \param [in] burst_len        Configured maximum burst length
 *
 * \return true if provided settings are supported, false otherwise
 */
bool hw_lcdc_layer_is_valid(const hw_lcdc_layer_t *layer, HW_LCDC_BURST_LEN burst_len);

/**
 * \brief Set layer parameters.
 *
 * Enable the layer and set attributes to it. LCD controller blends each layer with the underlying
 * image which can be the background color (defined by the background color \ref hw_lcdc_set_bg_color())
 * or the underlying layer. The layer can be disabled (its background will be displayed) or can be
 * placed anywhere in the horizontal plane of the LCD. It can be placed even outside (partially or
 * not) of the visual boundaries of the LCD. Layer is capable of displaying any sized image that can
 * be described with the \ref hw_lcdc_layer_t structure.
 *
 * \param [in] layer_no         Layer index
 * \param [in] enable           Enable / disable display of the layer
 * \param [in] layer            Layer attributes structure
 *
 * \return If layer is enabled and has a part inside the LCD frame.
 * \retval true  Layer is enabled
 * \retval false Layer is not enabled or not visible
 */
bool hw_lcdc_set_layer(HW_LCDC_LAYER layer_no, bool enable, const hw_lcdc_layer_t *layer);

/**
 * \brief Set chip select pin configuration
 *
 * \note In most use cases, chip select does not need to be configured, it is automatically done by
 * function \ref hw_lcdc_init()
 *
 * \param [in] state            Chip select configuration as defined in \ref HW_LCDC_SCS_CFG
 */
void hw_lcdc_set_scs(HW_LCDC_SCS_CFG state);

/**
 * \brief Get chip select pin configuration
 *
 * \return CS configuration
 */
HW_LCDC_SCS_CFG hw_lcdc_get_scs(void);

/**
 * \brief Sets hold flag to bind commands and data
 *
 * \param [in] enable           Enable/disable commands binding with data
 */
void hw_lcdc_set_hold(bool enable);

/**
 * \brief Set the tearing effect detection state
 *
 * \param [in] enable           Enable/disable the tearing effect detection
 * \param [in] polarity         TE level to detect
 */
void hw_lcdc_set_tearing_effect(bool enable, HW_LCDC_TE polarity);

/**
 * \brief Performs a single frame update to the screen using the configured physical interface
 */
void hw_lcdc_send_one_frame(void);

/**
 * \brief Enables the continuous update of the LCD controller.
 *
 * \note Only parallel LCDs (HW_LCDC_PHY_JDI_PARALLEL) support continuous mode update.
 *
 * \param [in] enable           Enable / disable the continuous update
 */
void hw_lcdc_set_continuous_mode(bool enable);

/** \} */

/**
 * \name                MIPI functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Set the configuration register parameters
 *
 * \param [in] cfg              Configuration flag(s) as defined in \ref HW_LCDC_MIPI_CFG
 */
void hw_lcdc_set_mipi_cfg(HW_LCDC_MIPI_CFG cfg);

/**
 * \brief Get the configuration register parameters
 *
 * \return Configuration flag(s) as defined in \ref HW_LCDC_MIPI_CFG
 */
__STATIC_INLINE HW_LCDC_MIPI_CFG hw_lcdc_get_mipi_cfg(void)
{
        return LCDC->LCDC_DBIB_CFG_REG;
}

/**
 * \brief Set QSPI mode
 *
 * \param [in] mode             QSPI mode, \ref HW_LCDC_QSPI_MODE
 */
void hw_lcdc_mipi_set_qpsi_mode(HW_LCDC_QSPI_MODE mode);

/**
 * \brief Send command or parameter to the LCD
 *
 * \param [in] type             Type of instruction as defined in \ref HW_LCDC_MIPI
 * \param [in] value            Command or data as defined in (but not limited to) \ref HW_LCDC_MIPI_DCS
 */
void hw_lcdc_mipi_cmd(HW_LCDC_MIPI type, HW_LCDC_MIPI_DCS value);

/**
 * \brief Send DCS command with parameters to the LCD
 *
 * \param [in] cmd              DCS command to be sent
 * \param [in] params           Command parameters
 * \param [in] param_len        Command parameters length in bytes
 *
 * \return 0 on success, <0: error (\ref HW_LCDC_ERR)
 */
int hw_lcdc_dcs_cmd_params(HW_LCDC_MIPI_DCS cmd, const uint8_t *params, size_t param_len);

/**
 * \brief Send generic command with parameters to the LCD.
 *
 * Can be used for non MIPI interfaces that do not follow the DCS specification
 *
 * \param [in] cmds             Command to be sent
 * \param [in] cmd_len          Command length in bytes
 * \param [in] params           Command parameters
 * \param [in] param_len        Command parameters length in bytes
 *
 * \return 0 on success, <0: error (\ref HW_LCDC_ERR)
 */
int hw_lcdc_gen_cmd_params(const uint8_t *cmds, size_t cmd_len, const uint8_t *params, size_t param_len);

/**
 * \brief Execute a DCS read operation
 *
 * \param [in] cmd              Command to be sent
 * \param [out] data            Data to be received
 * \param [in] data_len         Length in bytes of data to be received
 * \param [in] dummy_ticks      Number of dummy clock ticks between read command and data
 *
 * \return >=0: length of bytes read, <0: error (\ref HW_LCDC_ERR)
 */
int hw_lcdc_dcs_read(HW_LCDC_MIPI_DCS cmd, uint8_t *data, size_t data_len, size_t dummy_ticks);

/**
 * \brief Execute a generic read operation
 *
 * \param [in] cmd              Command to be sent
 * \param [in] cmd_len          Length in bytes of command
 * \param [out] data            Data to be received
 * \param [in] data_len         Length in bytes of data to be received
 * \param [in] dummy_ticks      Number of dummy clock ticks between read command and data
 *
 * \return >=0: length of bytes read, <0: error (\ref HW_LCDC_ERR)
 */
int hw_lcdc_gen_read(const uint8_t *cmd, size_t cmd_len, uint8_t *data, size_t data_len, size_t dummy_ticks);
/** \} */


/**
 * \name                JDI / Sharp functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief JDI serial / Sharp specific function to send a command to the LCD
 *
 * \param [in] cmd              Command byte to be sent
 */
void hw_lcdc_jdi_serial_cmd_send(HW_LCDC_JDIS_CMD cmd);

/**
 * \brief JDI parallel specific function to set exact timings of produced control signals
 */
void hw_lcdc_jdi_parallel(uint16_t resx, uint16_t resy, const hw_lcdc_jdi_par_t *jdi_par);
/** \} */

/**
 * \name                Interrupt functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Set callback function to be called upon an interrupt event
 *
 * \param [in] cb               Callback function to be called
 * \param [in] user_data        Parameters to be passed to callback function
 */
void hw_lcdc_set_callback(hw_lcdc_callback cb, void *user_data);

/**
 * \brief Enable/disable the VSync interrupt
 *
 * \param [in] enable           Enable/disable VSync
 *
 * \note VSYNC and Tearing Effect interrupts are enabled with LCDC_VSYNC_IRQ_EN. To enable tearing
 * effect detection, bit LCDC_DBIB_CFG_REG[LCDC_DBIB_TE_DIS] must be set.
 *
 * \sa hw_lcdc_set_tearing_effect()
 */
__STATIC_INLINE void hw_lcdc_enable_vsync_irq(bool enable)
{
        HW_LCDC_REG_SETF(LCDC_INTERRUPT_REG, VSYNC_IRQ_EN, enable ? 1 : 0);
}

/**
 * \brief Enable/disable the HSync interrupt
 *
 * \param [in] enable           Enable/disable HSync
 */
__STATIC_INLINE void hw_lcdc_enable_hsync_irq(bool enable)
{
        HW_LCDC_REG_SETF(LCDC_INTERRUPT_REG, HSYNC_IRQ_EN, enable ? 1 : 0);
}

/**
 * \brief Enable/disable the "frame end" interrupt
 *
 * \param [in] enable           Enable/disable frame end
 */
__STATIC_INLINE void hw_lcdc_enable_frame_end_irq(bool enable)
{
        HW_LCDC_REG_SETF(LCDC_INTERRUPT_REG, FE_IRQ_EN, enable ? 1 : 0);
}

/**
 * \brief Enable/disable the tearing effect interrupt
 *
 * \param [in] enable           Enable/disable tearing effect
 *
 * \note To enable tearing effect detection, bit LCDC_DBIB_CFG_REG[LCDC_DBIB_TE_DIS] must be set.
 *
 * \sa hw_lcdc_set_tearing_effect()
 */
__STATIC_INLINE void hw_lcdc_enable_tearing_effect_irq(bool enable)
{
        HW_LCDC_REG_SETF(LCDC_INTERRUPT_REG, TE_IRQ_EN, enable ? 1 : 0);
}
/** \} */


/**
 * \name                IO functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Control whether SPI data input is in the same pin with SPI data output
 *
 * \param [in] state            Set to true if SI is in the same pin with SO
 */
__STATIC_INLINE void hw_lcdc_set_spi_sio(bool state)
{
        HW_LCDC_REG_SETF(LCDC_GPIO_REG, GPIO_SPI_SI_ON_SD_PAD, state ? 1 : 0);
}

/**
 * \brief Control the SD (shutdown) pin when the DPI interface is selected
 *
 * \param [in] state            State of the pin
 */
__STATIC_INLINE void hw_lcdc_set_dpi_sd(bool state)
{
        HW_LCDC_REG_SETF(LCDC_GPIO_REG, DPI_SD_ASSERT, state ? 1 : 0);
}

/**
 * \brief Control the CM (color mode) pin when the DPI interface is selected
 *
 * \param [in] state            State of the pin
 */
__STATIC_INLINE void hw_lcdc_set_dpi_cm(bool state)
{
        HW_LCDC_REG_SETF(LCDC_GPIO_REG, DPI_CM_ASSERT, state ? 1 : 0);
}

/**
 * \brief Control the RESX (reset) pin when the DBIB interface is selected
 *
 * \param [in] state            State of the pin
 */
__STATIC_INLINE void hw_lcdc_mipi_set_resx(bool state)
{
        HW_LCDC_MIPI_CFG lcdc_dbib_cfg_reg = hw_lcdc_get_mipi_cfg();

        if (state) {
                lcdc_dbib_cfg_reg |= HW_LCDC_MIPI_CFG_RESX;
        } else {
                lcdc_dbib_cfg_reg &= ~HW_LCDC_MIPI_CFG_RESX;
        }

        hw_lcdc_set_mipi_cfg(lcdc_dbib_cfg_reg);
}
/** \} */

/**
 * \name                State functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Check if the LCD interface is active.
 *
 * \return true, if it is active, else false
 *
 */
__STATIC_FORCEINLINE bool hw_lcdc_is_active(void)
{
        return (REG_GETF(CRG_SYS, CLK_SYS_REG, LCD_ENABLE) == 1);
}

/**
 * \brief Check if the LCD interface is active and clocked by div1 clock.
 *
 * \return true, if it is active and clocked by div1 clock, else false
 *
 */
__STATIC_INLINE bool hw_lcdc_clk_is_div1(void)
{
        uint32_t clk_sys_reg = CRG_SYS->CLK_SYS_REG;

        return ((clk_sys_reg & REG_MSK(CRG_SYS, CLK_SYS_REG, LCD_ENABLE)) &&
                (clk_sys_reg & REG_MSK(CRG_SYS, CLK_SYS_REG, LCD_CLK_SEL)));
}

/** \} */

#endif /* dg_configUSE_HW_LCDC */


#endif /* HW_LCDC_H */

/**
 * \}
 * \}
 */
