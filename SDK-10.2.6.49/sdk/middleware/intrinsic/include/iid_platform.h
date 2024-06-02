/*! \copyright Copyright 2018 Intrinsic ID B.V. All rights reserved.\n
    This text contains proprietary, confidential information of Intrinsic ID B.V.,
    and may be used, copied, distributed and/or published only pursuant to the
    terms of a valid license agreement with Intrinsic ID B.V.\n
    This copyright notice must be retained as part of this text at all times.
*/

#ifndef _IID_PLATFORM_H_
#define _IID_PLATFORM_H_

#include "iid_configuration.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*! \addtogroup Platform
*/
/*@{*/

#if IID_HAS_STDINT_H == 1
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif
#else /* IID_HAS_STDINT_H == 1 */
#ifdef WIN32

/*! \brief Signed 8 bit value.
    \details A signed integer type with width 8.
*/
typedef signed __int8    int8_t;

/*! \brief Signed 16 bit value.
    \details A signed integer type with width 16.
*/
typedef signed __int16   int16_t;

/*! \brief Signed 32 bit value.
    \details A signed integer type with width 32.
*/
typedef signed __int32   int32_t;

/*! \brief Signed 64 bit value.
    \details A signed integer type with width 64.
*/
typedef signed __int64   int64_t;

/*! \brief Unsigned 8 bit value.
    \details An unsigned integer type with width 8.
*/
typedef unsigned __int8  uint8_t;

/*! \brief Unsigned 16 bit value.
    \details An unsigned integer type with width 16.
*/
typedef unsigned __int16 uint16_t;

/*! \brief Unsigned 32 bit value.
    \details An unsigned integer type with width 32.
*/
typedef unsigned __int32 uint32_t;

/*! \brief Unsigned 64 bit value.
    \details An unsigned integer type with width 64.
*/
typedef unsigned __int64 uint64_t;

/*! \brief Unsigned 32 bit pointer.
    \details An unsigned pointer type with width 32.
*/
typedef uint32_t           uintptr_t;

#else /* WIN32 */

/* These defaults may need tweaking. */

/*! \brief Signed 8 bit value.
    \details A signed integer type with width 8.
*/
typedef signed char        int8_t;

/*! \brief Signed 16 bit value.
    \details A signed integer type with width 16.
*/
typedef signed short       int16_t;

/*! \brief Signed 32 bit value.
    \details A signed integer type with width 32.
*/
typedef signed long        int32_t;

#if IID_HAS_LONG_LONG == 1
/*! \brief Signed 64 bit value.
    \details A signed integer type with width 64.
*/
typedef signed long long   int64_t;
#endif

/*! \brief Unsigned 8 bit value.
    \details An unsigned integer type with width 8.
*/
typedef unsigned char      uint8_t;

/*! \brief Unsigned 16 bit value.
    \details An unsigned integer type with width 16.
*/
typedef unsigned short     uint16_t;

/*! \brief Unsigned 32 bit value.
    \details An unsigned integer type with width 32.
*/
typedef unsigned long      uint32_t;

#if IID_HAS_LONG_LONG == 1
/*! \brief Unsigned 64 bit value.
    \details An unsigned integer type with width 64.
*/
typedef unsigned long long uint64_t;
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
/*! \brief Unsigned 64 bit pointer.
    \details An unsigned pointer type with width 64.
*/
typedef uint64_t           uintptr_t;
#else
/*! \brief Unsigned 32 bit pointer.
    \details An unsigned pointer type with width 32.
*/
typedef uint32_t           uintptr_t;
#endif

#endif /* WIN32 */

#ifndef __CONCAT
#define __CONCATenate(left,right)       left##right
#define __CONCAT(left,right)            __CONCATenate(left,right)
#endif

#ifndef INT8_C
/*! \brief Appends the correct suffix to a 8-bit signed integer literal.
    \details This macro appends the correct suffix to a 8-bit signed integer literal.
*/
#define INT8_C(value)       ((int8_t)value)
#endif

#ifndef INT16_C
/*! \brief Appends the correct suffix to a 16-bit signed integer literal.
    \details This macro appends the correct suffix to a 16-bit signed integer literal.
*/
#define INT16_C(value)      ((int16_t)value)
#endif

