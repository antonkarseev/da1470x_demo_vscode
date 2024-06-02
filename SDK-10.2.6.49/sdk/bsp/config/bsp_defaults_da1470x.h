/* Imperative instruction to document global objects via Doxygen special commands */
/*! \file */
/**
 * \addtogroup PLA_BSP_CONFIG
 * \{
 * \addtogroup BSP_CONFIG_DEFAULTS
 * \{
 */
/**
 ****************************************************************************************
 *
 * @file bsp_defaults_da1470x.h
 *
 * @brief Board Support Package. Device-specific system configuration default values.
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BSP_DEFAULTS_DA1470X_H_
#define BSP_DEFAULTS_DA1470X_H_


/* Deprecated configuration options must not be defined by the application. */

#define DG_CONFIG_NA_70X_MSG DG_CONFIG_NOT_APPLICABLE_MSG " for the DA1470x device family."
#define DG_CONFIG_NA_70X_FORCE_ZERO_MSG DG_CONFIG_NA_70X_MSG " Forcing to 0 (not used)."
#define DG_CONFIG_TIMER_NA_MSG DG_CONFIG_NA_70X_MSG " Use the generic dg_configUSE_HW_TIMER instead."

#ifdef dg_configTim1Prescaler
# pragma message "dg_configTim1Prescaler" DG_CONFIG_NA_70X_MSG
# undef  dg_configTim1Prescaler
#endif

#ifdef dg_configTim1PrescalerBitRange
# pragma message "dg_configTim1PrescalerBitRange" DG_CONFIG_NA_70X_MSG
# undef dg_configTim1PrescalerBitRange
#endif

#ifdef dg_configEXT_CRYSTAL_FREQ
# pragma message "dg_configEXT_CRYSTAL_FREQ" DG_CONFIG_NA_70X_MSG
# undef  dg_configEXT_CRYSTAL_FREQ
#endif

#ifdef dg_configUSER_CAN_USE_TIMER1
# pragma message "dg_configUSER_CAN_USE_TIMER1" DG_CONFIG_NA_70X_MSG
# undef  dg_configUSER_CAN_USE_TIMER1
#endif

#ifdef dg_configEMULATE_OTP_COPY
# pragma message "dg_configEMULATE_OTP_COPY" DG_CONFIG_NA_70X_MSG
# undef  dg_configEMULATE_OTP_COPY
#endif

#ifdef dg_configCACHEABLE_QSPI_AREA_LEN
# pragma message "dg_configCACHEABLE_QSPI_AREA_LEN" DG_CONFIG_NA_70X_MSG
# undef  dg_configCACHEABLE_QSPI_AREA_LEN
#endif

#ifdef dg_configUSE_DCDC
# pragma message "dg_configUSE_DCDC" DG_CONFIG_NA_70X_MSG
# undef  dg_configUSE_DCDC
#endif

#ifdef dg_configUSE_USB
# pragma message "dg_configUSE_USB" DG_CONFIG_NA_70X_MSG
# undef  dg_configUSE_USB
#endif

#ifdef dg_configUSE_USB_CHARGER
# pragma message "dg_configUSE_USB_CHARGER" DG_CONFIG_NA_70X_MSG
# undef  dg_configUSE_USB_CHARGER
#endif

#ifdef dg_configALLOW_CHARGING_NOT_ENUM
# pragma message "dg_configALLOW_CHARGING_NOT_ENUM" DG_CONFIG_NA_70X_MSG
# undef  dg_configALLOW_CHARGING_NOT_ENUM
#endif

#ifdef dg_configPOWER_1V8P
# pragma message "dg_configPOWER_1V8P" DG_CONFIG_NA_70X_MSG
# undef  dg_configPOWER_1V8P
#endif

#ifdef dg_configOPTIMAL_RETRAM
# pragma message "dg_configOPTIMAL_RETRAM" DG_CONFIG_NA_70X_MSG
# undef  dg_configOPTIMAL_RETRAM
#endif

#ifdef dg_configSHUFFLING_MODE
# pragma message "dg_configSHUFFLING_MODE" DG_CONFIG_NA_70X_MSG
# undef  dg_configSHUFFLING_MODE
#endif

#if (dg_configUSE_HW_RF != 0)
# pragma message "dg_configUSE_HW_RF" DG_CONFIG_NA_70X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_RF
# define dg_configUSE_HW_RF                             0
#endif

#if (dg_configUSE_HW_COEX != 0)
# pragma message "dg_configUSE_HW_COEX" DG_CONFIG_NA_70X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_COEX
# define dg_configUSE_HW_COEX                           0
#endif

#if (dg_configUSE_HW_ECC != 0)
# pragma message "dg_configUSE_HW_ECC" DG_CONFIG_NA_70X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_ECC
# define dg_configUSE_HW_ECC                            0
#endif

#if (dg_configUSE_HW_ERM != 0)
# pragma message "dg_configUSE_HW_ERM" DG_CONFIG_NA_70X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_ERM
# define dg_configUSE_HW_ERM                            0
#endif

#if (dg_configUSE_HW_LRA != 0)
# pragma message "dg_configUSE_HW_LRA" DG_CONFIG_NA_70X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_LRA
# define dg_configUSE_HW_LRA                            0
#endif

#if (dg_configUSE_IF_PDM != 0)
# pragma message "dg_configUSE_IF_PDM" DG_CONFIG_NA_70X_FORCE_ZERO_MSG
# undef  dg_configUSE_IF_PDM
# define dg_configUSE_IF_PDM                            0
#endif

#if (dg_configUSE_HW_IRGEN != 0)
# pragma message "dg_configUSE_HW_TEMPSENS" DG_CONFIG_NA_70X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_IRGEN
# define dg_configUSE_HW_IRGEN                          0
#endif

#if (dg_configUSE_HW_SMOTOR != 0)
# pragma message "dg_configUSE_HW_SMOTOR" DG_CONFIG_NA_70X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_SMOTOR
# define dg_configUSE_HW_SMOTOR                            0
#endif

#if (dg_configUSE_HW_SOC != 0)
# pragma message "dg_configUSE_HW_SOC" DG_CONFIG_NA_70X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_SOC
# define dg_configUSE_HW_SOC                            0
#endif

#if (dg_configUSE_HW_TRNG != 0)
# pragma message "dg_configUSE_HW_TRNG" DG_CONFIG_NA_70X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_TRNG
# define dg_configUSE_HW_TRNG                           0
#endif

#if (dg_configUSE_HW_TIMER0 != 0)
# pragma message "dg_configUSE_HW_TIMERX" DG_CONFIG_TIMER_NA_MSG
# undef  dg_configUSE_HW_TIMER0
# define dg_configUSE_HW_TIMER0                         0
#endif

#if (dg_configUSE_HW_TIMER1 != 0)
# pragma message "dg_configUSE_HW_TIMERX" DG_CONFIG_TIMER_NA_MSG
# undef  dg_configUSE_HW_TIMER1
# define dg_configUSE_HW_TIMER1                         0
#endif

