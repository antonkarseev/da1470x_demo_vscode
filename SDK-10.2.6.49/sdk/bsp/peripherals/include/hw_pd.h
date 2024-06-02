/**
\addtogroup PLA_DRI_PER_ANALOG
\{
\addtogroup PD Power Domain Driver
\{
\brief Power Domain Driver
*/

/**
****************************************************************************************
*
* @file hw_pd.h
*
* @brief Power Domain Driver header file.
*
* Copyright (C) 2015-2020 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/
#ifndef HW_PD_H_
#define HW_PD_H_


#if dg_configUSE_HW_PD

#include "sdk_defs.h"

/**
 * \enum HW_PD
 * \brief Hardware power domains.
 *
 */
typedef enum {
        HW_PD_AON = 0,      /**< Aon power domain */
        HW_PD_AUD,          /**< Audio and voice power domain */
        HW_PD_CTRL,         /**< External Memory Controller power domain */
        HW_PD_GPU,          /**< GPU power domain */
        HW_PD_MEM,          /**< Memory power domain */
        HW_PD_RAD,          /**< Radio power domain */
        HW_PD_SLP,          /**< Sleep power domain */
        HW_PD_SNC,          /**< Sensor Node Controller power domain */
        HW_PD_SYS,          /**< System power domain */
        HW_PD_TMR,          /**< Timers power domain */
        HW_PD_MAX           /**< Power domain max*/
} HW_PD;

/**
 * \brief This is a legacy function that does nothing on DA1470x.
 */
__STATIC_FORCEINLINE void hw_pd_power_up_periph(void)
{
#ifdef CRG_TOP_PMU_CTRL_REG_PERIPH_SLEEP_Msk
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, PERIPH_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, PER_IS_UP)) == 0);
#endif
}

/**
 * \brief This is a legacy function that does nothing on DA1470x.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_periph(void)
{
#ifdef CRG_TOP_PMU_CTRL_REG_PERIPH_SLEEP_Msk
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, PERIPH_SLEEP);
        GLOBAL_INT_RESTORE();
#endif
}

/**
 * \brief This is a legacy function that does nothing on DA1470x.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_periph(void)
{
#ifdef CRG_TOP_SYS_STAT_REG_PER_IS_DOWN_Msk
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, PER_IS_DOWN)) == 0);
#endif
}

/**
 * \brief This is a legacy function that does nothing on DA1470x.

 * \return false
 */
__STATIC_INLINE bool hw_pd_check_periph_status(void)
{
#ifdef CRG_TOP_SYS_STAT_REG_PER_IS_UP_Msk
        return REG_GETF(CRG_TOP, SYS_STAT_REG, PER_IS_UP) == 1;
#else
        return false;
#endif
}

/**
 * \brief Power up the Radio Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_rad(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, RADIO_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, RAD_IS_UP)) == 0);
}

__STATIC_INLINE void hw_pd_power_down_rad(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, RADIO_SLEEP);
        GLOBAL_INT_RESTORE();
}

__STATIC_FORCEINLINE void hw_pd_wait_power_down_rad(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, RAD_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Radio Power Domain.
 *
 * \return 0, if it is powered down and 1 if it is powered up.
 *
 */
__STATIC_INLINE bool hw_pd_check_rad_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, RAD_IS_UP) == 1;
}


/**
 * \brief Power up the Communications Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_com(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, SNC_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, SNC_IS_UP)) == 0);
}

/**
 * \brief Power down the Communications Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_com(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, SNC_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Communications Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_com(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, SNC_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Communications Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_com_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, SNC_IS_UP) == 1;
}

/**
 * \brief Power up the Timers Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_tim(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, TIM_IS_UP)) == 0);
}

/**
 * \brief Power down the Timers Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_tim(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Timers Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_tim(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, TIM_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Timers Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_tim_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP) == 1;
}

/**
 * \brief Power up the Audio Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_aud(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, AUD_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, AUD_IS_UP)) == 0);
}

/**
 * \brief Power down the Audio Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_aud(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, AUD_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Audio Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_aud(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, AUD_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Audio Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_aud_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, AUD_IS_UP) == 1;
}


/**
 * \brief Power up the Sensor Node Controller Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_snc(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, SNC_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, SNC_IS_UP)) == 0);
}

/**
 * \brief Power down the Sensor Node Controller Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_snc(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, SNC_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Sensor Node Controller Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_snc(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, SNC_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Sensor Node Controller Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_snc_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, SNC_IS_UP) == 1;
}

/**
 * \brief Power up the GPU.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_gpu(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, GPU_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, GPU_IS_UP)) == 0);
}

/**
 * \brief Power down the GPU Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_gpu(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, GPU_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for GPU Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_gpu(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, GPU_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of GPU Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_gpu_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, GPU_IS_UP) == 1;
}

/**
 * \brief Power up the External Memory Controller.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_ctrl(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, CTRL_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, CTRL_IS_UP)) == 0);
}

/**
 * \brief Power down the External Memory Controller Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_ctrl(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, CTRL_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for External Memory Controller Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_ctrl(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, CTRL_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of External Memory Controller Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_ctrl_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, CTRL_IS_UP) == 1;
}


#endif /* dg_configUSE_HW_PD */


#endif /* HW_PD_H_ */

/**
\}
\}
*/
