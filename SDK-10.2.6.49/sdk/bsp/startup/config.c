/**
 ****************************************************************************************
 *
 * @file config.c
 *
 * @brief System level configurations settings regarding the debug logging mechanism
 * (UART based retargeting (CONFIG_RETARGET), or SWD based via Segger's SystemView tool
 * (dg_configSYSTEMVIEW) or by employing Segger's Real Time Transfer technology (CONFIG_RTT)),
 * protection checks and related error definitions.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdbool.h>
#include <stdio.h>
#include "sdk_defs.h"
#include "interrupts.h"

#ifdef OS_PRESENT
#       include "osal.h"
#endif

/* System debug logging configuration specific header files inclusion */
#ifdef CONFIG_RETARGET
#       include <stddef.h>
#       include "hw_uart.h"
#       include "hw_gpio.h"
#       include "hw_sys.h"
# if  (dg_configUSE_CONSOLE == 1)
#       include "../adapters/src/sys_platform_devices_internal.h"
#       include "console.h"
# endif
#  if dg_configSYS_DBG_LOG_PROTECTION
#       include <stdarg.h>
#       include <string.h>
#       include "sys_bsr.h"
#  endif
/* defined CONFIG_RETARGET */

#elif defined CONFIG_RTT
#       include <stdarg.h>
#       include "SEGGER_RTT.h"
/* defined CONFIG_RTT */

#elif defined CONFIG_NO_PRINT

/* defined CONFIG_NO_PRINT */

#elif dg_configSYSTEMVIEW
#       include <string.h>
#       include <stdarg.h>
#       include "SEGGER_SYSVIEW.h"
#       include "SEGGER_SYSVIEW_ConfDefaults.h"
#endif
/* defined dg_configSYSTEMVIEW */


/* System debug logging configuration specific preprocessor, variables and functions definitions */
#if defined CONFIG_RETARGET

#ifndef CONFIG_RETARGET_UART
        #define CONFIG_RETARGET_UART            SER1_UART

        #define CONFIG_RETARGET_UART_TX_PORT    SER1_TX_PORT
        #define CONFIG_RETARGET_UART_TX_PIN     SER1_TX_PIN
        #define CONFIG_RETARGET_UART_TX_MODE    SER1_TX_MODE
        #define CONFIG_RETARGET_UART_TX_FUNC    SER1_TX_FUNC

        #define CONFIG_RETARGET_UART_RX_PORT    SER1_RX_PORT
        #define CONFIG_RETARGET_UART_RX_PIN     SER1_RX_PIN
        #define CONFIG_RETARGET_UART_RX_MODE    SER1_RX_MODE
        #define CONFIG_RETARGET_UART_RX_FUNC    SER1_RX_FUNC
#endif

#ifndef CONFIG_RETARGET_UART_BAUDRATE
#       define CONFIG_RETARGET_UART_BAUDRATE    HW_UART_BAUDRATE_115200
#endif

#ifndef CONFIG_RETARGET_UART_DATABITS
#       define CONFIG_RETARGET_UART_DATABITS    HW_UART_DATABITS_8
#endif

#ifndef CONFIG_RETARGET_UART_STOPBITS
#       define CONFIG_RETARGET_UART_STOPBITS    HW_UART_STOPBITS_1
#endif

#ifndef CONFIG_RETARGET_UART_PARITY
#       define CONFIG_RETARGET_UART_PARITY      HW_UART_PARITY_NONE
#endif

#define RETARGET_UART_IS_CONFIGURED_FLAG        (0x15)

/* System debug logging protection mechanism definitions */
# if dg_configSYS_DBG_LOG_PROTECTION

static __RETAINED bool single_char_print = false;       /* If true, indicates a single char printf() call. */
static __RETAINED bool retarget_initialized = false;    /* If true, the retarget module (config.c) is initialized. */

static char string[dg_configSYS_DBG_LOG_MAX_SIZE] = "";

#  if (MAIN_PROCESSOR_BUILD)
static __RETAINED OS_MUTEX sys_dbg_log_mutex;           /* Provides mutual exclusion for contenting printf()s */
#  endif
/* In case of multi-processor application deployments prepend the necessary prefixes. */
#  ifdef CONFIG_USE_SNC
#   if (MAIN_PROCESSOR_BUILD)
        /* Prepend an "[M33]: " prefix in all M33 debug logging messages for serial output readability. */
        static const char m33_prefix[] = "[M33]: ";
        #define PREFIX_LEN                      ARRAY_LENGTH(m33_prefix)