#if (dg_configUSE_HW_TIMER2 != 0)
# pragma message "dg_configUSE_HW_TIMERX" DG_CONFIG_TIMER_NA_MSG
# undef  dg_configUSE_HW_TIMER2
# define dg_configUSE_HW_TIMER2                         0
#endif

#if (MAIN_PROCESSOR_BUILD) || RUNNING_DOXYGEN

/* ------------------------------- M33 Peripherals -------------------------------------------------- */

/**
 * \addtogroup PERIPHERALS_700_M33 Peripherals for DA1470x - M33 processor
 *
 * \brief Peripheral Selection for the DA1470x Device Family applicable to M33 processor build
 *
 * \note refer to SNC-specific header-file for SNC build default peripheral configuration
 *
 * When enabled the specific low level driver is included in the compilation of the SDK.
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * The default option can be overridden in the application configuration file.
 *
 * \{
   Driver                            | Setting                                | Default option
   --------------------------------- | -------------------------------------- | :------------------:
   AES                               | dg_configUSE_HW_AES                    | 0
   AES HASH                          | dg_configUSE_HW_AES_HASH               | 0
   Cache Controller                  | dg_configUSE_HW_CACHE                  | 1
   HW charger                        | dg_configUSE_HW_CHARGER                | 0
   Clock driver                      | dg_configUSE_HW_CLK                    | 1
   Clock and Power Manager           | dg_configUSE_HW_CPM                    | 1
   Data Cache Controller             | dg_configUSE_HW_DCACHE                 | 0
   Direct Memory Access              | dg_configUSE_HW_DMA                    | 1
   EMMC Host controller              | dg_configUSE_HW_EMMC                   | 0
   General Purpose A-D  Converter    | dg_configUSE_HW_GPADC                  | 1
   General Purpose I/O               | dg_configUSE_HW_GPIO                   | 1
   HASH                              | dg_configUSE_HW_HASH                   | 0
   Inter-Integrated Circuit          | dg_configUSE_HW_I2C                    | 0
   Improved Inter-Integrated Circuit | dg_configUSE_HW_I3C                    | 0
   ISO7816                           | dg_configUSE_HW_ISO7816                | 0
   LCD controller                    | dg_configUSE_HW_LCDC                   | 0
   Memory Protection Unit            | dg_configUSE_HW_MPU                    | 0
   OTP controller                    | dg_configUSE_HW_OTPC                   | 1
   PCM                               | dg_configUSE_HW_PCM                    | 0
   Domain Driver                     | dg_configUSE_HW_PD                     | 1
   Power Domains Controller          | dg_configUSE_HW_PDC                    | 1
   PDM                               | dg_configUSE_HW_PDM                    | 0
   Power Manager                     | dg_configUSE_HW_PMU                    | 1
   QSPI controller                   | dg_configUSE_HW_QSPI                   | 1
   QSPI2 controller                  | dg_configUSE_HW_QSPI2                  | 0
   Real Time Clock                   | dg_configUSE_HW_RTC                    | 1
   SD Analog-Digital Converter       | dg_configUSE_HW_SDADC                  | 1
   Serial Peripheral Interface       | dg_configUSE_HW_SPI                    | 0
   Sample Rate Converter             | dg_configUSE_HW_SRC                    | 0
   Timer                             | dg_configUSE_HW_TIMER                  | 1
   UART                              | dg_configUSE_HW_UART                   | 1
   USB                               | dg_configUSE_HW_USB                    | 1
   USB charger                       | dg_configUSE_HW_USB_CHARGER            | 1
   USB HW port detection             | dg_configUSE_HW_PORT_DETECTION         | 1
   Vad controller                    | dg_configUSE_HW_VAD                    | 0
   Wakeup controller                 | dg_configUSE_HW_WKUP                   | 1
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */

/* -------------------------------- Peripherals (hw_*) selection -------------------------------- */

#ifndef dg_configUSE_HW_AES
#define dg_configUSE_HW_AES                             (0)
#endif

#ifndef dg_configUSE_HW_AES_HASH
#define dg_configUSE_HW_AES_HASH                        (0)
#endif

#ifndef dg_configUSE_HW_CACHE
#define dg_configUSE_HW_CACHE                           (1)
#endif

#ifndef dg_configUSE_HW_CHARGER
#define dg_configUSE_HW_CHARGER                         (0)
#endif

#ifndef dg_configUSE_HW_CLK
#define dg_configUSE_HW_CLK                             (1)
#endif

#ifndef dg_configUSE_HW_CPM
#define dg_configUSE_HW_CPM                             (1)
#endif

#ifndef dg_configUSE_HW_DCACHE
#define dg_configUSE_HW_DCACHE                          (0)
#endif

#ifndef dg_configUSE_HW_DMA
#define dg_configUSE_HW_DMA                             (1)
#endif

#ifndef dg_configUSE_HW_EMMC
#define dg_configUSE_HW_EMMC                            (0)
#endif

#ifndef dg_configUSE_HW_GPADC
#define dg_configUSE_HW_GPADC                           (1)
#endif

#ifndef dg_configUSE_HW_GPIO
#define dg_configUSE_HW_GPIO                            (1)
#endif

#ifndef dg_configUSE_HW_HASH
#define dg_configUSE_HW_HASH                            (0)
#endif

#ifndef dg_configUSE_HW_I2C
#define dg_configUSE_HW_I2C                             (0)
#endif

#ifndef dg_configUSE_HW_I3C
#define dg_configUSE_HW_I3C                             (0)
#endif


#ifndef dg_configUSE_HW_ISO7816
#define dg_configUSE_HW_ISO7816                         (0)
#endif

#ifndef dg_configUSE_HW_LCDC
#define dg_configUSE_HW_LCDC                            (0)
#endif

#ifndef dg_configUSE_HW_MPU
#define dg_configUSE_HW_MPU                             (0)
#endif

#ifndef dg_configUSE_HW_OTPC
#define dg_configUSE_HW_OTPC                            (1)
#endif

#ifndef dg_configUSE_HW_PCM
#define dg_configUSE_HW_PCM                             (0)
#endif

#ifndef dg_configUSE_HW_PD
#define dg_configUSE_HW_PD                              (1)
#endif

#ifndef dg_configUSE_HW_PDC
#define dg_configUSE_HW_PDC                             (1)
#endif

#ifndef dg_configUSE_HW_PDM
#define dg_configUSE_HW_PDM                             (0)
#endif

#ifndef dg_configUSE_HW_PMU
#define dg_configUSE_HW_PMU                             (1)
#endif

#ifndef dg_configUSE_HW_OQSPI
#define dg_configUSE_HW_OQSPI                           (1)
#endif


#ifndef dg_configUSE_HW_QSPI
#define dg_configUSE_HW_QSPI                            (1)
#endif

#ifndef dg_configUSE_HW_QSPI2
#define dg_configUSE_HW_QSPI2                           (0)
#endif

#ifndef dg_configUSE_HW_RTC
#define dg_configUSE_HW_RTC                             (1)
#endif

