/*! \copyright Copyright 2018 Intrinsic ID B.V. All rights reserved.\n
    This text contains proprietary, confidential information of Intrinsic ID B.V.,
    and may be used, copied, distributed and/or published only pursuant to the
    terms of a valid license agreement with Intrinsic ID B.V.\n
    This copyright notice must be retained as part of this text at all times.
*/

#ifndef IID_CONFIGURATION_H
#define IID_CONFIGURATION_H

#ifdef __cplusplus
extern "C"
{
#endif

/*! \addtogroup Configuration
*/

/*@{*/

/******************************************************************************
 * POSIX headers
 *****************************************************************************/

/*! \brief Does <stdint.h> exist.
    \details Macro IID_HAS_STDINT_H can be set to 1 if <stdint.h> is available AND it defines type definitions for intN_t and uintN_t.
 */
#define IID_HAS_STDINT_H 1

/*! \brief Does <stdbool.h> exist.
    \details Macro IID_HAS_STDBOOL_H can be set to 1 if <stdbool.h> is available AND it defines the type definition for bool and _Bool.
 */
#define IID_HAS_STDBOOL_H 1

/*! \brief Does long long type exist.
    \details Macro IID_HAS_LONG_LONG can be set to 1 if the data type \p long \p long exists for the current platform.
 */
#define IID_HAS_LONG_LONG 1

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* IID_CONFIGURATION_H */
