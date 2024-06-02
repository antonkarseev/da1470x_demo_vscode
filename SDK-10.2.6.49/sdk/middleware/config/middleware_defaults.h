/**
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup MIDDLEWARE_CONFIG_DEFAULTS
 *
 * \brief Middleware default configuration values
 *
 * The following tags are used to describe the type of each configuration option.
 *
 * - **\bsp_config_option_build**        : To be changed only in the build configuration
 *                                                of the project ("Defined symbols -D" in the
 *                                                preprocessor options).
 *
 * - **\bsp_config_option_app**          : To be changed only in the custom_config*.h
 *                                                project files.
 *
 * - **\bsp_config_option_expert_only**  : To be changed only by an expert user.
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file middleware_defaults.h
 *
 * @brief Middleware. System Configuration file default values.
 *
 * Copyright (C) 2018-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef MIDDLEWARE_DEFAULTS_H_
#define MIDDLEWARE_DEFAULTS_H_

/**
 * \addtogroup ADAPTER_SELECTION Adapters enabled by default
 *
 * \brief Adapter selection
 *
 * When enabled the specific adapter is included in the compilation of the SDK.
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * The default option can be overridden in the application configuration file.
 *
 * \{
   Adapter                        | Setting                                | Default option
   ------------------------------ | -------------------------------------- | :------------------:
   Table not yet fixed | dg_configXXXXX_ADAPTER                   | 1
 *
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */


/* -------------------------------- Adapters (ad_*) selection -------------------------------- */

#ifndef dg_configFLASH_ADAPTER
#define dg_configFLASH_ADAPTER                  (1)
#endif

#ifndef dg_configI2C_ADAPTER
#define dg_configI2C_ADAPTER                    (0)
#endif

#ifndef dg_configI3C_ADAPTER
#define dg_configI3C_ADAPTER                    (0)
#endif

#ifndef dg_configNVMS_ADAPTER
#define dg_configNVMS_ADAPTER                   (1)
#endif

#ifndef dg_configNVMS_FLASH_CACHE
#define dg_configNVMS_FLASH_CACHE               (0)
#endif

#ifndef dg_configNVMS_VES
#define dg_configNVMS_VES                       (1)
#endif

#ifndef dg_configSPI_ADAPTER
#define dg_configSPI_ADAPTER                    (0)
#endif

/* UART adapter is deactivated by default unless console service is enabled */
#ifndef dg_configUART_ADAPTER
# if (dg_configUSE_CONSOLE == 1)
#  define dg_configUART_ADAPTER                 (1)
# else
#  define dg_configUART_ADAPTER                 (0)
# endif
#endif

#ifndef dg_configGPADC_ADAPTER
#define dg_configGPADC_ADAPTER                  (0)
#endif

#ifdef dg_configSDADC_ADAPTER
#warning "Obsolete configuration. No SDADC adapter applicable"
#endif

#ifdef dg_configTEMPSENS_ADAPTER
#error "Configuration option dg_configTEMPSENS_ADAPTER  is no longer supported"
#endif

#ifdef dg_configBATTERY_ADAPTER
#error "Configuration option dg_configBATTERY_ADAPTER  is no longer supported"
#endif

#ifndef dg_configNVPARAM_ADAPTER
#define dg_configNVPARAM_ADAPTER                (0)
#endif

#ifndef dg_configNVPARAM_APP_AREA
#define dg_configNVPARAM_APP_AREA               (0)
#endif

#ifndef dg_configCRYPTO_ADAPTER
#define dg_configCRYPTO_ADAPTER                 (1)
#endif

#ifndef dg_configKEYBOARD_SCANNER_ADAPTER
#define dg_configKEYBOARD_SCANNER_ADAPTER       (0)
#endif


#if (MAIN_PROCESSOR_BUILD)
#ifndef dg_configPMU_ADAPTER
#define dg_configPMU_ADAPTER                    (1)
#endif
#elif (SNC_PROCESSOR_BUILD)
#undef dg_configPMU_ADAPTER
#define dg_configPMU_ADAPTER                    (0)
#endif