#ifndef dg_configUSE_HW_SDADC
#define dg_configUSE_HW_SDADC                           (1)
#endif


#ifndef dg_configUSE_HW_SPI
#define dg_configUSE_HW_SPI                             (0)
#endif

#ifndef dg_configUSE_HW_SRC
#define dg_configUSE_HW_SRC                             (0)
#endif

#ifndef dg_configUSE_HW_TIMER
#define dg_configUSE_HW_TIMER                           (1)
#endif

#ifndef dg_configUSE_HW_UART
#define dg_configUSE_HW_UART                            (1)
#endif

#ifndef dg_configUSE_HW_USB
#define dg_configUSE_HW_USB                             (1)
#endif

#ifndef dg_configUSE_HW_USB_CHARGER
#define dg_configUSE_HW_USB_CHARGER                     (1)
#endif

#ifndef dg_configUSE_HW_PORT_DETECTION
#define dg_configUSE_HW_PORT_DETECTION                  (1)
#endif

#ifndef dg_configUSE_HW_VAD
#define dg_configUSE_HW_VAD                             (0)
#endif

#ifndef dg_configUSE_HW_WKUP
#define dg_configUSE_HW_WKUP                            (1)
#endif

#ifndef dg_configGPADC_DMA_SUPPORT
# if dg_configUSE_HW_GPADC
#  define dg_configGPADC_DMA_SUPPORT                    dg_configUSE_HW_DMA
# else
#  define dg_configGPADC_DMA_SUPPORT                    (0)
# endif
#endif

#if (dg_configGPADC_DMA_SUPPORT == 1) && ((dg_configUSE_HW_GPADC == 0) || (dg_configUSE_HW_DMA == 0))
# pragma message "DMA support for GPADC needs both dg_configUSE_HW_GPADC and dg_configUSE_HW_DMA to be 1"
#endif

#ifndef dg_configI2C_DMA_SUPPORT
# if dg_configUSE_HW_I2C
#  define dg_configI2C_DMA_SUPPORT                      dg_configUSE_HW_DMA
# else
#  define dg_configI2C_DMA_SUPPORT                      (0)
# endif
#endif

#if (dg_configI2C_DMA_SUPPORT == 1) && ((dg_configUSE_HW_I2C == 0) || (dg_configUSE_HW_DMA == 0))
# pragma message "DMA support for I2C needs both dg_configUSE_HW_I2C and dg_configUSE_HW_DMA to be 1"
#endif

#ifndef dg_configI3C_DMA_SUPPORT
# if dg_configUSE_HW_I3C
#  define dg_configI3C_DMA_SUPPORT                      dg_configUSE_HW_DMA
# else
#  define dg_configI3C_DMA_SUPPORT                      (0)
# endif
#endif

#if (dg_configI3C_DMA_SUPPORT == 1) && ((dg_configUSE_HW_I3C == 0) || (dg_configUSE_HW_DMA == 0))
# pragma message "DMA support for I3C needs both dg_configUSE_HW_I3C and dg_configUSE_HW_DMA to be 1"
#endif

#ifndef dg_configSPI_DMA_SUPPORT
# if dg_configUSE_HW_SPI
#  define dg_configSPI_DMA_SUPPORT                      dg_configUSE_HW_DMA
# else
#  define dg_configSPI_DMA_SUPPORT                      (0)
# endif
#endif

#if (dg_configSPI_DMA_SUPPORT == 1) && ((dg_configUSE_HW_SPI == 0) || (dg_configUSE_HW_DMA == 0))
# pragma message "DMA support for SPI needs both dg_configUSE_HW_SPI and dg_configUSE_HW_DMA to be 1"
#endif

#ifndef dg_configUART_DMA_SUPPORT
# if dg_configUSE_HW_UART
#  define dg_configUART_DMA_SUPPORT                     dg_configUSE_HW_DMA
# else
#  define dg_configUART_DMA_SUPPORT                     (0)
# endif
#endif

#if (dg_configUART_DMA_SUPPORT == 1) && ((dg_configUSE_HW_UART == 0) || (dg_configUSE_HW_DMA == 0))
# pragma message "DMA support for UART needs both dg_configUSE_HW_UART and dg_configUSE_HW_DMA to be 1"
#endif

/**
 * \}
 */
#endif /* MAIN_PROCESSOR_BUILD) || RUNNING_DOXYGEN */

#if (SNC_PROCESSOR_BUILD) || RUNNING_DOXYGEN

/* ------------------------------- SNC Peripherals -------------------------------------------------- */

/**
 * \addtogroup PERIPHERALS_700_SNC Peripherals for DA1470x - SNC processor
 *
 * \brief Peripheral Selection for the DA1470x Device Family applicable to SNC processor build
 *
 * \note refer to M33-specific header-file for M33 build default peripheral configuration
 *
 * When enabled the specific low level driver is included in the compilation of the SDK.
 * - 0 : Disabled
 * - 1 : Enabled
 * - N/A : Not Available (forced to system default)
 *
 * The default option can be overridden in the application configuration file.
 *
 * \{
   Driver                            | Setting                                | Default option
   --------------------------------- | -------------------------------------- | :------------------:
   AES                               | dg_configUSE_HW_AES                    | N/A
   AES HASH                          | dg_configUSE_HW_AES_HASH               | N/A
   Cache Controller                  | dg_configUSE_HW_CACHE                  | N/A
   HW charger                        | dg_configUSE_HW_CHARGER                | N/A
   Clock driver                      | dg_configUSE_HW_CLK                    | 1
   Clock and Power Manager           | dg_configUSE_HW_CPM                    | N/A
   Data Cache Controller             | dg_configUSE_HW_DCACHE                 | N/A
   Direct Memory Access              | dg_configUSE_HW_DMA                    | N/A
   EMMC Host controller              | dg_configUSE_HW_EMMC                   | N/A
   General Purpose A-D  Converter    | dg_configUSE_HW_GPADC                  | 1
   General Purpose I/O               | dg_configUSE_HW_GPIO                   | 1
   HASH                              | dg_configUSE_HW_HASH                   | N/A
   Inter-Integrated Circuit          | dg_configUSE_HW_I2C                    | 0
   Improved Inter-Integrated Circuit | dg_configUSE_HW_I3C                    | 0
   ISO7816                           | dg_configUSE_HW_ISO7816                | 0
   LCD controller                    | dg_configUSE_HW_LCDC                   | N/A
   Memory Protection Unit            | dg_configUSE_HW_MPU                    | N/A
   OTP controller                    | dg_configUSE_HW_OTPC                   | N/A
   PCM                               | dg_configUSE_HW_PCM                    | 0
   Domain Driver                     | dg_configUSE_HW_PD                     | 1
   Power Domains Controller          | dg_configUSE_HW_PDC                    | 1
   PDM                               | dg_configUSE_HW_PDM                    | 0
   Power Manager                     | dg_configUSE_HW_PMU                    | N/A
   OQSPI controller                  | dg_configUSE_HW_OQSPI                  | N/A
   QSPI controller                   | dg_configUSE_HW_QSPI                   | N/A
   QSPI2 controller                  | dg_configUSE_HW_QSPI2                  | N/A
   Real Time Clock                   | dg_configUSE_HW_RTC                    | 1
   SD Analog-Digital Converter       | dg_configUSE_HW_SDADC                  | N/A
   Serial Peripheral Interface       | dg_configUSE_HW_SPI                    | 0
   Sample Rate Converter             | dg_configUSE_HW_SRC                    | 0
   Timer                             | dg_configUSE_HW_TIMER                  | 1
   UART                              | dg_configUSE_HW_UART                   | 1
   USB                               | dg_configUSE_HW_USB                    | N/A
   USB charger                       | dg_configUSE_HW_USB_CHARGER            | N/A
   USB HW port detection             | dg_configUSE_HW_PORT_DETECTION         | N/A
   Vad controller                    | dg_configUSE_HW_VAD                    | 0
   Wakeup controller                 | dg_configUSE_HW_WKUP                   | 1
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */

