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

File    : FlashOS.h
Purpose : Contains all defines and prototypes of public functions.
*/

/* Copyright (c) 2022 Modified by Dialog Semiconductor */

#define U8  unsigned char
#define U16 unsigned short
#define U32 unsigned long

#define I8  signed char
#define I16 signed short
#define I32 signed long

struct SEGGER_OPEN_CMD_INFO;  // Forward declaration of OFL lib private struct

typedef struct {
        U32 SectorSize;       // Sector Size in bytes
        U32 SectorStartAddr;  // Start address of the sector area (relative to the "BaseAddr" of the flash)
} sector_info_t;

typedef struct {
        U16 AlgoVer;                    // Algo version number
        U8  Name[128];                  // Flash device name. NEVER change the size of this array!
        U16 Type;                       // Flash device type
        U32 BaseAddr;                   // Flash base address
        U32 TotalSize;                  // Total flash device size in Bytes (256 KB)
        U32 PageSize;                   // Page Size (number of bytes that will be passed to ProgramPage().
                                        // MinAlig is 8 byte
        U32 Reserved;                   // Reserved, should be 0
        U8  ErasedVal;                  // Flash erased value
        U32 TimeoutProg;                // Program page timeout in ms
        U32 TimeoutErase;               // Erase sector timeout in ms
        sector_info_t SectorInfo[4];    // Flash sector layout definition. May be adapted up to 512 entries
} flash_device_t;

typedef struct {
        //
        // Optional functions may be NULL
        //
        void (*pfFeedWatchdog)   (void);                                            // Optional
        int  (*pfInit)           (U32 Addr, U32 Freq, U32 Func);                    // Mandatory
        int  (*pfUnInit)         (U32 Func);                                        // Mandatory
        int  (*pfEraseSector)    (U32 Addr);                                        // Mandatory
        int  (*pfProgramPage)    (U32 Addr, U32 NumBytes, U8 *pSrcBuff);            // Mandatory
        int  (*pfBlankCheck)     (U32 Addr, U32 NumBytes, U8 BlankData);            // Optional
        int  (*pfEraseChip)      (void);                                            // Optional
        U32  (*pfVerify)         (U32 Addr, U32 NumBytes, U8 *pSrcBuff);            // Optional
        U32  (*pfSEGGERCalcCRC)  (U32 CRC, U32 Addr, U32 NumBytes, U32 Polynom);    // Optional
        int  (*pfSEGGERRead)     (U32 Addr, U32 NumBytes, U8 *pDestBuff);           // Optional
        int  (*pfSEGGERProgram)  (U32 DestAddr, U32 NumBytes, U8 *pSrcBuff);        // Optional
        int  (*pfSEGGERErase)    (U32 SectorAddr, U32 SectorIndex, U32 NumSectors); // Optional
        void (*pfSEGGERStart)    (volatile struct SEGGER_OPEN_CMD_INFO* pInfo);     // Optional
} SEGGER_OFL_API;

//
// Keil / CMSIS API
//
__attribute__ ((noinline)) int  Init        (U32 Addr, U32 Freq, U32 Func);                    // Mandatory
__attribute__ ((noinline)) int  UnInit      (U32 Func);                                        // Mandatory
__attribute__ ((noinline)) int  EraseSector (U32 Addr);                                        // Mandatory
__attribute__ ((noinline)) int  ProgramPage (U32 Addr, U32 NumBytes, U8 *pSrcBuff);            // Mandatory
__attribute__ ((noinline)) int  BlankCheck  (U32 Addr, U32 NumBytes, U8 BlankData);            // Optional
__attribute__ ((noinline)) int  EraseChip   (void);                                            // Optional
__attribute__ ((noinline)) U32  Verify      (U32 Addr, U32 NumBytes, U8 *pSrcBuff);            // Optional
//
// SEGGER extensions
//
__attribute__ ((noinline)) U32  SEGGER_OPEN_CalcCRC  (U32 CRC, U32 Addr, U32 NumBytes, U32 Polynom);    // Optional
__attribute__ ((noinline)) int  SEGGER_OPEN_Read     (U32 Addr, U32 NumBytes, U8 *pDestBuff);           // Optional
__attribute__ ((noinline)) int  SEGGER_OPEN_Program  (U32 DestAddr, U32 NumBytes, U8 *pSrcBuff);        // Optional
__attribute__ ((noinline)) int  SEGGER_OPEN_Erase    (U32 SectorAddr, U32 SectorIndex, U32 NumSectors); // Optional
__attribute__ ((noinline)) void SEGGER_OPEN_Start    (volatile struct SEGGER_OPEN_CMD_INFO* pInfo);     // Optional
//
// SEGGER OFL lib helper functions that may be called by specific algo part
//
U32  SEGGER_OFL_Lib_CalcCRC     (const SEGGER_OFL_API* pAPI, U32 CRC, U32 Addr, U32 NumBytes, U32 Polynom);
void SEGGER_OFL_Lib_StartTurbo  (const SEGGER_OFL_API* pAPI, volatile struct SEGGER_OPEN_CMD_INFO* pInfo);

/**************************** End of file ***************************/