#   elif (SNC_PROCESSOR_BUILD)
        /* Prepend an "[SNC]: " prefix in all SNC debug logging messages for serial output readability. */
        static const char snc_prefix[] = "[SNC]: ";
        #define PREFIX_LEN                      ARRAY_LENGTH(snc_prefix)
#   endif
        #define MAX_LEN                         (dg_configSYS_DBG_LOG_MAX_SIZE - PREFIX_LEN)
#  else
        #define MAX_LEN                         (dg_configSYS_DBG_LOG_MAX_SIZE)
#  endif
# endif /* dg_configSYS_DBG_LOG_PROTECTION */

void retarget_init(void)
{
#if dg_configUSE_CONSOLE
        console_init(&sys_platform_console_controller_conf);
#endif /* dg_configUSE_CONSOLE */

# if dg_configSYS_DBG_LOG_PROTECTION
        if (!retarget_initialized) {
#  if (MAIN_PROCESSOR_BUILD)
                OS_MUTEX_CREATE(sys_dbg_log_mutex);
#  endif
                retarget_initialized = true;
        }
# endif
}

#if !dg_configUSE_CONSOLE
static void retarget_reinit(void)
{
        uart_config uart_init = {
                .baud_rate = CONFIG_RETARGET_UART_BAUDRATE,
                .data      = CONFIG_RETARGET_UART_DATABITS,
                .stop      = CONFIG_RETARGET_UART_STOPBITS,
                .parity    = CONFIG_RETARGET_UART_PARITY,
                .use_fifo  = 1,
#if (HW_UART_DMA_SUPPORT == 1)
                .use_dma   = 0,
                .rx_dma_channel = HW_DMA_CHANNEL_INVALID,
                .tx_dma_channel = HW_DMA_CHANNEL_INVALID,
#endif
        };

        hw_uart_init(CONFIG_RETARGET_UART, &uart_init);
        hw_uart_write_scr(CONFIG_RETARGET_UART, RETARGET_UART_IS_CONFIGURED_FLAG);
}

__STATIC_INLINE bool uart_needs_initialization(void)
{
        const uint32_t uart_clk_enables = CRG_SNC->CLK_SNC_REG;
        enum {
                UART_ENABLE = CRG_SNC_CLK_SNC_REG_UART_ENABLE_Msk,
                UART2_ENABLE = CRG_SNC_CLK_SNC_REG_UART2_ENABLE_Msk,
                UART3_ENABLE = CRG_SNC_CLK_SNC_REG_UART3_ENABLE_Msk
        };
        if (CONFIG_RETARGET_UART == HW_UART2) {
                return (!(uart_clk_enables & UART2_ENABLE)
                        || (hw_uart_read_scr(HW_UART2) != RETARGET_UART_IS_CONFIGURED_FLAG));
        } else if (CONFIG_RETARGET_UART == HW_UART3) {
                return (!(uart_clk_enables & UART3_ENABLE)
                        || (hw_uart_read_scr(HW_UART3) != RETARGET_UART_IS_CONFIGURED_FLAG));
        } else {
                return (!(uart_clk_enables & UART_ENABLE)
                        || (hw_uart_read_scr(HW_UART1) != RETARGET_UART_IS_CONFIGURED_FLAG));
        }
        /* Code flow should not reach here */
}

#if !dg_configSYS_DBG_LOG_PROTECTION
__USED
int _write (int fd, char *ptr, int len)
{
        hw_sys_pd_com_enable();
        HW_GPIO_SET_PIN_FUNCTION(CONFIG_RETARGET_UART_TX);
        HW_GPIO_PAD_LATCH_ENABLE(CONFIG_RETARGET_UART_TX);

        /* Enable UART if it's not enabled - can happen after exiting sleep */
        if (uart_needs_initialization()) {
                retarget_reinit();
        }

        /* Write "len" of char from "ptr" to file id "fd"
         * Return number of char written. */
        hw_uart_send(CONFIG_RETARGET_UART, ptr, len, NULL, NULL);

        while (hw_uart_is_busy(CONFIG_RETARGET_UART)) {}
        HW_GPIO_PAD_LATCH_DISABLE(CONFIG_RETARGET_UART_TX);
        hw_sys_pd_com_disable();

        return len;
}
/* Writes (wr) a character (ch) to the console (tty) */
void _ttywrch(int ch)
{
        _write(1 /* STDOUT */, (char*) &ch, 1);
}
#endif /* !dg_configSYS_DBG_LOG_PROTECTION */