/* -------------------------------- Peripherals (hw_*) selection -------------------------------- */

#ifndef dg_configUSE_HW_CLK
#define dg_configUSE_HW_CLK                             (1)
#endif

#ifndef dg_configUSE_HW_GPADC
#define dg_configUSE_HW_GPADC                           (1)
#endif

#ifndef dg_configUSE_HW_GPIO
#define dg_configUSE_HW_GPIO                            (1)
#endif

#ifndef dg_configUSE_HW_I2C
#define dg_configUSE_HW_I2C                             (0)
#endif

#ifndef dg_configUSE_HW_I3C
#define dg_configUSE_HW_I3C                             (0)
#endif


#ifndef dg_configUSE_HW_ISO7816
#define dg_configUSE_HW_ISO7816                         (0)
#endif

#ifndef dg_configUSE_HW_PCM
#define dg_configUSE_HW_PCM                             (0)
#endif

#ifndef dg_configUSE_HW_PD
#define dg_configUSE_HW_PD                              (1)
#endif

#ifndef dg_configUSE_HW_PDC
#define dg_configUSE_HW_PDC                             (1)
#endif

#ifndef dg_configUSE_HW_PDM
#define dg_configUSE_HW_PDM                             (0)
#endif

#ifndef dg_configUSE_HW_RTC
#define dg_configUSE_HW_RTC                             (1)
#endif

#ifndef dg_configUSE_HW_SPI
#define dg_configUSE_HW_SPI                             (0)
#endif

#ifndef dg_configUSE_HW_SRC
#define dg_configUSE_HW_SRC                             (0)
#endif

#ifndef dg_configUSE_HW_TIMER
#define dg_configUSE_HW_TIMER                           (1)
#endif

#ifndef dg_configUSE_HW_UART
#define dg_configUSE_HW_UART                            (1)
#endif

#ifndef dg_configUSE_HW_VAD
#define dg_configUSE_HW_VAD                             (0)
#endif

#ifndef dg_configUSE_HW_WKUP
#define dg_configUSE_HW_WKUP                            (1)
#endif

#ifndef dg_configGPADC_DMA_SUPPORT
# define dg_configGPADC_DMA_SUPPORT                     (0)
#endif

#ifndef dg_configI2C_DMA_SUPPORT
# define dg_configI2C_DMA_SUPPORT                       (0)
#endif

#ifndef dg_configI3C_DMA_SUPPORT
# define dg_configI3C_DMA_SUPPORT                       (0)
#endif

#ifndef dg_configSPI_DMA_SUPPORT
# define dg_configSPI_DMA_SUPPORT                       (0)
#endif

#ifndef dg_configUART_DMA_SUPPORT
# define dg_configUART_DMA_SUPPORT                      (0)
#endif

/**
 * \}
 */

#endif /* SNC_PROCESSOR_BUILD || RUNNING_DOXYGEN */

/* ------------------------------- Clock Settings ----------------------------------------------- */

/**
 * \addtogroup CLOCK_SETTINGS
 *
 * \{
 */

#if (dg_configUSE_LP_CLK != LP_CLK_32000) && (dg_configUSE_LP_CLK != LP_CLK_32768) && (dg_configUSE_LP_CLK != LP_CLK_RCX) && (dg_configUSE_LP_CLK != LP_CLK_ANY)
#error "dg_configUSE_LP_CLK has invalid setting"
#endif

#if (dg_configUSE_LP_CLK == LP_CLK_ANY)
#pragma message "In order to support the option LP_CLK_ANY for the low-power clock source, "\
                "some configuration options MUST be defined by the application, including "\
                "dg_configMIN_SLEEP_TIME, dg_configIMAGE_COPY_TIME, dg_configPM_MAX_ADAPTER_DEFER_TIME, "\
                "OS_TICK_PERIOD, BLE_WUP_LATENCY, sleep_duration_in_lp_cycles and rwip_check_wakeup_boundary. "
#pragma message "Additionally, some device-specific configuration options MUST be defined by the application, "\
                "including dg_configXTAL32K_FREQ."
#endif

#ifndef dg_configXTAL32M_FREQ
#define dg_configXTAL32M_FREQ                           (32000000)
#endif

#ifndef dg_configRCHS_32M_FREQ
#define dg_configRCHS_32M_FREQ                          (32000000)
#endif
#ifndef dg_configRCHS_64M_FREQ
#define dg_configRCHS_64M_FREQ                          (64000000)
#endif
#ifndef dg_configRCHS_96M_FREQ
#define dg_configRCHS_96M_FREQ                          (96000000)
#endif

#ifndef dg_configRCHS_FREQ_MIN
#define dg_configRCHS_FREQ_MIN                          (30600000)
#endif

#ifndef dg_configDIVN_FREQ
#define dg_configDIVN_FREQ                              (32000000)
#endif

#ifndef dg_configPLL160M_FREQ
#define dg_configPLL160M_FREQ                           (160000000)
#endif

#if dg_configUSE_LP_CLK == LP_CLK_32768
# undef dg_configXTAL32K_FREQ
# define dg_configXTAL32K_FREQ                          (32768)
#elif dg_configUSE_LP_CLK == LP_CLK_32000
# undef dg_configXTAL32K_FREQ
# define dg_configXTAL32K_FREQ                          (32000)
#elif dg_configUSE_LP_CLK == LP_CLK_RCX
# undef dg_configXTAL32K_FREQ
# define dg_configXTAL32K_FREQ                          (0)
#endif