#ifndef INT32_C
/*! \brief Appends the correct suffix to a 32-bit signed integer literal.
    \details This macro appends the correct suffix to a 32-bit signed integer literal.
*/
#define INT32_C(value)      ((int32_t)__CONCAT(value, L))
#endif

#if IID_HAS_LONG_LONG == 1
#ifndef INT64_C
/*! \brief Appends the correct suffix to a 64-bit signed integer literal.
    \details This macro appends the correct suffix to a 64-bit signed integer literal.
*/
#define INT64_C(value)      ((int64_t)__CONCAT(value, LL))
#endif
#endif

#ifndef UINT8_C
/*! \brief Appends the correct suffix to a 8-bit unsigned integer literal.
    \details This macro appends the correct suffix to a 8-bit unsigned integer literal.
*/
#define UINT8_C(value)      ((uint8_t)__CONCAT(value, U))
#endif

#ifndef UINT16_C
/*! \brief Appends the correct suffix to a 16-bit unsigned integer literal.
    \details This macro appends the correct suffix to a 16-bit unsigned integer literal.
*/
#define UINT16_C(value)     ((uint16_t)__CONCAT(value, U))
#endif

#ifndef UINT32_C
/*! \brief Appends the correct suffix to a 32-bit unsigned integer literal.
    \details This macro appends the correct suffix to a 32-bit unsigned integer literal.
*/
#define UINT32_C(value)     ((uint32_t)__CONCAT(value, UL))
#endif

#if IID_HAS_LONG_LONG == 1
#ifndef UINT64_C
/*! \brief Appends the correct suffix to a 64-bit unsigned integer literal.
    \details This macro appends the correct suffix to a 64-bit unsigned integer literal.
*/
#define UINT64_C(value)     ((uint64_t)__CONCAT(value, ULL))
#endif
#endif /* IID_HAS_LONG_LONG == 1 */

#ifndef INT8_MAX
/*! \brief Max signed 8 bit value
    \details The maximum value of a 8 bit signed data type.
*/
#define INT8_MAX        0x7F
#endif

#ifndef INT8_MIN
/*! \brief Min signed 8 bit value
    \details The minimum value of a 8 bit signed data type.
*/
#define INT8_MIN        (-INT_MAX - 1)
#endif

#ifndef UINT8_MAX
/*! \brief Max unsigned 8 bit value
    \details The maximum value of a 8 bit unsigned data type.
*/
#define UINT8_MAX       ((INT8_MAX * 2) + 1)
#endif

#ifndef INT16_MAX
/*! \brief Max signed 16 bit value
    \details The maximum value of a 16 bit signed data type.
*/
#define INT16_MAX       0x7FFF
#endif

/*! \brief Min signed 16 bit value
    \details The minimum value of a 16 bit signed data type.
*/
#define INT16_MIN       (-INT16_MAX -1)

#ifndef UINT16_MAX
/*! \brief Max unsigned 16 bit value
    \details The maximum value of a 16 bit unsigned data type.
*/
#define UINT16_MAX      ((__CONCAT(INT16_MAX, U) * 2U) + 1U)
#endif

#ifndef INT32_MAX
/*! \brief Max signed 32 bit value
    \details The maximum value of a 32 bit signed data type.
*/
#define INT32_MAX       0x7FFFFFFFL
#endif

#ifndef INT32_MIN
/*! \brief Min signed 32 bit value
    \details The minimum value of a 32 bit signed data type.
*/
#define INT32_MIN       (-INT32_MAX -1L)
#endif

#ifndef UINT32_MAX
/*! \brief Max unsigned 32 bit value
    \details The maximum value of a 32 bit unsigned data type.
*/
#define UINT32_MAX      ((__CONCAT(INT32_MAX, U) * 2UL) + 1UL)
#endif

#if IID_HAS_LONG_LONG == 1
#ifndef INT64_MAX
/*! \brief Max signed 64 bit value
    \details The maximum value of a 64 bit signed data type.
*/
#define INT64_MAX       0x7FFFFFFFFFFFFFFFLL
#endif

#ifndef INT64_MIN
/*! \brief Min signed 64 bit value
    \details The minimum value of a 64 bit signed data type.
*/
#define INT64_MIN       (-INT64_MAX -1LL)
#endif