#if (MAIN_PROCESSOR_BUILD)
#if defined(OS_PRESENT)
#ifndef dg_configENABLE_RCHS_CALIBRATION
#define dg_configENABLE_RCHS_CALIBRATION       (1)
#endif
#elif defined(OS_BAREMETAL)
#if dg_configENABLE_RCHS_CALIBRATION
# error "RCHS calibration cannot be enabled in baremetal projects"
#endif
#endif
#endif /* MAIN_PROCESSOR_BUILD */


/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 */


/**
 * \addtogroup CONSOLE_IO_SETTINGS Console I/O Settings
 *
 * \brief Console IO configuration settings
 *
 * \{
   Description                               | Setting                    | Default option
   ----------------------------------------- | -------------------------- | :---------------:
   Enable serial console service module      | dg_configUSE_CONSOLE       | 0
   Enable serial console stubbed API         | dg_configUSE_CONSOLE_STUBS | 0
   Enable Command Line Interface module      | dg_configUSE_CLI           | 0
   Enable Command Line Interface stubbed API | dg_configUSE_CLI_STUBS     | 0

   \see console.h cli.h

   \note CLI module requires dg_configUSE_CONSOLE to be enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */

/* -------------------------------------- Console IO configuration settings --------------------- */

#ifndef dg_configUSE_CONSOLE
#define dg_configUSE_CONSOLE                    (0)
#endif

#ifndef dg_configUSE_CONSOLE_STUBS
#define dg_configUSE_CONSOLE_STUBS              (0)
#endif

#ifndef dg_configUSE_CLI
#define dg_configUSE_CLI                        (0)
#endif

#ifndef dg_configUSE_CLI_STUBS
#define dg_configUSE_CLI_STUBS                  (0)
#endif
/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 */

/* ----------------------------- DGTL ----------------------------------------------------------- */

/**
 * \brief Enable D.GTL interface
 *
 * When this macro is enabled, the DGTL framework is available for use.
 * The framework must furthermore be initialized in the application using
 * dgtl_init(). Additionally, the UART adapter must be initialized accordingly.
 *
 * Please see sdk/middleware/dgtl/include/ for further DGTL configuration
 * (in dgtl_config.h) and API.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 *
 */
#ifndef dg_configUSE_DGTL
#define dg_configUSE_DGTL                       (0)
#endif

/**
 * \addtogroup MIDDLEWARE_DEBUG_SETTINGS Debug Settings
 *
 * \{
 */
/* -------------------------------------- Debug settings ---------------------------------------- */

/**
 * \def dg_configENABLE_TASK_MONITORING
 *
 * \brief Enable task monitoring.
 *
 * \note Task monitoring can only be enabled if RTT or RETARGET is enabled
 * \bsp_default_note{\bsp_config_option_app,}
 */
#if !defined(dg_configENABLE_TASK_MONITORING) || defined(RUNNING_DOXYGEN)
#define dg_configENABLE_TASK_MONITORING         (0)
#endif

/**
 * \def dg_configENABLE_MTB
 *
 * \brief Enable Micro Trace Buffer
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */

/**
 * \def dg_configENABLE_MTB
 *
 * \note MTB is available on all 3 cores
 */
#ifndef dg_configENABLE_MTB
# if (MAIN_PROCESSOR_BUILD)
#   define dg_configENABLE_MTB                  (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
# else
#   define dg_configENABLE_MTB                  (0)
# endif
#endif



/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------- OS related configuration ---------------------------------- */

/**
 * \def dg_configTRACK_OS_HEAP
 *
 * \brief Monitor OS heap allocations
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configTRACK_OS_HEAP
#define dg_configTRACK_OS_HEAP                  (0)
#endif

/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 */