/**
 * \brief Value of the RC32K oscillator frequency in Hz
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configRC32K_FREQ
#define dg_configRC32K_FREQ                             (32000)
#endif

/**
 * \brief Acceptable clock tick drift (in parts per million) for the Low-power clock
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configLP_CLK_DRIFT
# if dg_configUSE_LP_CLK == LP_CLK_RCX
#  define dg_configLP_CLK_DRIFT                         (500) //ppm
# else
#  define dg_configLP_CLK_DRIFT                         (50) //ppm
# endif
#endif

/**
 * \brief Time needed for the settling of the LP clock, in msec.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configXTAL32K_SETTLE_TIME
# if dg_configLP_CLK_SOURCE == LP_CLK_IS_ANALOG
#  define dg_configXTAL32K_SETTLE_TIME                  (8000)
# else
#  define dg_configXTAL32K_SETTLE_TIME                  (1000)
# endif
#endif

/**
 * \brief XTAL32M settle time
 *
 * Time needed for the settling of the XTAL32M, in usec.
 *
 * \note If set to zero, the settling time will be automatically adjusted.
 */
#ifndef dg_configXTAL32M_SETTLE_TIME_IN_USEC
#define dg_configXTAL32M_SETTLE_TIME_IN_USEC            (0)
#endif

/**
 * \brief Enable XTAL32M upon system wake-up
 *
 * If set to 1 the PDC will enable XTAL32M when it wakes-up M33
 *
 */
#ifndef dg_configENABLE_XTAL32M_ON_WAKEUP
#define dg_configENABLE_XTAL32M_ON_WAKEUP               (0)
#endif

/**
 * \brief The time in us needed to wake-up in normal wake-up mode and RCLP at 32 KHz
 *
 * This is the maximum time needed to wake-up the chip and start executing code
 * using RCLP at 32 KHz in normal wake-up mode.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#define dg_configWAKEUP_RCLP32_NORMAL                   (350)

/**
 * \brief The time in us needed to wake-up in normal wake-up mode, RCLP at 512KHz and VDD changes from 0.9 V to 1.2 V during wake up.
 *
 * This is the maximum time needed to wake-up the chip and start executing code
 * using RCLP at 512 KHz in normal wakeup mode, VDD sleep 0.9 V and VDD active 1.2V. According to datasheet this time is 74us
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#define dg_configWAKEUP_RCLP512_NORMAL_VDD_0V90_TO_1V2   (74)

/**
 * \brief The time in us needed to wake-up in normal wake-up mode RCLP at 512Khz and VDD sleep and active in same level.
 *
 * This is the maximum time needed to wake-up the chip and start executing code
 * using RCLP at 512 KHz in normal wake up mode and VDD sleep and active in same level. According to datasheet this time is 56us
 *
 * \note A safe guard margin on top of 56us has been added.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#define dg_configWAKEUP_RCLP512_NORMAL_VDD_SAME         (74)


/**
 * \brief The time in us needed to wake up in fast wake up mode
 *
 * This is the maximum time needed to wake-up the chip and start executing code.
 * According to datasheet this time is 10us
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#define dg_configWAKEUP_RCLP512_FAST                    (10)

/**
 * \brief XTAL32M trimming default settings
 */
#ifndef dg_configDEFAULT_XTAL32M_START_REG__XTAL32M_TRIM__VALUE
#define dg_configDEFAULT_XTAL32M_START_REG__XTAL32M_TRIM__VALUE                 (0x0)
#endif

#ifndef dg_configDEFAULT_XTAL32M_START_REG__XTAL32M_CUR_SET__VALUE
#define dg_configDEFAULT_XTAL32M_START_REG__XTAL32M_CUR_SET__VALUE              (0xE)
#endif

#if (MAIN_PROCESSOR_BUILD)
# ifndef dg_configUSE_CLOCK_MGR
#  ifdef OS_PRESENT
#   define dg_configUSE_CLOCK_MGR                       (1)
#  else
#   define dg_configUSE_CLOCK_MGR                       (0)
#  endif
# endif
#endif /* PROCESSOR_BUILD */

/**
 * \}
 */

/* ------------------------------- System configuration settings -------------------------------- */

/**
 * \addtogroup SYSTEM_CONFIGURATION_SETTINGS
 *
 * \{
 */

#if (dg_configUSE_WDOG == 0) && defined(dg_configWDOG_IDLE_RESET_VALUE)
# pragma message "dg_configWDOG_IDLE_RESET_VALUE is ignored. Maximum watchdog value will be used."
# undef dg_configWDOG_IDLE_RESET_VALUE
#endif

/**
 * \brief Reset value for Watchdog when system is idle.
 */
#ifndef dg_configWDOG_IDLE_RESET_VALUE
#define dg_configWDOG_IDLE_RESET_VALUE  (SYS_WDOG_WATCHDOG_REG_WDOG_VAL_Msk >> SYS_WDOG_WATCHDOG_REG_WDOG_VAL_Pos)
#endif

/**
 * \brief Maximum watchdog tasks
 *
 * Maximum number of tasks that the Watchdog Service can monitor. It can be larger (up to 32) than
 * needed, at the expense of increased Retention Memory requirement.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configWDOG_MAX_TASKS_CNT
#define dg_configWDOG_MAX_TASKS_CNT                     (6)
#endif

/*
 * \brief When set to 1, the LEDs are used from M33.
 *        When set to 0, the LEDs are used from SNC.
 */
#ifndef dg_configM33_USES_LEDS
#define dg_configM33_USES_LEDS                          (1)
#endif

/*
 * \brief When set to 1, the GPIO configuration becomes static, i.e. it does not change during runtime.
 *        When set to 0, the GPIO configuration can change during runtime.
 */
#ifndef dg_configUSE_STATIC_IO_CONFIG
#define dg_configUSE_STATIC_IO_CONFIG                   (0)
#endif

/**
 * \brief System debug logging protection mechanism.
 *
 *        When set to 1, a mutual exclusion mechanism is employed and any ongoing printing activity
 *        will not be interpolated by another printing attempt that is initiated from another system
 *        processing unit (e.g. SNC) or from another task running in the main processing unit (M33).
 *        The mechanism supports both M33-only and M33-SNC applications. M33 baremetal build configurations
 *        are not supported. For SNC both OS-based and baremetal build configurations are supported.
 *        In particular, the libC standard output functions are overridden in sdk/bsp/startup/config.c by a:
 *              - custom printf, in case the debug logging string contains:
 *                     -# only characters, e.g. printf("a b c d e f"); or any char-only string prefixed
 *                        with newline char ("\n"), e.g. printf("\na b c d e f");
 *                     -# any format specifiers (subsequences beginning with %), e.g. printf("a %d c\n",2);
 *              - custom puts, in case the debug logging string contains one or more newline chars ("\n")
 *                      in the end but no format specifiers, e.g. printf("\na b c d e f\n"); or printf("f\n");
 *              - custom putchar, in case the debug logging string is of one only character, even if it is
 *                      an escaping one, e.g. printf("#"); or printf("\n");
 *
 *        For multi-processor M33-SNC applications the debug logging string originating from each processing
 *        unit printing attempt is prefixed with a "[M33]: " and a "[SNC]: " respectively for readability
 *        purposes. For simple putchar() calls the prefix is discarded for the same reasons.
 *
 *        When set to 0, contenting printing attempts initiated from different contexts may end up in a race
 *        condition that can result in a disordered and unreadable serial output.
 *
 *        The debug logging protection mechanism cannot be employed if the console service is enabled.
 *
 *        \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configSYS_DBG_LOG_PROTECTION
# if (!dg_configUSE_CONSOLE)
#       define dg_configSYS_DBG_LOG_PROTECTION                  (1)
# endif
#endif

/* The system debug logging protection mechanism does not support M33 baremetal build configurations. */
#if (dg_configSYS_DBG_LOG_PROTECTION == 1)
# if (MAIN_PROCESSOR_BUILD)
#  ifdef OS_BAREMETAL
#   undef dg_configSYS_DBG_LOG_PROTECTION
#       define dg_configSYS_DBG_LOG_PROTECTION                  (0)
#  endif
# endif
#endif

