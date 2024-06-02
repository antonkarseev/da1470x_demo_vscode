/***********************************************************************
*                    SEGGER Microcontroller GmbH                       *
*                        The Embedded Experts                          *
************************************************************************
*                                                                      *
*                  (c) SEGGER Microcontroller GmbH                     *
*                        All rights reserved                           *
*                          www.segger.com                              *
*                                                                      *
************************************************************************
*                                                                      *
************************************************************************
*                                                                      *
*                                                                      *
*  Licensing terms                                                     *
*                                                                      *
* Redistribution and use in source and binary forms, with or without   *
* modification, are permitted provided that the following conditions   *
* are met:                                                             *
*                                                                      *
* 1. Redistributions of source code must retain the above copyright    *
* notice, this list of conditions and the following disclaimer.        *
*                                                                      *
* 2. Redistributions in binary form must reproduce the above           *
* copyright notice, this list of conditions and the following          *
* disclaimer in the documentation and/or other materials provided      *
* with the distribution.                                               *
*                                                                      *
*                                                                      *
* THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDER "AS IS" AND ANY        *
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE    *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR   *
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE        *
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,     *
* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,             *
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR   *
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY  *
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT         *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE    *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH.    *
* DAMAGE.                                                              *
*                                                                      *
************************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : FlashPrg.c
Purpose : Implementation of RAMCode template
*/

/* Copyright (c) 2022 Modified by Dialog Semiconductor */

#include <stddef.h>
#include "sdk_defs.h"
#include "FlashOS.h"
#include "hw_watchdog.h"
#include "hw_qspi.h"
#include "hw_pd.h"

#       if defined(FLASH_DEV_OQFLASH)
#include "oqspi_automode.h"
#       elif defined(FLASH_DEV_QFLASH)
#include "qspi_automode.h"
#       endif

/*********************************************************************
*
*       Defines (configurable)
*
**********************************************************************
*/
#define SEGGER_DBG_SET_HIGH(flag, name)                                          \
{                                                                                \
        if (flag == 1) {                                                         \
                name##_MODE_REG = 0x300;                                         \
                DBG_SET_PIN_REG(name);                                           \
                DBG_TOGGLE_PIN_PAD_LATCH(name);                                  \
        }                                                                        \
}

#define SEGGER_DBG_SET_LOW(flag, name)                                           \
{                                                                                \
        if (flag == 1) {                                                         \
                name##_MODE_REG = 0x300;                                         \
                DBG_RESET_PIN_REG(name);                                         \
                DBG_TOGGLE_PIN_PAD_LATCH(name);                                  \
        }                                                                        \
}


#if (SEGGER_FLASH_LOADER_DEBUG == 0)
// Debug Init() (initial configuration: low)
#define SEGGERDBG_INIT_MODE_REG                 *(volatile int *)0x20000000
#define SEGGERDBG_INIT_SET_REG                  *(volatile int *)0x20000000
#define SEGGERDBG_INIT_RESET_REG                *(volatile int *)0x20000000
#define SEGGERDBG_INIT_PIN                      (0)

// Debug Uninit() (initial configuration: low)
#define SEGGERDBG_UNINIT_MODE_REG               *(volatile int *)0x20000000
#define SEGGERDBG_UNINIT_SET_REG                *(volatile int *)0x20000000
#define SEGGERDBG_UNINIT_RESET_REG              *(volatile int *)0x20000000
#define SEGGERDBG_UNINIT_PIN                    (0)

// Debug EraseSector() (initial configuration: low)
#define SEGGERDBG_ERASE_SECTOR_MODE_REG         *(volatile int *)0x20000000
#define SEGGERDBG_ERASE_SECTOR_SET_REG          *(volatile int *)0x20000000
#define SEGGERDBG_ERASE_SECTOR_RESET_REG        *(volatile int *)0x20000000
#define SEGGERDBG_ERASE_SECTOR_PIN              (0)

// Debug ProgramPage() (initial configuration: low)
#define SEGGERDBG_PAGE_PROGRAM_MODE_REG         *(volatile int *)0x20000000
#define SEGGERDBG_PAGE_PROGRAM_SET_REG          *(volatile int *)0x20000000
#define SEGGERDBG_PAGE_PROGRAM_RESET_REG        *(volatile int *)0x20000000
#define SEGGERDBG_PAGE_PROGRAM_PIN              (0)