__USED
int _read (int fd, char *ptr, int len)
{
        int ret = 0;

        hw_sys_pd_com_enable();
        HW_GPIO_SET_PIN_FUNCTION(CONFIG_RETARGET_UART_RX);
        HW_GPIO_PAD_LATCH_ENABLE(CONFIG_RETARGET_UART_RX);

        if (uart_needs_initialization()) {
                retarget_reinit();
        }

        /*
         * we need to wait for anything to read and return since stdio will assume EOF when we just
         * return 0 from _read()
         */
        while (!hw_uart_is_data_ready(CONFIG_RETARGET_UART)) {

#if defined(OS_PRESENT)
#if !defined(OS_FEATURE_SINGLE_STACK)
                /*
                 * Use some short sleep to give a time for the Idle task to make its work
                 * e.g. freeing memory in OS e.g. deleting task if it is not needed anymore.
                 */
                OS_DELAY(2);
#endif
#endif /* OS_PRESENT */
        }

        /* and now read as much as possible */
        while (hw_uart_is_data_ready(CONFIG_RETARGET_UART) && ret < len) {
                ptr[ret++] = hw_uart_read(CONFIG_RETARGET_UART);
        }

        HW_GPIO_PAD_LATCH_DISABLE(CONFIG_RETARGET_UART_RX);
        hw_sys_pd_com_disable();

        return ret;
}

# if dg_configSYS_DBG_LOG_PROTECTION
/*
 * Overridden libC standard output functions. They are enabled only for M33 OS-based applications
 * that aim to resolve contention issues when more than one M33 tasks or processing units (e.g. M33
 * and SNC) aim to print in parallel. For SNC both OS-based and baremetal configurations are supported.
 *
 * For multi-processor M33-SNC applications the debug logging string originating from each processing
 * unit printing attempt is prefixed with a "[M33]: " and a "[SNC]: " respectively for readability
 * purposes. For simple putchar() calls the prefix is discarded for the same reasons.
 */

/* Ancillary precompiler definitions to accomplish inter-processor mutual exclusion. */
#if (MAIN_PROCESSOR_BUILD)
#       define BSR_MASTER                      SYS_BSR_MASTER_SYSCPU
#elif (SNC_PROCESSOR_BUILD)
#       define BSR_MASTER                      SYS_BSR_MASTER_SNC
#endif

__STATIC_INLINE SYS_BSR_PERIPH_ID get_bsr_id()
{
        if (CONFIG_RETARGET_UART == HW_UART2) {
                return SYS_BSR_PERIPH_ID_UART2;
        } else if (CONFIG_RETARGET_UART == HW_UART3) {
                return SYS_BSR_PERIPH_ID_UART3;
        }
        return SYS_BSR_PERIPH_ID_UART1;
}

#define BSR_RETARGET_GET()      \
        do {                    \
        } while (sys_bsr_try_acquire(BSR_MASTER, get_bsr_id()) == false);

#define BSR_RETARGET_PUT()      sys_bsr_release(BSR_MASTER, get_bsr_id());