/* Maximum number of characters of a debug logging string that can be printed at a time. A set of chars is
 * also reserved for printing the processing unit prefix where the string originated from. If the string
 * is greater in length than the maximum characters minus the prefix then an error message is displayed
 * instead. This prefix related limitation does not apply for single processing unit applications. */
#if dg_configSYS_DBG_LOG_PROTECTION
# define dg_configSYS_DBG_LOG_MAX_SIZE                  200
#endif

/**
 * \brief Select BSR locking protection mechanism.
 *
 *  - SW_BSR_IMPLEMENTATION: Software BSR implementation for exclusive locking of resource per master is used.
 *  - HW_BSR_IMPLEMENTATION: Hardware BSR implementation for exclusive locking of resource per master is used.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configBSR_IMPLEMENTATION
#define dg_configBSR_IMPLEMENTATION                     (SW_BSR_IMPLEMENTATION)
#endif

/**
 * \brief When set to 1, the sys trng service is enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#if (MAIN_PROCESSOR_BUILD)
#ifndef dg_configUSE_SYS_TRNG
#define dg_configUSE_SYS_TRNG                           (1)
#endif
#endif /* PROCESSOR_BUILD */

#if dg_configUSE_SYS_TRNG
/**
 * \brief A pointer to the physical address of the SYSRAM that is used as entropy source.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 *
 * \note The address MUST be word aligned!
 */
# ifndef dg_configSYS_TRNG_ENTROPY_SRC_ADDR
#   define dg_configSYS_TRNG_ENTROPY_SRC_ADDR           MEMORY_SYSRAM10_BASE
# endif /* dg_configSYS_TRNG_ENTROPY_SRC_ADDR */
#endif /* dg_configUSE_SYS_TRNG */

/**
 * \brief When set to 1, the sys drbg service is enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#if (MAIN_PROCESSOR_BUILD)
#ifndef dg_configUSE_SYS_DRBG
#define dg_configUSE_SYS_DRBG                           (1)
#endif
#endif /* PROCESSOR_BUILD */

/**
 * \brief The length of the buffer which holds the random numbers.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SYS_DRBG_BUFFER_LENGTH
#define dg_configUSE_SYS_DRBG_BUFFER_LENGTH             (30)
#endif

/**
 * \brief Threshold (index) in the buffer which holds the random numbers. When the buffer index
 *        reaches the threshold or becomes greater than the threshold a request for buffer update
 *        will be issued.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SYS_DRBG_BUFFER_THRESHOLD
#define dg_configUSE_SYS_DRBG_BUFFER_THRESHOLD          (24)
#endif

/**
 * \brief When set to 1, the ChaCha20 random number generator is enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_CHACHA20_RAND
#define dg_configUSE_CHACHA20_RAND                      (1)
#endif

/**
 * \brief When set to 1, the stdlib.h random number generator is enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_STDLIB_RAND
#define dg_configUSE_STDLIB_RAND                        (0)
#endif

#if ((dg_configUSE_CHACHA20_RAND + dg_configUSE_STDLIB_RAND) != 1)
      #error "Only one random number generator must be enabled each time."
#endif

/*
 * \brief When set to 1, PD_SNC is enabled by power manager when Cortex-M33 master is active. This allows
 * the master to have access to I2C, I3C, SPI, UART and GPADC interfaces.
 * When set to 0, PD_SNC can be enabled by the adapters or the application. PDC can also be configured to
 * enable PD_SNC by setting the appropriate flag in the PDC LUT entry.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configPM_ENABLES_PD_SNC_WHILE_ACTIVE
#define dg_configPM_ENABLES_PD_SNC_WHILE_ACTIVE             (1)
#endif

/**
 * \brief Enable flash background operations.
 *
 * The flash background operations API is responsible to handle the sector erase and page write
 * operations of the XiP flash memory when the system is idle. If an XiP read operation is requested,
 * while an sector erase operation is ongoing, the API suspends the erase operation, serves the XiP
 * request and in turns resumes the sector erase operation. When enabled, the user must call the
 * sys_background_flash_ops_erase_sector() and sys_background_flash_ops_write_page() in order to
 * register a sector erase or page write operation. The rest API
 *
 * \warning     The background flash operations are not supported by bare metal projects.
 *
 * \warning     When the flash background operations are enabled, the oqspi_automode_erase_flash_sector()
 *              and oqspi_automode_write_flash_page() should never be used, because they will bypass
 *              the background flash operations.
 *
 * \warning     It must be always disabled for SNC.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#if (MAIN_PROCESSOR_BUILD) && defined(OS_PRESENT) && (dg_configUSE_HW_OQSPI)
# ifndef dg_configUSE_SYS_BACKGROUND_FLASH_OPS
# define dg_configUSE_SYS_BACKGROUND_FLASH_OPS          (1)
# endif
#else
# ifndef dg_configUSE_SYS_BACKGROUND_FLASH_OPS
# define dg_configUSE_SYS_BACKGROUND_FLASH_OPS          (0)
# endif
#endif

#if (dg_configUSE_SYS_BACKGROUND_FLASH_OPS) && !defined(OS_PRESENT)
#error "The background flash operations are not supported by bare metal projects"
#endif

#if (dg_configUSE_SYS_BACKGROUND_FLASH_OPS && SNC_PROCESSOR_BUILD)
#error "The background flash operations are not supported by SNC builds"
#endif

#if (dg_configUSE_SYS_BACKGROUND_FLASH_OPS && !dg_configUSE_HW_OQSPI)
#error "The low level driver of the OQSPI controller must be enabled (dg_configUSE_HW_OQSPI = 1)"
#endif

/**
 * \}
 */

/* ----------------------------------- USB Configuration ---------------------------------------- */

/**
 * \addtogroup USB_SETTINGS USB configuration settings
 *
 * \brief USB configuration settings
 *
 * \{
 */

/**
 * \brief Controls how the system will behave when the USB i/f is suspended.
 *
 * \details When the USB Node is suspended by the USB Host, the application may have to act in
 *          order to comply with the USB specification (consume less than 2.5mA). The available
 *          options are:
 *          USB_SUSPEND_MODE_NONE  (0): do nothing
 *          USB_SUSPEND_MODE_PAUSE (1): pause system clock => the LP clock is stopped and only VBUS and USB irqs are handled
 *          USB_SUSPEND_MODE_IDLE  (2): pause application => The system is not paused but the application must stop all
 *                                      timers and make sure all tasks are blocked.
 *
 *          In both modes PAUSE and IDLE, the application must make sure that all external peripherals are
 *          either powered off or placed in the lowest power consumption mode.
 */
