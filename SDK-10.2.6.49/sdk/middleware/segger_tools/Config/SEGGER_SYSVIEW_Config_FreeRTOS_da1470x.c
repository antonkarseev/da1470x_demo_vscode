/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 1995 - 2021 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER SystemView * Real-time application analysis           *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* SEGGER strongly recommends to not make any changes                 *
* to or modify the source code of this software in order to stay     *
* compatible with the SystemView and RTT protocol, and J-Link.       *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* condition is met:                                                  *
*                                                                    *
* o Redistributions of source code must retain the above copyright   *
*   notice, this condition and the following disclaimer.             *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************
*                                                                    *
*       SystemView version: 3.30                                    *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : SEGGER_SYSVIEW_Config_FreeRTOS.c
Purpose : Sample setup configuration of SystemView with FreeRTOS.
Revision: $Rev: 7745 $
*/

#if (dg_configSYSTEMVIEW == 1)
#if (DEVICE_FAMILY == DA1470X)
#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_SYSVIEW.h"
#include "sys_timer.h"
#include "interrupts.h"
#include "SEGGER_RTT.h"
extern const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI;

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
// The application name to be displayed in SystemViewer
#define SYSVIEW_APP_NAME        "DemoApp"

// The target device name
#define SYSVIEW_DEVICE_NAME     "DA1470x"

// Frequency of the timestamp. Must match SEGGER_SYSVIEW_GET_TIMESTAMP in SEGGER_SYSVIEW_Conf.h
#define SYSVIEW_TIMESTAMP_FREQ  (configSYSTICK_CLOCK_HZ)

// System Frequency. SystemcoreClock is used in most CMSIS compatible projects.
#define SYSVIEW_CPU_FREQ        configCPU_CLOCK_HZ

// The lowest RAM address used for IDs (pointers)
#define SYSVIEW_RAM_BASE        (MEMORY_SYSRAM_BASE)
/*********************************************************************
*
*       _cbSendSystemDesc()
*
*  Function description
*    Sends SystemView description strings.
*/
static void _cbSendSystemDesc(void)
{
        /*
           * The maximum size of the string passed as argument to SEGGER_SYSVIEW_SendSysDesc()
           * should not exceed SEGGER_SYSVIEW_MAX_STRING_LEN (128) bytes. Values can be comma
           * seperated.
           *
           * More ISR entries could be added but this would result in a slower system and might
           * also affect time critical tasks or trigger assertions.
           *
           * This is because multiple SEGGER_SYSVIEW_SendSysDesc() calls will result in multiple
           * RTT transactions.
           *
           * Note also that _cbSendSystemDesc() is called multiple times from the host PC and not
           * just during initialization, so assertions may occur anytime during SystemView monitoring.
           *
           */
        const char* sys_desc =
                "N="SYSVIEW_APP_NAME",D="SYSVIEW_DEVICE_NAME",O=FreeRTOS,"
                "I#15=SysTick,"
                "I#16=CMAC2SYS,"
                //"I#17=Sensor_Node,"
                //"I#18=MRM,"
                //"I#19=PDC,"
                //"I#20=Key_Wkup_GPIO,"
                //"I#21=VBUS,"
                //"I#22=Charger_State,"
                //"I#23=Charger_Error,"
                //"I#24=DCDC,"
                //"I#25=PLL48_Lock,"
                //"I#26=Crypto,"
                //"I#27=PLL_Lock,"
                "I#28=XTAL32M_Ready,"
                //"I#29=RFDIAG,"
                //"I#30=GPIO_P0,"
                //"I#31=GPIO_P1,"
                //"I#32=GPIO_P2,"
                //"I#33=Timer,"
                "I#34=Timer2,"
                //"I#35=Timer3,"
                //"I#36=Timer4,"
                //"I#37=Timer5,"
                //"I#38=Timer6,"
                //"I#39=RTC,"
                //"I#40=RTC_Event,"
                //"I#41=CAPTIMER1,"
                //"I#42=ADC,"
                //"I#43=ADC2,"
                "I#44=DMA,"
                //"I#45=UART,"
                //"I#46=UART2,"
                //"I#47=UART3,"
                //"I#48=SPI,"
                //"I#49=SPI2,"
                //"I#50=SPI3,"
                //"I#51=I2C,"
                //"I#52=I2C2,"
                //"I#53=I2C3,"
                //"I#54=I3C,"
                //"I#55=USB,"
                //"I#56=PCM,"
                //"I#57=SRC_In,"
                //"I#58=SRC_Out,"
                //"I#59=SRC2_In,"
                //"I#60=SRC2_Out,"
                //"I#61=VAD,"
                //"I#62=EMMC,"
#if dg_configUNDISCLOSED_API_DEPREC_CONFIG
                //"I#63=SDIO,"
#else
                //"I#63=RSVD47,"
#endif /* dg_configUNDISCLOSED_API_DEPREC_CONFIG */
                //"I#64=GPU,"
                //"I#65=LCD_Controller,"
#if dg_configUNDISCLOSED_API_DEPREC_CONFIG
                //"I#66=DSI,"
#else
                //"I#66=RSVD50,"
#endif /* dg_configUNDISCLOSED_API_DEPREC_CONFIG */
                //"I#67=Charger_Det,"
                //"I#68=DCACHE_MRM,"
                //"I#69=CLK_CALIBRATION,"
                //"I#70=VSYS_GEN,"
                "I#71=RSVD55";

          SEGGER_SYSVIEW_SendSysDesc(sys_desc);
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
void SEGGER_SYSVIEW_Conf(void)
{
  SEGGER_SYSVIEW_Init(SYSVIEW_TIMESTAMP_FREQ, SYSVIEW_CPU_FREQ,
                      &SYSVIEW_X_OS_TraceAPI, _cbSendSystemDesc);
  SEGGER_SYSVIEW_SetRAMBase(SYSVIEW_RAM_BASE);
}

uint64_t sys_timer_get_timestamp_fromCPM(uint32_t* timer_value);

__RETAINED_CODE
U32 SEGGER_SYSVIEW_X_GetTimestamp()
{
        uint32_t timestamp;
        uint32_t timer_value;

        SEGGER_RTT_LOCK();
        timestamp = (uint32_t) sys_timer_get_timestamp_fromCPM(&timer_value);
        SEGGER_RTT_UNLOCK();

        return timestamp;
}

__RETAINED_CODE
U32 SEGGER_SYSVIEW_X_GetInterruptId(void)
{
        return ((*(U32 *)(0xE000ED04)) & 0x1FF);
}
#endif /* DEVICE_FAMILY */
#endif /* (dg_configSYSTEMVIEW == 1) */

/*************************** End of file ****************************/