/* Ancillary print function that calls UART LLD with the specified string to be output. */
static int vprint(const char *fmt, va_list argp)
{
        int len = 0;

        hw_sys_pd_com_enable();
        HW_GPIO_SET_PIN_FUNCTION(CONFIG_RETARGET_UART_TX);
        HW_GPIO_PAD_LATCH_ENABLE(CONFIG_RETARGET_UART_TX);

        /* Enable UART if it's not enabled - can happen after exiting sleep */
        if (uart_needs_initialization()) {
                retarget_reinit();
        }

        len = strlen(fmt);

        if (len > MAX_LEN) {
                hw_uart_send(CONFIG_RETARGET_UART, "Error in printing more than max chars!\n", 39, NULL, NULL);
        } else {
                /* Step 1 - Print the processing unit prefix */
#ifdef CONFIG_USE_SNC
# if (MAIN_PROCESSOR_BUILD)
                hw_uart_send(CONFIG_RETARGET_UART, m33_prefix, PREFIX_LEN, NULL, NULL);
# elif (SNC_PROCESSOR_BUILD)
                hw_uart_send(CONFIG_RETARGET_UART, snc_prefix, PREFIX_LEN, NULL, NULL);
# endif
#endif
                /* Step 2 - Print the actual debug logging string */
                len = vsnprintf(string, MAX_LEN, fmt, argp);
                if (len > 0) {
                        hw_uart_send(CONFIG_RETARGET_UART, string, len, NULL, NULL);
                } else {
                        hw_uart_send(CONFIG_RETARGET_UART, "Error in composing the debug message!\n", 38, NULL, NULL);
                }
        }
        while (hw_uart_is_busy(CONFIG_RETARGET_UART)) {}

        HW_GPIO_PAD_LATCH_DISABLE(CONFIG_RETARGET_UART_TX);
        hw_sys_pd_com_disable();
        return len;
}
/* Overridden libC printf() with a mutual exclusion mechanism for contenting debug logging activity. */
int printf(const char *__restrict format, ...) __attribute__((format (printf, 1, 2)));
int printf(const char *__restrict format, ...)
{
        int ret;
        va_list param_list;

        /* Enable retarget mechanism if it's not enabled - can happen if it was omitted in application */
        if (!retarget_initialized) {
            retarget_init();
        }

        /* Intra-processor (M33) mutual exclusion mechanism based on mutex use */
#if (MAIN_PROCESSOR_BUILD)
        OS_MUTEX_GET(sys_dbg_log_mutex, OS_MUTEX_FOREVER);
#endif
        /* Inter-processor (M33 and SCN) mutual exclusion mechanism based on BSR use */
#ifdef CONFIG_USE_SNC
        BSR_RETARGET_GET()
#endif

        /* ------ CRITICAL SECTION START ------ */
        va_start(param_list, format);
        ret = vprint(format, param_list);
        va_end(param_list);
        /* ------ CRITICAL SECTION END -------- */

#ifdef CONFIG_USE_SNC
        BSR_RETARGET_PUT()
#endif

#if (MAIN_PROCESSOR_BUILD)
        OS_MUTEX_PUT(sys_dbg_log_mutex);
#endif
        return ret;
}

/*
 * Overridden libC puts() with a mutual exclusion mechanism for contenting debug logging activity.
 * It is used when calling directly puts() in application context, or in case the debug logging string
 * when calling printf() contains one or more newline chars ('\n') in the end but no format specifiers.
 * It is used also by the overridden putchar() when invoked for single character printf() statements.
 *
 * In an application deployment it is recommended to avoid using directly puts() as by design an extra
 * newline char ('\n') is appended in the end of the debug logging string.
 */