#ifndef dg_configUSB_SUSPEND_MODE
#define dg_configUSB_SUSPEND_MODE                       USB_SUSPEND_MODE_IDLE
#endif

/**
 * \brief Enable the DMA for reading/writing data to USB EP.\n
 * By default the USB DMA is not enabled.\n
 * To enable the DMA for the USB, set this the macro to value (1) in the custom_config_xxx.h file.\n
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSB_DMA_SUPPORT
#define dg_configUSB_DMA_SUPPORT                        (0)
#endif

/**
 * \}
 */

/* -------------------------------------- Flash settings ---------------------------------------- */

/**
 * \addtogroup FLASH_SETTINGS
 *
 * \{
 */

/**
 * \brief When set to 1, the flash connected to OQSPIC is put to power-down state during sleep.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configOQSPI_FLASH_POWER_DOWN
#define dg_configOQSPI_FLASH_POWER_DOWN                 (0)
#endif

/**
 * \brief When set to 1, the Flash connected to OQSPIC is powered off during sleep.
 */
#ifndef dg_configOQSPI_FLASH_POWER_OFF
#define dg_configOQSPI_FLASH_POWER_OFF                  (0)
#endif

#if dg_configOQSPI_FLASH_POWER_DOWN && dg_configOQSPI_FLASH_POWER_OFF
#error "Choose either dg_configOQSPI_FLASH_POWER_DOWN or dg_configOQSPI_FLASH_POWER_OFF."
#endif

/**
 * \brief Enable the Flash Auto-detection mode for OQSPIC
 *
 * \warning THIS WILL GREATLY INCREASE THE CODE SIZE AND RETRAM USAGE!!! MAKE SURE YOUR PROJECT
 *          CAN SUPPORT THIS.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configOQSPI_FLASH_AUTODETECT
#define dg_configOQSPI_FLASH_AUTODETECT                 (0)
#endif

#if (dg_configOQSPI_FLASH_AUTODETECT == 1) && (dg_configUSE_HW_OQSPI == 0)
# error "dg_configOQSPI_FLASH_AUTODETECT cannot be enabled if the dg_configUSE_HW_OQSPI = 0"
#endif

#if (dg_configOQSPI_FLASH_AUTODETECT == 1) && (dg_configOQSPI_FLASH_CONFIG_VERIFY == 1)
# error "dg_configOQSPI_FLASH_AUTODETECT and dg_configOQSPI_FLASH_CONFIG_VERIFY are mutually exclusive options"
#endif

/**
 * \brief The header file where the custom OQSPI flash configuration table is instantiated.
 *
 * This compilation option has effect when the dg_configOQSPI_FLASH_AUTODETECT is set.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configOQSPI_MEMORY_CONFIG_TABLE_HEADER
#define dg_configOQSPI_MEMORY_CONFIG_TABLE_HEADER        "oqspi_memory_config_table_internal.h"
#endif

/**
 * \brief Set the OQSPI flash page size.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configOQSPI_FLASH_MAX_WRITE_SIZE
#define dg_configOQSPI_FLASH_MAX_WRITE_SIZE             (128)
#endif

#if (dg_configOQSPI_FLASH_MAX_WRITE_SIZE > 256)
# error "dg_configOQSPI_FLASH_MAX_WRITE_SIZE cannot be higher than the OQSPI Flash page size (256)"
#endif

/**
 * \brief When set to 1, the Flash is powered off during sleep.
 */
#ifndef dg_configFLASH_POWER_OFF
#define dg_configFLASH_POWER_OFF                        (0)
#endif

/**
 * \brief Set the Drive Strength of the Octa SPI Controller
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 *
 * \sa HW_OQSPI_DRIVE_CURRENT
 */
#ifndef dg_configOQSPI_DRIVE_CURRENT
#define dg_configOQSPI_DRIVE_CURRENT                    (HW_OQSPI_DRIVE_CURRENT_4)
#endif

/**
 * \brief Set the Slew Rate of the Octa SPI Controller
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 *
 * \sa HW_OQSPI_SLEW_RATE
 */
#ifndef dg_configOQSPI_SLEW_RATE
#define dg_configOQSPI_SLEW_RATE                        (HW_OQSPI_SLEW_RATE_0)
#endif

/**
 * \brief Select whether the Octa SPI Flash memory will be erased in Auto or in Manual Access Mode.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dgconfigOQSPI_ERASE_IN_AUTOMODE
#define dgconfigOQSPI_ERASE_IN_AUTOMODE                 (1)
#endif

#if (dg_configOQSPI_FLASH_AUTODETECT == 0)

/**
 * \brief The Octa SPI Flash Driver header file to include
 *
 * The header file must be in the include path of the compiler
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configOQSPI_FLASH_HEADER_FILE
#define dg_configOQSPI_FLASH_HEADER_FILE                "oqspi_at25sl128.h" // "oqspi_mx25u6432.h"
#endif /* dg_configOQSPI_FLASH_HEADER_FILE */

/**
 * \brief The Octa Flash Driver configuration structure
 *
 * The configuration structure must be in the include path of the compiler
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configOQSPI_FLASH_CONFIG
#define dg_configOQSPI_FLASH_CONFIG                      oqspi_at25sl128_cfg
#endif /* dg_configOQSPI_FLASH_CONFIG */

#elif (dg_configOQSPI_FLASH_AUTODETECT == 1)
/**
 * \brief Delay after RESET sequence (in usec)
 *
 * When dg_configOQSPI_FLASH_AUTODETECT is enabled a Flash reset sequence must be applied before
 * reading the JEDEC ID. Since the memory is unkwown the corresponding Reset Delay cannot be fetched
 * by the Flash memory driver.
 *
 * \warning This delay must be equal or higher than the minimum required reset time of all supported
 *          flash memories. Consider re-defining this macro, if necessary, based on the datasheets
 *          of the corresponding flash memories.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configOQSPI_FLASH_AUTODETECT_RESET_DELAY
#define dg_configOQSPI_FLASH_AUTODETECT_RESET_DELAY     (12000)
#endif

#endif /* dg_configOQSPI_FLASH_AUTODETECT */