/* ---------------------------------- SYSTEM CONFIGURATION ------------------------------------ */
/**
 * \brief Enable gpadc monitoring.
 *
 * \note The application must not explicitly set dg_configUSE_SYS_ADC to 1.\n
 *       Use instead dg_configRF_ENABLE_RECALIBRATION\n
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SYS_ADC
#define dg_configUSE_SYS_ADC                    (0)
#endif

/**
 * \brief      When set to 1, the audio manager is enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SYS_AUDIO_MGR
#define dg_configUSE_SYS_AUDIO_MGR              (0)
#endif

/**
 * \brief Enable System Boot handler
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SYS_BOOT
# if defined(OS_PRESENT) && MAIN_PROCESSOR_BUILD && (dg_configCODE_LOCATION != NON_VOLATILE_IS_NONE)
#  define dg_configUSE_SYS_BOOT                 (1)
# else
#  define dg_configUSE_SYS_BOOT                 (0)
# endif
#endif

#if defined(OS_PRESENT) && MAIN_PROCESSOR_BUILD && \
    (dg_configCODE_LOCATION != NON_VOLATILE_IS_NONE) && (dg_configUSE_SYS_BOOT == 0)
#error "It is mandatory to enable the system boot service (sys_boot)"
#endif

/**
 * \brief When set to 1, USB enumeration is enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_USB_ENUMERATION
#define dg_configUSE_USB_ENUMERATION            (0)
#endif

/**
 * \brief When set to 1, the sys charger service is used to charge the battery.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SYS_CHARGER
#define dg_configUSE_SYS_CHARGER                (0)
#endif

/**
 * \brief When set to 1, Charger oscillation detection is enabled
 *
 * The charger may get into an oscillation between the CC and pre-charge modes, when there is
 * a high resistance in the path between the charger pins and the battery. This is caused
 * by a low VBUS-VBAT headroom.
 *
 * If oscillation detection is enabled, the charger service will monitor charger activity,
 * to detect a "high" number of transitions in a relatively small period of time. If the
 * above threshold is exceeded, charging will stop, to avoid flooding of application code
 * with charger transition events, and the application will be notified about the event.
 *
 * The application will decide how to proceed.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configSYS_CHARGER_OSC_CHECK_EN
#define dg_configSYS_CHARGER_OSC_CHECK_EN                       (1)
#endif

/**
 * \brief Oscillation check time interval
 *
 * The length of the observation window that the charger will try to
 * detect a possible oscillation, given in milliseconds.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configSYS_CHARGER_OSC_CHECK_TIMER_INTERVAL_MS
#define dg_configSYS_CHARGER_OSC_CHECK_TIMER_INTERVAL_MS        (10)
#endif

/**
 * \brief Oscillation threshold
 *
 * The number of interrupts that need to be raised inside an oscillation
 * observation window, to trigger charger oscillation detection.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */

#ifndef dg_configSYS_CHARGER_VBUS_IRQ_CNT_THRESH
#define dg_configSYS_CHARGER_VBUS_IRQ_CNT_THRESH                (40)
#endif

/**
 * \brief When set to 1, the sys usb service is used to manage:
 *        - VBUS attach / detach and  USB suspend / resume operations.
 *        - Notifications towards SDK and applications.
 *        - Suspend / resume sleep.
 *        - Suspend / resume DC/DC if in use.
 *
 * \note The service is automatically enabled when charging or USB enumeration
 *       are involved (see dg_configUSE_SYS_CHARGER , dg_configUSE_USB_ENUMERATION).
 *       It's recommended to be enabled when:
 *       - The power supply is a non rechargeable battery and the application is not
 *         interested in USB enumeration.
 *       - The only source of power supply is VBUS e.g a USB dongle.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configUSE_SYS_USB
#define dg_configUSE_SYS_USB                    (0)
#endif

/**
 * \brief When set to 1, the mailbox module is enabled. When it is enabled, the mailbox handler
 *        is registered to the SNC2SYS/SYS2SNC hardware interrupt handler and dispatches
 *        the notifications which are triggered through the SNC2SYS/SYS2SNC hardware interrupt
 *        events.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_MAILBOX
#define dg_configUSE_MAILBOX                    (0)
#endif

/**
 * \brief When set to 1, the RPMsg-Lite inter-processor (Main to SNC processor) communication
 *        framework is enabled. The framework requires the mailbox module to be enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_RPMSG_LITE
#define dg_configUSE_RPMSG_LITE                 (0)
#endif
#if (dg_configUSE_RPMSG_LITE == 1)
#undef dg_configUSE_MAILBOX
#define dg_configUSE_MAILBOX                    (1)
#endif

/**
 * \brief Number of shared space handles defined by the application
 *        between M33 and SNC processors.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configSNC_SHARED_SPACE_APP_HANDLES
#define dg_configSNC_SHARED_SPACE_APP_HANDLES   (0)
#endif

/* ---------------------------------------------------------------------------------------------- */