int puts(const char *s)
{
        int len = 0;

        /* Enable retarget mechanism if it's not enabled - can happen if it was omitted in application */
        if (!retarget_initialized) {
            retarget_init();
        }

        /* Intra-processor (M33) mutual exclusion mechanism based on mutex use */
#if (MAIN_PROCESSOR_BUILD)
        OS_MUTEX_GET(sys_dbg_log_mutex, OS_MUTEX_FOREVER);
#endif
        /* Inter-processor (M33 and SCN) mutual exclusion mechanism based on BSR use */
#ifdef CONFIG_USE_SNC
        BSR_RETARGET_GET()
#endif

        /* ------ CRITICAL SECTION START ------ */
        hw_sys_pd_com_enable();
        HW_GPIO_SET_PIN_FUNCTION(CONFIG_RETARGET_UART_TX);
        HW_GPIO_PAD_LATCH_ENABLE(CONFIG_RETARGET_UART_TX);

        /* Enable UART if it's not enabled - can happen after exiting sleep */
        if (uart_needs_initialization()) {
            retarget_reinit();
        }

        len = strlen(s);

        /* The puts() libC implementation discards a newline char ('\n') in the end of a string
         * that was attempted to be printed via printf() so we need to send it independently. */
        if (len > 1 || ((len == 1) && !single_char_print)) {
                if (len > MAX_LEN) {
                        hw_uart_send(CONFIG_RETARGET_UART, "Error in printing more than max chars!\n", 39, NULL, NULL);
                } else {
                        /* Step 1 - Print the processing unit prefix */
#ifdef CONFIG_USE_SNC
# if (MAIN_PROCESSOR_BUILD)
                        hw_uart_send(CONFIG_RETARGET_UART, m33_prefix, PREFIX_LEN, NULL, NULL);
# elif (SNC_PROCESSOR_BUILD)
                        hw_uart_send(CONFIG_RETARGET_UART, snc_prefix, PREFIX_LEN, NULL, NULL);
# endif
#endif
                        /* Step 2 - Print the actual debug logging string */
                        hw_uart_send(CONFIG_RETARGET_UART, s, len, NULL, NULL);
                        /* Step 3 - Append a newline char ('\n') in the debug logging string */
                        hw_uart_send(CONFIG_RETARGET_UART, "\n", 1, NULL, NULL);
                }
        }
        /* When invoked from a printf() that aims to send a single character to serial output */
        else {
                hw_uart_send(CONFIG_RETARGET_UART, s, len, NULL, NULL);
        }
        while (hw_uart_is_busy(CONFIG_RETARGET_UART)) {}

        HW_GPIO_PAD_LATCH_DISABLE(CONFIG_RETARGET_UART_TX);
        hw_sys_pd_com_disable();
        /* ------ CRITICAL SECTION END -------- */

#ifdef CONFIG_USE_SNC
        BSR_RETARGET_PUT()
#endif

#if (MAIN_PROCESSOR_BUILD)
        OS_MUTEX_PUT(sys_dbg_log_mutex);
#endif
        return len;
}
/* Overridden libC putchar() with a mutual exclusion mechanism via puts() for contenting debug logging activity. */
int putchar(int a)
{
        int ret;
        char *ptr = (char *)&a;
        single_char_print = true;
        ret = puts(ptr);

        return ret;
}
#  endif /* dg_configSYS_DBG_LOG_PROTECTION */
#endif /* !dg_configUSE_CONSOLE */

/* defined CONFIG_RETARGET */

#elif defined CONFIG_RTT

/*
 * override libC printf()
 */
extern int SEGGER_RTT_vprintf(unsigned BufferIndex, const char * sFormat, va_list * pParamList);
int printf(const char *__restrict format, ...) __attribute__((format (printf, 1, 2)));

int printf(const char *__restrict format, ...)
{
        int ret;
        va_list param_list;

        va_start(param_list, format);
        ret = SEGGER_RTT_vprintf(0, format, &param_list);
        va_end(param_list);
        return ret;
}


/*
 *       _write()
 *
 * Function description
 *   Low-level write function.
 *   libC subroutines will use this system routine for output to all files,
 *   including stdout.
 *   Write data via RTT.
 */
int _write(int file, char *ptr, int len) {
        (void) file;  /* Not used, avoid warning */
        SEGGER_RTT_Write(0, ptr, len);
        return len;
}

int _read(int fd, char *ptr, int len)
{
        int ret = 1;

        /*
         * we need to return at least one character from this call as otherwise stdio functions
         * will assume EOF on file and won't read from it anymore.
         */
        ptr[0] = SEGGER_RTT_WaitKey();

        if (len > 1) {
                ret += SEGGER_RTT_Read(0, ptr + 1, len - 1);
        }

        return ret;
}

int _putc(int a)
{
        char *ptr = (char *)&a;
        int ret;
        ret = SEGGER_RTT_Write(0, ptr, 1);
        return ret;
}

/* defined CONFIG_RTT */

#elif dg_configSYSTEMVIEW

#if ( !defined(SEGGER_RTT_MAX_INTERRUPT_PRIORITY) || (SEGGER_RTT_MAX_INTERRUPT_PRIORITY > configMAX_SYSCALL_INTERRUPT_PRIORITY) )
#pragma GCC error "\r\n\n\t\t\
The SEGGER_RTT_MAX_INTERRUPT_PRIORITY must be defined and be less or equal to configMAX_SYSCALL_INTERRUPT_PRIORITY. \r\n\t\t\
Please set the correct value by defining the dg_configSEGGER_RTT_MAX_INTERRUPT_PRIORITY macro in the application's custom_config_qspi.h file, \r\n\t\t\
and set it to value smaller or equal to the one defined for the configMAX_SYSCALL_INTERRUPT_PRIORITY macro in the FreeRTOSConfig.h \r\n"
#endif

extern void _VPrintHost(const char* s, U32 Options, va_list* pParamList);