/**
 * \brief Flash device configuration verification.
 *
 * When set, the API matches the OQSPI Flash JEDEC ID with the JEDEC ID of the selected Flash Driver
 *
 * Applicable only when flash auto detection is not enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configOQSPI_FLASH_CONFIG_VERIFY
#define dg_configOQSPI_FLASH_CONFIG_VERIFY              (0)
#endif

/**
 * \brief Enable the Auto-detection mode for QSPIC2 device
 *
 * \warning THIS WILL GREATLY INCREASE THE CODE SIZE AND RETRAM USAGE!!! MAKE SURE YOUR PROJECT
 *          CAN SUPPORT THIS.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configQSPIC2_DEV_AUTODETECT
#define dg_configQSPIC2_DEV_AUTODETECT                  (0)
#endif

#if dg_configQSPIC2_DEV_AUTODETECT == 0

/**
 * \brief The QSPI  2 Driver header file to include
 *
 * The header file must be in the include path of the compiler
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPIC2_DEV_HEADER_FILE
#define dg_configQSPIC2_DEV_HEADER_FILE                 "psram_aps6404jsq.h"
#endif

/**
 * \brief The QSPI 2 Driver configuration structure
 *
 * The configuration structure must be in the include path of the compiler
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPIC2_DEV_CONFIG
#define dg_configQSPIC2_DEV_CONFIG                      psram_aps6404jsq_config
#endif

#endif /* dg_configQSPIC2_DEV_AUTODETECT  == 0 */

#if dg_configFLASH_AUTODETECT == 0

/**
 * \brief The Flash Driver header file to include
 *
 * The header file must be in the include path of the compiler
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configFLASH_HEADER_FILE
#define dg_configFLASH_HEADER_FILE                      "qspi_at25sl128.h"
#endif /* dg_configFLASH_HEADER_FILE */

/**
 * \brief The Flash Driver configuration structure
 *
 * The configuration structure must be in the include path of the compiler
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configFLASH_CONFIG
#define dg_configFLASH_CONFIG                           flash_at25sl128_config
#endif /* dg_configFLASH_CONFIG */

#endif /* dg_configFLASH_AUTODETECT == 0 */

#if (dg_configFLASH_AUTODETECT == 0) || (dg_configQSPIC2_DEV_AUTODETECT == 0)

/**
 * \brief Flash device configuration verification.
 *
 * When set to 1, the Flash device id configuration is checked against the JEDEC ID read
 * from the controller.
 *
 * Applicable only when flash auto detection is not enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configFLASH_CONFIG_VERIFY
#define dg_configFLASH_CONFIG_VERIFY    (0)
#endif
#endif /* (dg_configFLASH_AUTODETECT == 0) || (dg_configQSPIC2_DEV_AUTODETECT == 0) */

/**
 * \}
 */

/* ----------------------------------- Charger settings ----------------------------------------- */

/**
 * \addtogroup CHARGER_SETTINGS Charger configuration settings
 *
 * \{
 */

/**
 * \brief When set to 1, State of Charge function is enabled.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configUSE_SOC
#define dg_configUSE_SOC                                (0)
#endif

/**
 * \}
 */

/* ----------------------------------- UART settings -------------------------------------------- */

/**
 * \addtogroup UART_SETTINGS
 *
 * \{
 */

/**
 * \brief UART3's software FIFO size
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUART3_SOFTWARE_FIFO_SIZE
#define dg_configUART3_SOFTWARE_FIFO_SIZE               (0)
#endif

/**
 * \brief UART3's Circular DMA buffer size for RX
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUART3_RX_CIRCULAR_DMA_BUF_SIZE
# define dg_configUART3_RX_CIRCULAR_DMA_BUF_SIZE        (0)
#endif

/**
 * \}
 */

/* ----------------------------------- MPU settings -------------------------------------------- */

/**
 * \addtogroup MPU_SETTINGS
 *
 * \{
 */

/**
 * \brief MPU region for CMAC protection
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configCMAC_PROTECT_REGION
#define dg_configCMAC_PROTECT_REGION                    MPU_REGION_6
#endif

/**
 * \brief MPU region for IVT protection
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configIVT_PROTECT_REGION
#define dg_configIVT_PROTECT_REGION                     MPU_REGION_7
#endif

/**
 * \}
 */

/* ----------------------------------- GPU Configuration ---------------------------------------- */

/**
 * \addtogroup GPU_SETTINGS GPU configuration settings
 *
 * \brief GPU configuration settings
 *
 * \{
 */

/**
 * \brief Enables the GPU.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_GPU
#define dg_configUSE_GPU                                (0)
#endif

/**
 * \}
 */

/*
 */
/*------------------------------------ BOARDS DEFINITIONS ----------------------------------------*/

/**
 * \brief Set the board that is used.
 */
#ifndef dg_configUSE_BOARD
# include "boards/brd_prodk_da1470x.h"
# define dg_configUSE_BOARD
#endif

/*------------------------------------ SYSTEM PROTECTION SETTINGS ----------------------------------------*/
/**
 * \addtogroup SYSTEM_PROTECTION_SETTINGS System protection settings
 *
 * \brief System protection settings that prohibit or warn when enabling unsupported HW features.
 *
 * \{
 */

/*! \def dg_configUSE_HW_CHARGER
    \brief JEITA charger is not supported in a **DA14701** device variant.

    \warning A compilation error is produced in case the `dg_configUSE_HW_CHARGER` is accidentally enabled.
*/
/*! \def dg_configUSE_HW_USB_CHARGER
    \brief JEITA charger is not supported in a **DA14701** device variant.

    \warning A compilation error is produced in case the `dg_configUSE_HW_USB_CHARGER` is accidentally enabled.
*/
#if (DEVICE_VARIANT == DA14701)
# if (dg_configUSE_HW_CHARGER == 1) || (dg_configUSE_HW_USB_CHARGER == 1)
#  error "JEITA charger is not supported in a DA14701 device variant."
# endif
#endif

/*! \def dg_configUSE_HW_DCACHE
    \brief PSRAM is not supported in a **DA14705** device variant.

    \warning A compilation error is produced in case the `dg_configUSE_HW_DCACHE` is accidentally enabled.
*/

/* eMMC and PSRAM features are only supported in specific DA1470x device variants. */
#if (DEVICE_VARIANT == DA14705)

# if (dg_configUSE_HW_QSPI2 == 1) || (dg_configUSE_HW_DCACHE == 1)
#  error "PSRAM is not supported in a DA14705 device variant."
# endif

# if (dg_configUSE_HW_EMMC == 1)
#  error "eMMC is not supported in a DA14705 device variant."
# endif


#endif /* DEVICE_VARIANT */

#if (DEVICE_VARIANT == DA14706)

# if (dg_configUSE_HW_EMMC == 1)
#  error "eMMC is not supported in a DA14706 device variant."
# endif


#endif /* DEVICE_VARIANT */

/*! \def dg_configUSE_HW_DCACHE

    \warning If the Data Cache controller is enabled on system level then the appropriate data memory interface controller must be enabled too.
*/
#if (dg_configUSE_HW_DCACHE == 1)
#  if (dg_configUSE_HW_QSPI2 == 0)
#   error "It is imperative to enable the QSPIC2 when the Data Cache controller is enabled."
#  endif
#endif

#if (DEVICE_VARIANT == DA14701)
# if (dg_configUSE_HW_PMU == 1)
#  pragma message "Boost DCDC converter is not supported in a DA14701 variant and thus it cannot be set as the VLED power source."
# endif
#endif

/**
 * \}
 */
/* ---------------------------------------------------------------------------------------------- */


#endif /* BSP_DEFAULTS_DA1470X_H_ */
/**
 * \}
 * \}
 */
