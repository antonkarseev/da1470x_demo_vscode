/**
\addtogroup PLA_DRI_PER_ANALOG
\{
\addtogroup HW_BOD BOD driver
\{
\brief DA1470x BOD LLD
*/

/**
****************************************************************************************
*
* @file hw_bod_da1470x.h
*
* @brief BOD LLD header file for DA1470x.
*
* Copyright (C) 2020-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef HW_BOD_DA1470x_H_
#define HW_BOD_DA1470x_H_


#include "sdk_defs.h"

#if dg_configUSE_BOD

/**
 * \brief The BOD channel name
 */
typedef enum {
        BOD_CHANNEL_VBUS        = REG_POS(CRG_TOP, BOD_CTRL_REG, BOD_VBUS_EN),  /**< VBUS channel */
        BOD_CHANNEL_VBAT        = REG_POS(CRG_TOP, BOD_CTRL_REG, BOD_VBAT_EN),  /**< VBAT channel */
        BOD_CHANNEL_VSYS        = REG_POS(CRG_TOP, BOD_CTRL_REG, BOD_VSYS_EN),  /**< VSYS channel */
        BOD_CHANNEL_1V8         = REG_POS(CRG_TOP, BOD_CTRL_REG, BOD_V18_EN),   /**< 1V8 channel */
        BOD_CHANNEL_1V8P        = REG_POS(CRG_TOP, BOD_CTRL_REG, BOD_V18P_EN),  /**< 1V8P channel */
        BOD_CHANNEL_1V8F        = REG_POS(CRG_TOP, BOD_CTRL_REG, BOD_V18F_EN),  /**< 1V8F channel */
        BOD_CHANNEL_1V4         = REG_POS(CRG_TOP, BOD_CTRL_REG, BOD_V14_EN),   /**< 1V4 channel */
        BOD_CHANNEL_VDD         = REG_POS(CRG_TOP, BOD_CTRL_REG, BOD_V12_EN),   /**< 1V2 channel */
} bod_channel_t;

/**
 * \brief Activate BOD for a channel.
 *
 * \param[in] channel BOD channel
 *
 */
__STATIC_FORCEINLINE void hw_bod_activate_channel(bod_channel_t channel)
{
        switch (channel) {
        case BOD_CHANNEL_VBUS:
                REG_SET_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VBUS_EN);
                break;
        case BOD_CHANNEL_VBAT:
                REG_SET_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VBAT_EN);
                break;
        case BOD_CHANNEL_VSYS:
                REG_SET_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VSYS_EN);
                break;
        case BOD_CHANNEL_1V8:
                REG_SET_BIT(CRG_TOP, BOD_CTRL_REG, BOD_V18_EN);
                break;
        case BOD_CHANNEL_1V8P:
        case BOD_CHANNEL_1V8F:
                REG_SET_MASKED(CRG_TOP, BOD_CTRL_REG,
                               (REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18P_EN) |
                                REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18F_EN)),
                                UINT32_MAX);
                break;
        case BOD_CHANNEL_1V4:
                REG_SET_BIT(CRG_TOP, BOD_CTRL_REG, BOD_V14_EN);
                break;
        case BOD_CHANNEL_VDD:
                REG_SET_BIT(CRG_TOP, BOD_CTRL_REG, BOD_V12_EN);
                break;
        default:
                /* Invalid channel, we should not reach here. */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Deactivate BOD for a channel.
 *
 * \param[in] channel BOD channel
 *
 */
__STATIC_FORCEINLINE void hw_bod_deactivate_channel(bod_channel_t channel)
{
        switch (channel) {
        case BOD_CHANNEL_VBUS:
                REG_CLR_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VBUS_EN);
                break;
        case BOD_CHANNEL_VBAT:
                REG_CLR_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VBAT_EN);
                break;
        case BOD_CHANNEL_VSYS:
                REG_CLR_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VSYS_EN);
                break;
        case BOD_CHANNEL_1V8:
                REG_CLR_BIT(CRG_TOP, BOD_CTRL_REG, BOD_V18_EN);
                break;
        case BOD_CHANNEL_1V8P:
        case BOD_CHANNEL_1V8F:
                REG_SET_MASKED(CRG_TOP, BOD_CTRL_REG,
                              (REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18P_EN) |
                               REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18F_EN)),
                               0);
                break;
        case BOD_CHANNEL_1V4:
                REG_CLR_BIT(CRG_TOP, BOD_CTRL_REG, BOD_V14_EN);
                break;
        case BOD_CHANNEL_VDD:
                REG_CLR_BIT(CRG_TOP, BOD_CTRL_REG, BOD_V12_EN);
                break;
        default:
                /* Invalid channel, we should not reach here. */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Configure BOD.
 *
 */
void hw_bod_configure(void);


#endif /* dg_configUSE_BOD */

/**
 * \brief Deactivate BOD for all channels.
 *
 */
__STATIC_FORCEINLINE void hw_bod_deactivate(void)
{
        uint32_t mask = REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VBUS_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VBAT_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VSYS_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18_EN)  |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18P_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18F_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V14_EN)  |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V12_EN);


        REG_SET_MASKED(CRG_TOP, BOD_CTRL_REG, mask, 0);
}


#endif /* HW_BOD_DA1470x_H_ */

/**
\}
\}
*/
