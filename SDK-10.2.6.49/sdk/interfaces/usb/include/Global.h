/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2020     SEGGER Microcontroller GmbH              *
*                                                                    *
*       www.segger.com     Support: www.segger.com/ticket            *
*                                                                    *
**********************************************************************
*                                                                    *
*       emUSB-Device * USB Device stack for embedded applications    *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product.                          *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       emUSB-Device version: V3.36.1                                *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
Licensing information
Licensor:                 SEGGER Software GmbH
Licensed to:              Dialog Semiconductor BV, Het Zuiderkruis 53, 5215 MV S-Hertogenbosch, The Netherlands
Licensed SEGGER software: emUSB-Device
License number:           USBD-00327
License model:            Buyout SRC [Buyout Source Code License], signed on 8th August, 2016 and
                          Amendment No. 1, signed on 26th September, 2017 and
                          Amendment No. 2, signed on 23rd May, 2018 and
                          Amendment No. 3, signed on 3rd June, 2019 and
                          Amendment No. 4, signed on 5th November, 2020 and
                          Amendment No. 5, signed on 5th November, 2020
Licensed product:         Any
Licensed platform:        D2320, D2522, D2798
Licensed number of seats: -
----------------------------------------------------------------------
Support and Update Agreement (SUA)
SUA period:               2016-08-10 - 2021-02-21
Contact to extend SUA:    sales@segger.com
----------------------------------------------------------------------
File    : Global.h
Purpose : Global types
          In case your application already has a Global.h, you should
          merge the files. In order to use Segger code, the types
          U8, U16, U32, I8, I16, I32 need to be defined in Global.h;
          additional definitions do not hurt.
Revision: $Rev: 20326 $
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GLOBAL_H            // Guard against multiple inclusion
#define GLOBAL_H

#define U8    unsigned char
#define I8    signed char
#define U16   unsigned short
#define I16   signed short
#if defined(__x86_64__) || defined(__aarch64__)
#define U32   unsigned int
#define I32   int
#else
#define U32   unsigned long
#define I32   signed long
#endif

//
// CC_NO_LONG_SUPPORT can be defined to compile test
// without long support for compilers that do not
// support C99 and its long type.
//
#ifdef CC_NO_LONG_SUPPORT
  #define PTR_ADDR  U32
#else  // Supports long type.
#if defined(_WIN32) && !defined(__clang__) && !defined(__MINGW32__)
  //
  // Microsoft VC6 compiler related
  //
  #define U64   unsigned __int64
  #define U128  unsigned __int128
  #define I64   __int64
  #define I128  __int128
  #if _MSC_VER <= 1200
    #define U64_C(x) x##UI64
  #else
    #define U64_C(x) x##ULL
  #endif
#else
  //
  // C99 compliant compiler
  //
  #define U64   unsigned long long
  #define I64   signed long long
  #define U64_C(x) x##ULL
#endif

#if (defined(_WIN64) || defined(__LP64__))  // 64-bit symbols used by Visual Studio and GCC, maybe others as well.
  #define PTR_ADDR  U64
#else
  #define PTR_ADDR  U32
#endif
#endif  // Supports long type.

#endif                      // Avoid multiple inclusion

/*************************** End of file ****************************/