/* ----------------------------------- Driver dependencies -------------------------------------- */


/*
 * SYS_CHARGER will also enable SYS_USB which implements the USB and VBUS interrupt handlers
 */
#if (dg_configUSE_SYS_CHARGER == 1)
#undef dg_configUSE_HW_USB_CHARGER
#define dg_configUSE_HW_USB_CHARGER             (1)
#undef dg_configUSE_HW_CHARGER
#define dg_configUSE_HW_CHARGER                 (1)
#undef dg_configUSE_HW_USB
#define dg_configUSE_HW_USB                     (1)
#undef dg_configUSE_SYS_USB
#define dg_configUSE_SYS_USB                    (1)
#endif

/*
 * For USB_ENUMERATION to work, SYS_USB is also enabled, and library usb_lib needs to be linked
 * with the active project
 */
#if (dg_configUSE_USB_ENUMERATION == 1)
  #undef dg_configUSE_SYS_USB
  #define dg_configUSE_SYS_USB                  (1)
#endif

/*
 * SYS_USB can be explicitly enabled, even if dg_configUSE_SYS_CHARGER and dg_configUSE_USB_ENUMERATION
 * are not used. This will enable the VBUS and USB interrupt handlers, but no functionality will be
 * attached to them
 */
#if (dg_configUSE_SYS_USB == 1)
#undef dg_configUSE_HW_USB
#define dg_configUSE_HW_USB                     (1)
#endif

/* If RF or RCHS recalibration is enabled, we need to enable GPADC as well */
#if (MAIN_PROCESSOR_BUILD)
#if (dg_configRF_ENABLE_RECALIBRATION || (dg_configENABLE_RCHS_CALIBRATION && defined(OS_FREERTOS)))
#undef dg_configUSE_SYS_ADC
#define dg_configUSE_SYS_ADC                 	(1)
# undef dg_configUSE_HW_GPADC
# define dg_configUSE_HW_GPADC                  (1)
# undef dg_configGPADC_ADAPTER
# define dg_configGPADC_ADAPTER                 (1)
# endif
#endif /* MAIN_PROCESSOR_BUILD */


/*
 * \brief Enable the RTC correction mechanism
 *
 * When RCX is set as the low power clock and Real Time Clock is used (i.e. dg_configUSE_HW_RTC is defined),
 * setting this macro to 1 enables the RTC correction mechanism.
 */
#if (dg_configUSE_LP_CLK == LP_CLK_RCX && defined(OS_PRESENT) && dg_configUSE_HW_RTC)
#ifndef dg_configRTC_CORRECTION
#define dg_configRTC_CORRECTION (1)
#endif
#else
#if dg_configRTC_CORRECTION
# pragma message "dg_configRTC_CORRECTION is only used in RTOS based projects when RCX is set as low power clock. Forcing to 0."
#undef dg_configRTC_CORRECTION
#define dg_configRTC_CORRECTION (0)
#endif
#endif

/*
 * If SYS_TRNG is enabled, it is also necessary to enable the AES/HASH driver because
 * sys_trng makes use of aes ecb encryption functionality
 */
#if (dg_configUSE_SYS_TRNG == 1)
#undef  dg_configUSE_HW_AES
#define dg_configUSE_HW_AES                     (1)
#endif /* dg_configUSE_SYS_TRNG */
/* ---------------------------------------------------------------------------------------------- */

#endif /* MIDDLEWARE_DEFAULTS_H_ */

/**
\}
\}
*/