// Debug SEGGER_OPEN_CalcCRC() (initial configuration: low)
#define SEGGERDBG_OPEN_CALC_CRC_MODE_REG        *(volatile int *)0x20000000
#define SEGGERDBG_OPEN_CALC_CRC_SET_REG         *(volatile int *)0x20000000
#define SEGGERDBG_OPEN_CALC_CRC_RESET_REG       *(volatile int *)0x20000000
#define SEGGERDBG_OPEN_CALC_CRC_PIN             (0)

// Debug SEGGER_OPEN_Program() (initial configuration: low)
#define SEGGERDBG_OPEN_PROGRAM_MODE_REG         *(volatile int *)0x20000000
#define SEGGERDBG_OPEN_PROGRAM_SET_REG          *(volatile int *)0x20000000
#define SEGGERDBG_OPEN_PROGRAM_RESET_REG        *(volatile int *)0x20000000
#define SEGGERDBG_OPEN_PROGRAM_PIN              (0)

// Debug SEGGER_OPEN_Erase() (initial configuration: low)
#define SEGGERDBG_OPEN_ERASE_MODE_REG           *(volatile int *)0x20000000
#define SEGGERDBG_OPEN_ERASE_SET_REG            *(volatile int *)0x20000000
#define SEGGERDBG_OPEN_ERASE_RESET_REG          *(volatile int *)0x20000000
#define SEGGERDBG_OPEN_ERASE_PIN                (0)

// Debug SEGGER_OPEN_Start() (initial configuration: low)
#define SEGGERDBG_OPEN_START_MODE_REG          *(volatile int *)0x20000000
#define SEGGERDBG_OPEN_START_SET_REG           *(volatile int *)0x20000000
#define SEGGERDBG_OPEN_START_RESET_REG         *(volatile int *)0x20000000
#define SEGGERDBG_OPEN_START_PIN               (0)

#else /* SEGGER_FLASH_LOADER_DEBUG */
// Debug Init() (initial configuration: low)
#define SEGGERDBG_INIT_MODE_REG                 GPIO->P0_18_MODE_REG
#define SEGGERDBG_INIT_SET_REG                  GPIO->P0_SET_DATA_REG
#define SEGGERDBG_INIT_RESET_REG                GPIO->P0_RESET_DATA_REG
#define SEGGERDBG_INIT_PIN                      (1 << 18)

// Debug Uninit() (initial configuration: low)
#define SEGGERDBG_UNINIT_MODE_REG               GPIO->P0_19_MODE_REG
#define SEGGERDBG_UNINIT_SET_REG                GPIO->P0_SET_DATA_REG
#define SEGGERDBG_UNINIT_RESET_REG              GPIO->P0_RESET_DATA_REG
#define SEGGERDBG_UNINIT_PIN                    (1 << 19)

// Debug EraseSector() (initial configuration: low)
#define SEGGERDBG_ERASE_SECTOR_MODE_REG         GPIO->P0_20_MODE_REG
#define SEGGERDBG_ERASE_SECTOR_SET_REG          GPIO->P0_SET_DATA_REG
#define SEGGERDBG_ERASE_SECTOR_RESET_REG        GPIO->P0_RESET_DATA_REG
#define SEGGERDBG_ERASE_SECTOR_PIN              (1 << 20)

// Debug ProgramPage() (initial configuration: low)
#define SEGGERDBG_PAGE_PROGRAM_MODE_REG         GPIO->P0_21_MODE_REG
#define SEGGERDBG_PAGE_PROGRAM_SET_REG          GPIO->P0_SET_DATA_REG
#define SEGGERDBG_PAGE_PROGRAM_RESET_REG        GPIO->P0_RESET_DATA_REG
#define SEGGERDBG_PAGE_PROGRAM_PIN              (1 << 21)