int printf(const char *__restrict format, ...) __attribute__((format (printf, 1, 2)));
int printf(const char *__restrict format, ...)
{
        va_list ParamList;
        va_start(ParamList, format);
        _VPrintHost(format, SEGGER_SYSVIEW_LOG, &ParamList);
        va_end(ParamList);
        return 0;
}


/*
 *       _write()
 *
 * Function description
 *   Low-level write function.
 *   libC subroutines will use this system routine for output to all files,
 *   including stdout.
 *   Write data via RTT.
 */
int _write(int file, char *ptr, int len) {
        (void) file;  /* Not used, avoid warning */
        static char send_buf[SEGGER_SYSVIEW_MAX_STRING_LEN - 1];
        int send_len;

        /*
         * Messages bigger than SEGGER_SYSVIEW_MAX_STRING_LEN are not supported by
         * systemview, so only the first SEGGER_SYSVIEW_MAX_STRING_LEN chars will
         * be actually sent to host.
         */
        send_len = (sizeof(send_buf) - 1 > len) ? len : sizeof(send_buf) - 1 ;
        memcpy(send_buf, ptr, send_len);
        send_buf[send_len] = '\0';
        SEGGER_SYSVIEW_Print(send_buf);

        return len;
}

int _read(int fd, char *ptr, int len)
{
        int ret = 1;

        /*
         * we need to return at least one character from this call as otherwise stdio functions
         * will assume EOF on file and won't read from it anymore.
         */
        ptr[0] = 0;
        return ret;
}

/* defined dg_configSYSTEMVIEW */

#elif defined CONFIG_NO_PRINT || (!defined CONFIG_CUSTOM_PRINT && !defined(CONFIG_SEMIHOSTING))

/* CONFIG_NO_PRINT, by default */

/*
 *       _write()
 *
 * Function description
 *   Low-level write function.
 *   libC subroutines will use this system routine for output to all files,
 *   including stdout.
 *   Empty stub.
 */
int _write(int file, char *ptr, int len) {
        return len;
}

int _read(int fd, char *ptr, int len)
{
        int ret = 1;

        /*
         * we need to return at least one character from this call as otherwise stdio functions
         * will assume EOF on file and won't read from it anymore.
         */
        ptr[0] = 0;
        return ret;
}

/*
 * override libC printf()
 *
 * empty stub
 */
int printf(const char *__restrict format, ...) __attribute__((format (printf, 1, 2)));
int printf(const char *__restrict format, ...)
{
        return 0;
}

int puts(const char *s)
{
        return EOF;
}

int _putc(int c)
{
        return EOF;
}

int putchar(int c)
{
        return EOF;
}
#endif


/*
 * System configuration checks
 */
#ifdef PRINT_POWER_RAIL_SETUP

# define PPRS_THEME "\n\n******* 1V8 and 1V8P power rails & Flash operational mode configuration *******\n"



#  define PPRS_1V8P_TITLE "1V8P rail:\n"
#  if (dg_configPOWER_1V8P_ACTIVE == 1)
#   define PPRS_1V8P_ACTIVE "\tactive: on\n"
#  elif (dg_configPOWER_1V8P_ACTIVE == 0)
#   define PPRS_1V8P_ACTIVE "\tactive: off\n"
#  else
#   define PPRS_1V8P_ACTIVE "\tactive: sw defined\n"
#  endif
#  if (dg_configPOWER_1V8P_SLEEP == 1)
#   define PPRS_1V8P_SLEEP "\tsleep: on\n"
#  elif (dg_configPOWER_1V8P_SLEEP == 0)
#   define PPRS_1V8P_SLEEP "\tsleep: off\n"
#  else
#   define PPRS_1V8P_SLEEP "\tsleep: sw defined\n"
#  endif


# if (dg_configFLASH_CONNECTED_TO == FLASH_CONNECTED_TO_1V8P)
#  define PPRS_FLASH_TITLE "Flash is connected to 1V8P"
#  if (dg_configFLASH_POWER_DOWN)
#   define PPRS_FLASH_POWER_DOWN "\nFlash Power Down mode is on\n"
#  else
#   define PPRS_FLASH_POWER_DOWN "\n"
#  endif
#  define PPRS_FLASH_POWER_OFF "\t"