#ifndef UINT64_MAX
/*! \brief Max unsigned 64 bit value
    \details The maximum value of a 64 bit unsigned data type.
*/
#define UINT64_MAX      ((__CONCAT(INT64_MAX, U) * 2ULL) + 1ULL)
#endif
#endif /* IID_HAS_LONG_LONG == 1 */

#endif /* IID_HAS_STDINT_H == 1 */

#ifndef NULL
#ifdef __cplusplus
/*! \brief Pointer to an invalid object.
    \details Indicates that the pointer does not refer to a valid object.
*/
#define NULL    0
#else
/*! \brief Pointer to an invalid object.
    \details Indicates that the pointer does not refer to a valid object.
*/
#define NULL    ((void *)0)
#endif
#endif /* NULL */

#if IID_HAS_STDBOOL_H == 1
    #include <stdbool.h>
#else

/*! \brief Signed int value.
    \details A signed integer type.
*/
/*typedef int _Bool;*/

/*! \brief Expands to \ref _Bool
   \details Expands to \ref _Bool. This is a reserved keyword in C++.
*/
#define bool uint8_t

/*! \brief Expands to the integer 1.
    \details Expands to the integer 1.
*/
#define true    1

/*! \brief Expands to the integer 0.
    \details Expands to the integer 0.
*/
#define false   0

/*! \brief Expands to the integer 1.
    \details Expands to the integer 1.
    \remark An application may undefine and then possibly redefine the macros bool, true, and false.
*/
#define __bool_true_false_are_defined   1
#endif /* IID_HAS_STDBOOL_H == 1 */

#if defined(_MSC_VER)
/* Visual Studio */

/*! \brief Macro to force memory alignment
    \details Macro to force memory alignment.
*/
#define PRE_HIS_ALIGN           __declspec(align(4))

/*! \brief Macro to force memory alignment
\details Macro to force memory alignment.
*/
#define POST_HIS_ALIGN

#ifdef _WIN64
#define UNASSIGNED_POINTER (void *)0xCCCCCCCCCCCCCCCC
#else
#define UNASSIGNED_POINTER (void *)0xCCCCCCCC
#endif

#elif defined(__arm)
/* ARMCC compiler */

/*! \brief Macro to force memory alignment
    \details Macro to force memory alignment.
*/
#define PRE_HIS_ALIGN

/*! \brief Macro to force memory alignment
    \details Macro to force memory alignment.
*/
#define POST_HIS_ALIGN __attribute__((aligned(4)))

#define UNASSIGNED_POINTER NULL

#elif defined(__ICCRX__) || defined(__CCRX__)
/* Renesas RX compiler */

/*! \brief Macro to force memory alignment
    \details Macro to force memory alignment.
*/
#define PRE_HIS_ALIGN

/*! \brief Macro to force memory alignment
    \details Macro to force memory alignment.
*/
#define POST_HIS_ALIGN

#define UNASSIGNED_POINTER NULL

#elif defined(__IAR_SYSTEMS_ICC__)
/* IAR compiler */

/*! \brief Macro to force memory alignment
    \details Macro to force memory alignment.
*/
#define PRE_HIS_ALIGN _Pragma("data_alignment = 4")

/*! \brief Macro to force memory alignment
    \details Macro to force memory alignment.
*/
#define POST_HIS_ALIGN

#define UNASSIGNED_POINTER NULL

#elif defined(__GNUC__)
/* GCC */

/*! \brief Macro to force memory alignment
    \details Macro to force memory alignment.
*/
#define PRE_HIS_ALIGN

/*! \brief Macro to force memory alignment
    \details Macro to force memory alignment.
*/
#define POST_HIS_ALIGN          __attribute__((aligned (4)))

#define UNASSIGNED_POINTER NULL
#else

#warning "Compiler not recognized. There are no macros available for alignment. UNASSIGNED_POINTER has been set to NULL."

/*! \brief Macro to force memory alignment
    \details Macro to force memory alignment.
*/
#define PRE_HIS_ALIGN

/*! \brief Macro to force memory alignment
    \details Macro to force memory alignment.
*/
#define POST_HIS_ALIGN

#define UNASSIGNED_POINTER NULL
#endif /* defined(_MSC_VER) */


/*! \brief The function return type
    \details
*/
typedef uint8_t iid_return_t;

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _IID_PLATFORM_H_ */