// Debug SEGGER_OPEN_CalcCRC() (initial configuration: low)
#define SEGGERDBG_OPEN_CALC_CRC_MODE_REG        GPIO->P0_26_MODE_REG
#define SEGGERDBG_OPEN_CALC_CRC_SET_REG         GPIO->P0_SET_DATA_REG
#define SEGGERDBG_OPEN_CALC_CRC_RESET_REG       GPIO->P0_RESET_DATA_REG
#define SEGGERDBG_OPEN_CALC_CRC_PIN             (1 << 26)

// Debug SEGGER_OPEN_Program() (initial configuration: low)
#define SEGGERDBG_OPEN_PROGRAM_MODE_REG         GPIO->P0_27_MODE_REG
#define SEGGERDBG_OPEN_PROGRAM_SET_REG          GPIO->P0_SET_DATA_REG
#define SEGGERDBG_OPEN_PROGRAM_RESET_REG        GPIO->P0_RESET_DATA_REG
#define SEGGERDBG_OPEN_PROGRAM_PIN              (1 << 27)

// Debug SEGGER_OPEN_Erase() (initial configuration: low)
#define SEGGERDBG_OPEN_ERASE_MODE_REG           GPIO->P0_28_MODE_REG
#define SEGGERDBG_OPEN_ERASE_SET_REG            GPIO->P0_SET_DATA_REG
#define SEGGERDBG_OPEN_ERASE_RESET_REG          GPIO->P0_RESET_DATA_REG
#define SEGGERDBG_OPEN_ERASE_PIN                (1 << 28)

// Debug SEGGER_OPEN_Start() (initial configuration: low)
#define SEGGERDBG_OPEN_START_MODE_REG          GPIO->P0_29_MODE_REG
#define SEGGERDBG_OPEN_START_SET_REG           GPIO->P0_SET_DATA_REG
#define SEGGERDBG_OPEN_START_RESET_REG         GPIO->P0_RESET_DATA_REG
#define SEGGERDBG_OPEN_START_PIN               (1 << 29)
#endif /* SEGGER_FLASH_LOADER_DEBUG */

/*********************************************************************
*
*       Defines (fixed)
*
**********************************************************************
*/
// Smallest amount of data that can be programmed.
// <PageSize> = 2 ^ Shift. Shift = 8 => <PageSize> = 2^8 = 256 bytes
#define PAGE_SIZE_SHIFT              (8)
// Flashes with uniform sectors only.
// <SectorSize> = 2 ^ Shift. Shift = 12 => <SectorSize> = 2 ^ 12 = 4096 bytes
#define SECTOR_SIZE_SHIFT           (12)

#       if defined(FLASH_DEV_OQFLASH)
#define PHYSICAL_TO_VIRTUAL_ADDR(X)     ((X) - MEMORY_OQSPIC_S_BASE + OQSPI_MEM1_VIRTUAL_BASE_ADDR)
#       elif defined(FLASH_DEV_QFLASH)
#define PHYSICAL_TO_VIRTUAL_ADDR(X)     ((X) - MEMORY_QSPIC_BASE + QSPI_MEM1_VIRTUAL_BASE_ADDR)
#       endif

//
// Default definitions for optional functions if not compiled in
// Makes Api table code further down less ugly
//
#if (SUPPORT_ERASE_CHIP == 0)
#       define EraseChip NULL
#endif
#if (SUPPORT_NATIVE_VERIFY == 0)
#       define Verify NULL
#endif
#if (SUPPORT_NATIVE_READ_FUNCTION == 0)
#       define SEGGER_OPEN_Read NULL
#endif
#if (SUPPORT_SEGGER_OPEN_ERASE == 0)
#       define SEGGER_OPEN_Erase NULL
#endif
#if (SUPPORT_TURBO_MODE == 0)
#       define SEGGER_OPEN_Start NULL
#endif
#if (SUPPORT_BLANK_CHECK == 0)
#       define BlankCheck NULL
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
        U32 AddVariablesHere;
} RESTORE_INFO;

static void _FeedWatchdog(void);

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static RESTORE_INFO _RestoreInfo;

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

/*
 * Mark start of <PrgData> segment. Non-static to make sure linker can keep this symbol.
 * Dummy needed to make sure that <PrgData> section in resulting ELF file is present.
 * Needed by open flash loader logic on PC side
 */