# elif (dg_configFLASH_CONNECTED_TO == FLASH_CONNECTED_TO_1V8)
#  define PPRS_FLASH_TITLE "Flash is connected to 1V8"
#  if (dg_configFLASH_POWER_OFF)
#   define PPRS_FLASH_POWER_OFF "\nFlash Power Off mode is on"
#  else
#   define PPRS_FLASH_POWER_OFF "\t"
#  endif
#  if (dg_configFLASH_POWER_DOWN)
#   define PPRS_FLASH_POWER_DOWN "\nFlash Power Down mode is on\n"
#  else
#   define PPRS_FLASH_POWER_DOWN "\n"
#  endif

# else
#  define PPRS_FLASH_TITLE "A Flash is not connected"
#  define PPRS_FLASH_POWER_OFF "\t"
#  define PPRS_FLASH_POWER_DOWN "\n"
# endif /* dg_configFLASH_CONNECTED_TO */

#  pragma message PPRS_THEME \
                "> " PPRS_1V8P_TITLE PPRS_1V8P_ACTIVE PPRS_1V8P_SLEEP "\n" \
                "> " PPRS_FLASH_TITLE PPRS_FLASH_POWER_OFF PPRS_FLASH_POWER_DOWN

# undef PPRS_THEME
# undef PPRS_1V8_TITLE
# undef PPRS_1V8_ACTIVE
# undef PPRS_1V8_SLEEP
# undef PPRS_1V8P_TITLE
# undef PPRS_1V8P_ACTIVE
# undef PPRS_1V8P_SLEEP
# undef PPRS_FLASH_TITLE
# undef PPRS_FLASH_POWER_OFF
# undef PPRS_FLASH_POWER_DOWN
#endif /* PRINT_POWER_RAIL_SETUP */

#if (dg_configIMAGE_SETUP == PRODUCTION_MODE)
# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
#  if (MAIN_PROCESSOR_BUILD)
#   error "Production mode build: Please define an appropriate code location!"
#  endif
# endif

#else /* dg_configIMAGE_SETUP == DEVELOPMENT_MODE */
# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_OTP)
#  pragma message"Development mode build: code will be built for OTP execution!"
# endif

# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) && \
     (dg_configFLASH_CONNECTED_TO == FLASH_IS_NOT_CONNECTED)
#  error "Building for QSPI Flash code but a Flash is not connected!"
# endif

# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_OQSPI_FLASH) && \
     (dg_configFLASH_CONNECTED_TO == FLASH_IS_NOT_CONNECTED)
#  error "Building for OCTA Flash code but a Flash is not connected!"
# endif




#  if (dg_configFLASH_POWER_DOWN == 1)
#   if defined(dg_configFLASH_POWER_OFF) && (dg_configFLASH_POWER_OFF == 1)
#    error "dg_configFLASH_POWER_DOWN and dg_configFLASH_POWER_OFF cannot be both set to 1"
#   endif
#  endif /* dg_configFLASH_POWER_DOWN */

# if (dg_configUSE_USB == 1) && (dg_configUSE_USB_CHARGER == 0) && (dg_configUSE_USB_ENUMERATION == 0)
#  error "Wrong USB configuration!"
# endif

# if (dg_configLOG_BLE_STACK_MEM_USAGE == 1 ) && (dg_configIMAGE_SETUP != DEVELOPMENT_MODE)
#  error "dg_configLOG_BLE_STACK_MEM_USAGE must not be set when building for PRODUCTION_MODE "
# endif

#endif /* dg_configIMAGE_SETUP */

#if (dg_configUSB_DMA_SUPPORT == 1) && (dg_configUSE_USB_ENUMERATION == 0)
# error "The DMA support aims to assist the USB data transfer, and thus, enabling this feature requires the USB data functionality (enumeratation) to be also enabled."
#endif


#if (dg_configNVPARAM_ADAPTER)
# if (dg_configNVMS_ADAPTER != 1)
#  pragma message "NVMS adapter is mandatory to make use of NVPARAM and will be enabled silently"
#  undef dg_configNVMS_ADAPTER
#  define dg_configNVMS_ADAPTER 1
# endif
#endif

/*
 * Error about not supported DK motherboards
 */
# if (dg_configBLACK_ORCA_MB_REV != BLACK_ORCA_MB_REV_D)
#  error "dg_configBLACK_ORCA_MB_REV is set to a value that is not supported by this SDK."
# endif

/*
 * Check if EXTERNAL_RAM region is outside QSPIC2 controller memory space
 */