volatile int PRGDATA_StartMarker __attribute__((section ("PrgData")));

// Mark start of <PrgCode> segment. Non-static to make sure linker can keep this symbol.
const SEGGER_OFL_API SEGGER_OFL_Api __attribute__((section ("PrgCode"))) =
{
        _FeedWatchdog,
        Init,
        UnInit,
        EraseSector,
        ProgramPage,
        BlankCheck,
        EraseChip,
        Verify,
        SEGGER_OPEN_CalcCRC,
        SEGGER_OPEN_Read,
        SEGGER_OPEN_Program,
        SEGGER_OPEN_Erase,
        SEGGER_OPEN_Start
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
* _FeedWatchdog
*
* Feeds the watchdog. Needs to be called during RAMCode execution in case of an watchdog is active.
* In case no handling is necessary, it could perform a dummy access, to make sure that this function
* is linked in.
*/
static void _FeedWatchdog(void)
{
#if (DEBUG == 0)
        *((volatile int*) &PRGDATA_StartMarker);  // Dummy operation
#endif
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Init
*
*  Function description
*    Handles the initialization of the flash module.
*    It is called once per flash programming step (Erase, Program, Verify)
*
*  Parameters
*    Addr: Flash base address
*    Freq: Clock frequency in Hz
*    Func: Specifies the action followed by Init() (e.g.: 1 - Erase, 2 - Program, 3 - Verify / Read)
*
*  Return value
*    == 0  O.K.
*    == 1  Error
*
*  Notes
*    (1) This function is mandatory.
*    (2) Use "noinline" attribute to make sure that function is never inlined and label not
*        accidentally removed by linker from ELF file.
*/
__attribute__ ((noinline)) int Init(U32 Addr, U32 Freq, U32 Func)
{
        (void)Addr;
        (void)Freq;
        (void)Func;
        (void)_RestoreInfo;

#if SEGGER_FLASH_LOADER_DEBUG
        hw_pd_power_up_com();
#endif

        SEGGER_DBG_SET_HIGH(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_INIT);
        hw_watchdog_freeze();
        SEGGER_DBG_SET_LOW(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_INIT);

#       if defined(FLASH_DEV_OQFLASH)
        return oqspi_automode_init() ? 0 : 1;
#       elif defined(FLASH_DEV_QFLASH)
        return qspi_automode_init() ? 0 : 1;
#       endif


}

/*********************************************************************
*
*       UnInit
*
*  Function description
*    Handles the de-initialization of the flash module.
*    It is called once per flash programming step (Erase, Program, Verify)
*
*  Parameters
*    Func  Caller type (e.g.: 1 - Erase, 2 - Program, 3 - Verify)
*
*  Return value
*    == 0  O.K.
*    == 1  Error
*
*  Notes
*    (1) This function is mandatory.
*    (2) Use "noinline" attribute to make sure that function is never inlined and label not
*        accidentally removed by linker from ELF file.
*/
__attribute__ ((noinline)) int UnInit(U32 Func)
{
        (void)Func;

        SEGGER_DBG_SET_HIGH(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_UNINIT);

#       if defined(FLASH_DEV_OQFLASH)
        hw_oqspi_set_access_mode(HW_OQSPI_ACCESS_MODE_AUTO);
#       elif defined(FLASH_DEV_QFLASH)
        hw_qspi_set_access_mode(HW_QSPIC, HW_QSPI_ACCESS_MODE_AUTO);
#       endif

        SEGGER_DBG_SET_LOW(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_UNINIT);

#if SEGGER_FLASH_LOADER_DEBUG
        hw_pd_power_down_com();
#endif

        return(0);
}

/*********************************************************************
*
*       EraseSector
*
*  Function description
*    Erases one flash sector.
*
*  Parameters
*    SectorAddr  Absolute address of the sector to be erased
*
*  Return value
*    == 0  O.K.
*    == 1  Error
*
*  Notes
*    (1) This function is mandatory.
*    (2) Use "noinline" attribute to make sure that function is never inlined and label not
*        accidentally removed by linker from ELF file.
*/
__attribute__ ((noinline)) int EraseSector(U32 SectorAddr)
{
        SEGGER_DBG_SET_HIGH(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_ERASE_SECTOR);

        SectorAddr = PHYSICAL_TO_VIRTUAL_ADDR(SectorAddr);

#       if defined(FLASH_DEV_OQFLASH)
        oqspi_automode_erase_flash_sector(SectorAddr);
#       elif defined(FLASH_DEV_QFLASH)
        qspi_automode_erase_flash_sector(SectorAddr);
#       endif

        SEGGER_DBG_SET_LOW(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_ERASE_SECTOR);

        return(0);
}

/*********************************************************************
*
*       ProgramPage
*
*  Function description
*    Programs one flash page.
*
*  Parameters
*    DestAddr  Address to start programming on
*    NumBytes  Number of bytes to program. Guaranteed to be == <FlashDevice.PageSize>
*    pSrcBuff  Pointer to data to be programmed
*
*  Return value
*    == 0  O.K.
*    == 1  Error
*
*  Notes
*    (1) This function is mandatory.
*    (2) Use "noinline" attribute to make sure that function is never inlined and label not
*        accidentally removed by linker from ELF file.
*/
__attribute__ ((noinline)) int ProgramPage(U32 DestAddr, U32 NumBytes, U8 *pSrcBuff)
{
        U32 tmp;
        U8 *curPtr = pSrcBuff;

        SEGGER_DBG_SET_HIGH(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_PAGE_PROGRAM);

        DestAddr = PHYSICAL_TO_VIRTUAL_ADDR(DestAddr);

        while (NumBytes > 0) {
#       if defined(FLASH_DEV_OQFLASH)
                tmp = oqspi_automode_write_flash_page(DestAddr, curPtr, NumBytes);
#       elif defined(FLASH_DEV_QFLASH)
                tmp = qspi_automode_write_flash_page(DestAddr, curPtr, NumBytes);
#       endif
                curPtr += tmp;
                DestAddr += tmp;
                NumBytes -= tmp;
        }

        SEGGER_DBG_SET_LOW(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_PAGE_PROGRAM);

        return (0);
}

/*********************************************************************
*
*       BlankCheck
*
*  Function description
*    Checks if a memory region is blank
*
*  Parameters
*    Addr       Address to start checking
*    NumBytes   Number of bytes to be checked
*    BlankData  Blank (erased) value of flash (Most flashes have 0xFF, some have 0x00, some do
*               not have a defined erased value).
*
*  Return value
*    == 0  O.K., blank
*    == 1  O.K., *not* blank
*     < 0  Error
*
*  Notes
*    (1) This function is optional. If not present, the J-Link software will assume that erased
*        state of a sector can be determined via normal memory-mapped readback of sector.
*    (2) Use "noinline" attribute to make sure that function is never inlined and label not
*        accidentally removed by linker from ELF file.
*/
#if SUPPORT_BLANK_CHECK
__attribute__ ((noinline)) int BlankCheck(U32 Addr, U32 NumBytes, U8 BlankData)
{
        volatile U8* pData;
        //
        // Simply read data from flash and compare against <BlankData>
        //
        _FeedWatchdog();
        pData = (volatile U8*)Addr;
        do {
                if (*pData++ != BlankData) {
                        return 1;
                }
        } while (--NumBytes);

        return 0;
}
#endif
/*********************************************************************
*
*       SEGGER_OPEN_CalcCRC
*
*  Function description
*    Calculates the CRC over a specified number of bytes
*    Even more optimized version of Verify() as this avoids downloading the compare data into the
*    RAMCode for comparison. Heavily reduces traffic between J-Link software and target and therefore
*    speeds up verification process significantly.
*
*  Parameters
*    CRC       CRC start value
*    Addr      Address where to start calculating CRC from
*    NumBytes  Number of bytes to calculate CRC on
*    Polynom   Polynom to be used for CRC calculation
*
*  Return value
*    CRC
*
*  Notes
*    (1) This function is optional
*    (2) Use "noinline" attribute to make sure that function is never inlined and label not
*        accidentally removed by linker from ELF file.
*/
__attribute__ ((noinline)) U32 SEGGER_OPEN_CalcCRC(U32 CRC, U32 Addr, U32 NumBytes, U32 Polynom)
{
        SEGGER_DBG_SET_HIGH(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_OPEN_CALC_CRC);
        // Use lib function from SEGGER by default. Pass API pointer to it because it may need to
        // call the read function (non-memory mapped flashes)
        CRC = SEGGER_OFL_Lib_CalcCRC(&SEGGER_OFL_Api, CRC, Addr, NumBytes, Polynom);
        SEGGER_DBG_SET_LOW(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_OPEN_CALC_CRC);

        return CRC;
}

/*********************************************************************
*
*       SEGGER_OPEN_Program
*
*  Function description
*    Optimized variant of ProgramPage() which allows multiple pages to be programmed in 1 RAMCode call.
*
*  Parameters
*    DestAddr  Address to start flash programming at.
*    NumBytes  Number of bytes to be program. Guaranteed to be multiple of <FlashDevice.PageSize>
*    pSrcBuff  Pointer to data to be programmed
*
*  Return value
*    == 0  O.K.
*    == 1  Error
*
*  Notes
*    (1) This function is optional. If not present, the J-Link software will use ProgramPage()
*    (2) Use "noinline" attribute to make sure that function is never inlined and label not
*        accidentally removed by linker from ELF file.
*/
int SEGGER_OPEN_Program(U32 DestAddr, U32 NumBytes, U8 *pSrcBuff)
{
        U32 NumPages;
        int r;

        SEGGER_DBG_SET_HIGH(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_OPEN_PROGRAM);

        NumPages = (NumBytes >> PAGE_SIZE_SHIFT);
        r = 0;

        do {
                r = ProgramPage(DestAddr, (1uL << PAGE_SIZE_SHIFT), pSrcBuff);
                if (r < 0) {
                        return r;
                }
                DestAddr += (1uL << PAGE_SIZE_SHIFT);
                pSrcBuff += (1uL << PAGE_SIZE_SHIFT);
        } while (--NumPages);

        SEGGER_DBG_SET_LOW(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_OPEN_PROGRAM);

        return r;
}

/*********************************************************************
*
*       Verify
*
*  Function description
*    Verifies flash contents.
*    Usually not compiled in. Only needed for non-memory mapped flashes.
*
*  Parameters
*    Addr      Address to start verify on
*    NumBytes  Number of bytes to verify
*    pBuff     Pointer data to compare flash contents to
*
*  Return value
*    == (Addr + NumBytes): O.K.
*    != (Addr + NumBytes): *not* O.K. (ideally the fail address is returned)
*
*  Notes
*    (1) This function is optional. If not present, the J-Link software will assume that flash
*        memory can be verified via memory-mapped readback of flash contents.
*    (2) Use "noinline" attribute to make sure that function is never inlined and label not
*        accidentally removed by linker from ELF file.
*/
#if SUPPORT_NATIVE_VERIFY
__attribute__ ((noinline)) U32 Verify(U32 Addr, U32 NumBytes, U8 *pBuff)
{
        unsigned char *pFlash;
        unsigned long r;

        SEGGER_DBG_SET_HIGH(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_OPEN_CALC_CRC);

        pFlash = (unsigned char *) Addr;
        r = Addr + NumBytes;
        do {
                if (*pFlash != *pBuff) {
                        r = (unsigned long) pFlash;
                        break;
                }
                pFlash++;
                pBuff++;
        } while (--NumBytes);

        SEGGER_DBG_SET_LOW(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_OPEN_CALC_CRC);

        return r;
}
#endif /* SUPPORT_NATIVE_VERIFY */

/*********************************************************************
*
*       EraseChip
*
*  Function description
*    Erases the entire flash.
*
*  Return value
*    == 0  O.K.
*    == 1  Error
*
*  Notes
*    (1) This function is optional. If not present, J-Link will always use EraseSector() for erasing.
*    (2) Use "noinline" attribute to make sure that function is never inlined and label not
*        accidentally removed by linker from ELF file.
*/
#if SUPPORT_ERASE_CHIP
__attribute__ ((noinline)) int EraseChip(void)
{
#       if defined(FLASH_DEV_OQFLASH)
        oqspi_automode_erase_chip();
#       elif defined(FLASH_DEV_QFLASH)
        qspi_automode_erase_chip();
#       endif

        return 0;
}
#endif /* SUPPORT_ERASE_CHIP */

/*********************************************************************
*
*       SEGGER_OPEN_Read
*
*  Function description
*    Reads a specified number of bytes from flash into the provided buffer.
*    Usually not compiled in. Only needed for non-memory mapped flashes.
*
*  Parameters
*    Addr      Address to start reading from
*    NumBytes  Number of bytes to read
*    pDestBuff Pointer to buffer to store read data
*
*  Return value
*    >= 0: O.K., NumBytes read
*    <  0: Error
*
*  Notes
*    (1) This function is optional. If not present, the J-Link software will assume that a
*        normal memory-mapped read can be performed to read from flash.
*    (2) Use "noinline" attribute to make sure that function is never inlined and label not
*        accidentally removed by linker from ELF file.
*/
#if SUPPORT_NATIVE_READ_FUNCTION
__attribute__ ((noinline)) int SEGGER_OPEN_Read(U32 Addr, U32 NumBytes, U8 *pDestBuff)
{
        //
        // Read function
        // Add your code here...
        //
        //_FeedWatchdog();
        return NumBytes;
}
#endif /* SUPPORT_NATIVE_READ_FUNCTION */

/*********************************************************************
*
*       SEGGER_OPEN_Erase
*
*  Function description
*    Erases one or more flash sectors.
*    The implementation from this template only works on flashes that have uniform sectors.
*
*  Notes
*    (1) This function can rely on that at least one sector will be passed
*    (2) This function must be able to handle multiple sectors at once
*    (3) This function can rely on that only multiple sectors of the same sector
*        size will be passed. (e.g. if the device has two sectors with different
*        sizes, the DLL will call this function two times with NumSectors = 1)
*
*  Parameters
*    SectorAddr:  Address of the start sector to be erased
*    SectorIndex: Index of the start sector to be erased (1st sector handled by this flash bank:
*                 SectorIndex == 0)
*    NumSectors:  Number of sectors to be erased. Min. 1
*
*  Return value
*    == 0  O.K.
*    == 1  Error
*
*  Notes
*    (1) This function is optional. If not present, the J-Link software will use EraseSector()
*    (2) Use "noinline" attribute to make sure that function is never inlined and label not
*        accidentally removed by linker from ELF file.
*/
#if SUPPORT_SEGGER_OPEN_ERASE
__attribute__ ((noinline)) int SEGGER_OPEN_Erase(U32 SectorAddr, U32 SectorIndex, U32 NumSectors)
{
        int r;

        SEGGER_DBG_SET_HIGH(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_OPEN_ERASE);

        (void)SectorIndex;
        _FeedWatchdog();
        r = 0;
        do {
                r = EraseSector(SectorAddr);
                if (r) {
                        break;
                }
                SectorAddr += (1 << SECTOR_SIZE_SHIFT);
        } while (--NumSectors);

        SEGGER_DBG_SET_LOW(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_OPEN_ERASE);

        return r;
}
#endif /* SUPPORT_SEGGER_OPEN_ERASE */

/*********************************************************************
*
*       SEGGER_OPEN_Start
*
*  Function description
*    Starts the turbo mode of flash algo.
*    Currently only available for Cortex-M based targets.
*/
#if SUPPORT_TURBO_MODE
__attribute__ ((noinline)) void SEGGER_OPEN_Start(volatile struct SEGGER_OPEN_CMD_INFO* pInfo)
{
#if SEGGER_FLASH_LOADER_DEBUG
        hw_pd_power_up_com();
#endif

        SEGGER_DBG_SET_HIGH(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_OPEN_START);
        SEGGER_OFL_Lib_StartTurbo(&SEGGER_OFL_Api, pInfo);
        SEGGER_DBG_SET_LOW(SEGGER_FLASH_LOADER_DEBUG, SEGGERDBG_OPEN_START);
}
#endif /* SUPPORT_TURBO_MODE */

/**************************** End of file ***************************/