# if (dg_configSTORE_VARIABLES_TO_EXTERNAL_RAM == 1)
#  if (dg_configEXTERNAL_RAM_BASE < MEMORY_QSPIC2_BASE) || (MEMORY_QSPIC2_END < (dg_configEXTERNAL_RAM_BASE + dg_configEXTERNAL_RAM_SIZE))
#   error "EXTERNAL_RAM region is outside QSPIC2 controller memory space!"
#  endif
# endif

#if SNC_PROCESSOR_BUILD
/*
 * dg_config options for SNC - verification section
 */

/*
 * System options
 */
#if dg_configUSE_BOD
# error "dg_configUSE_BOD is not applicable for SNC"
#endif

#if dg_configWDOG_NOTIFY_TRIGGER_TMO
# error "dg_configWDOG_NOTIFY_TRIGGER_TMO is not applicable for SNC"
#endif


#if dg_configUSE_CLOCK_MGR
# error "dg_configUSE_CLOCK_MGR is not applicable for SNC"
#endif

#if dg_configUSE_SYS_TRNG
# error "dg_configUSE_SYS_TRNG is not applicable for SNC"
#endif

#if dg_configUSE_SYS_DRBG
# error "dg_configUSE_SYS_DRBG is not applicable for SNC"
#endif

/*
 * Peripheral options
 */
#if dg_configUSE_HW_AES
# error "dg_configUSE_HW_AES is not applicable for SNC"
#endif

#if dg_configUSE_HW_AES_HASH
# error "dg_configUSE_HW_AES_HASH is not applicable for SNC"
#endif

#if dg_configUSE_HW_CACHE
# error "dg_configUSE_HW_CACHE is not applicable for SNC"
#endif

#if dg_configUSE_HW_CHARGER
# error "dg_configUSE_HW_CHARGER is not applicable for SNC."
#endif

#if dg_configUSE_HW_CPM
# error "dg_configUSE_HW_CPM"
#endif

#if dg_configUSE_HW_DCACHE
# error "dg_configUSE_HW_DCACHE is not applicable for SNC"
#endif

#if dg_configUSE_HW_DMA
# error "dg_configUSE_HW_DMA is not applicable for SNC"
#endif

#if dg_configUSE_HW_EMMC
# error "dg_configUSE_HW_EMMC is not applicable for SNC"
#endif

#if dg_configUSE_HW_HASH
# error "dg_configUSE_HW_HASH is not applicable for SNC"
#endif

#if dg_configUSE_HW_LCDC
# error "dg_configUSE_HW_LCDC is not applicable for SNC"
#endif

#if dg_configUSE_HW_MPU
# error "dg_configUSE_HW_MPU is not applicable for SNC"
#endif

#if dg_configUSE_HW_OQSPI
# error "dg_configUSE_HW_OQSPI is not applicable for SNC"
#endif


#if dg_configUSE_HW_OTPC
# error "dg_configUSE_HW_OTPC is not applicable for SNC"
#endif

#if dg_configUSE_HW_PMU
# error "dg_configUSE_HW_PMU is not applicable for SNC"
#endif

#if dg_configUSE_HW_QSPI
# error "dg_configUSE_HW_QSPI is not applicable for SNC"
#endif

#if dg_configUSE_HW_QSPI2
# error "dg_configUSE_HW_QSPI2 is not applicable for SNC"
#endif

#if dg_configUSE_HW_SDADC
# error "dg_configUSE_HW_SDADC is not applicable for SNC"
#endif


#if dg_configUSE_HW_USB
# error "dg_configUSE_HW_USB is not applicable for SNC"
#endif

#if dg_configUSE_HW_USB_CHARGER
# error "dg_configUSE_HW_USB_CHARGER is not applicable for SNC"
#endif

#if dg_configUSE_HW_PORT_DETECTION
# error "dg_configUSE_HW_PORT_DETECTION"
#endif

#if dg_configGPADC_DMA_SUPPORT || dg_configSPI_DMA_SUPPORT || dg_configI2C_DMA_SUPPORT  \
        || dg_configSDADC_DMA_SUPPORT || dg_configUART_DMA_SUPPORT || dg_configUSB_DMA_SUPPORT
# error "DMA support is not available for SNC."
#endif

#endif /* SNC_PROCESSOR_BUILD */

