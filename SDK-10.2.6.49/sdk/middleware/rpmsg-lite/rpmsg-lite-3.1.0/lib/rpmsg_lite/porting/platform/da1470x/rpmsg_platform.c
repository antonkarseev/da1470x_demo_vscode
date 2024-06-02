/**
 ****************************************************************************************
 *
 * @file rpmsg_platform.c
 *
 * @brief RPMsg-Lite DA1470x platform layer
 *
 * Copyright (C) 2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_RPMSG_LITE

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "rpmsg_platform.h"
#include "rpmsg_env.h"
#include "sdk_defs.h"
#include "snc.h"
#include "mailbox.h"

#if defined(RL_USE_ENVIRONMENT_CONTEXT) && (RL_USE_ENVIRONMENT_CONTEXT == 1)
#error "This RPMsg-Lite port requires RL_USE_ENVIRONMENT_CONTEXT set to 0"
#endif

#define SHARED_RAM_BASE_ADDR_SNC_PROCESSOR         (uint32_t)(0x00030000)
#define SHARED_RAM_BASE_ADDR_MAIN_PROCESSOR        (uint32_t)(0x20110000)

/**
 * \brief Processor core IDs
 */
typedef enum {
        RPMSG_PLATFORM_CORE_ID_MAIN_PROCESSOR = 0U,         /**< Main processor id */
        RPMSG_PLATFORM_CORE_ID_SNC_PROCESSOR,               /**< SNC processor id */
        RPMSG_PLATFORM_CORE_ID_MAX                          /**< Invalid processor id */
} RPMSG_PLATFORM_CORE_ID;

/*
 * Processor core pending interrupt information
 */
typedef struct {
        uint32_t core[RPMSG_PLATFORM_CORE_ID_MAX];
} isr_pending_t;

static int32_t isr_counter     = 0;
static int32_t disable_counter = 0;
static void *platform_lock;

#if (SNC_PROCESSOR_BUILD)
__SNC_SHARED static volatile isr_pending_t isr_pending;
#endif /* SNC_PROCESSOR_BUILD */

/*
 * Allocate space for RPMsg-Lite data in shared memory.
 * Real needs in memory must be defined per application.
 */
#if (SNC_PROCESSOR_BUILD)
__SNC_SHARED static uint8_t snc_rpmsg_base_addr[RL_PLATFORM_SH_MEM_SIZE];
#endif /* SNC_PROCESSOR_BUILD */

/* RPMSg-Lite interrupt handler */
static void rpmsg_lite_handler(void);

static void platform_global_isr_disable(void)
{
        __disable_irq();
}

static void platform_global_isr_enable(void)
{
        __enable_irq();
}

#if (MAIN_PROCESSOR_BUILD)
static void rpmsg_lite_handler(void)
{
        isr_pending_t *isr_pending = (isr_pending_t *)snc_get_shared_space_addr(SNC_SHARED_SPACE_RPMSG_LITE_ISR_PENDING);

        if (0 != (0x01 & isr_pending->core[RPMSG_PLATFORM_CORE_ID_MAIN_PROCESSOR])) {
                /* Clear internal interrupt status bit */
                isr_pending->core[RPMSG_PLATFORM_CORE_ID_MAIN_PROCESSOR] &= ~(0x01);

                env_isr(0);
        }

        if (0 != (0x02 & isr_pending->core[RPMSG_PLATFORM_CORE_ID_MAIN_PROCESSOR])) {
                /* Clear internal interrupt status bit */
                isr_pending->core[RPMSG_PLATFORM_CORE_ID_MAIN_PROCESSOR] &= ~(0x02);

                env_isr(1);
        }
}
#elif (SNC_PROCESSOR_BUILD)
static void rpmsg_lite_handler(void)
{
        if (0 != (0x01 & isr_pending.core[RPMSG_PLATFORM_CORE_ID_SNC_PROCESSOR])) {
                /* Clear internal interrupt status bit */
                isr_pending.core[RPMSG_PLATFORM_CORE_ID_SNC_PROCESSOR] &= ~(0x01);

                env_isr(0);
        }

        if (0 != (0x02 & isr_pending.core[RPMSG_PLATFORM_CORE_ID_SNC_PROCESSOR])) {
                /* Clear internal interrupt status bit */
                isr_pending.core[RPMSG_PLATFORM_CORE_ID_SNC_PROCESSOR] &= ~(0x02);

                env_isr(1);
        }
}
#endif /* PROCESSOR_BUILD */

int32_t platform_init_interrupt(uint32_t vector_id, void *isr_data)
{
        /* Register ISR to environment layer */
        env_register_isr(vector_id, isr_data);

        env_lock_mutex(platform_lock);

        RL_ASSERT(0 <= isr_counter);
        if (isr_counter == 0) {
#if (MAIN_PROCESSOR_BUILD)
                NVIC_SetPriority(SNC2SYS_IRQn, 2);
#elif (SNC_PROCESSOR_BUILD)
                NVIC_SetPriority(SYS2SNC_IRQn, 1);
#endif /* PROCESSOR_BUILD */
        }
        isr_counter++;

        env_unlock_mutex(platform_lock);

        return 0;
}

int32_t platform_deinit_interrupt(uint32_t vector_id)
{
        /* Prepare the MU Hardware */
        env_lock_mutex(platform_lock);

        RL_ASSERT(0 < isr_counter);
        isr_counter--;
        if (isr_counter == 0) {
#if (MAIN_PROCESSOR_BUILD)
                NVIC_DisableIRQ(SNC2SYS_IRQn);
#elif (SNC_PROCESSOR_BUILD)
                NVIC_DisableIRQ(SYS2SNC_IRQn);
#endif /* PROCESSOR_BUILD */
        }

        /* Unregister ISR from environment layer */
        env_unregister_isr(vector_id);

        env_unlock_mutex(platform_lock);

        return 0;
}

void platform_notify(uint32_t vector_id)
{
#if defined(RL_USE_MCMGR_IPC_ISR_HANDLER) && (RL_USE_MCMGR_IPC_ISR_HANDLER == 1)
        env_lock_mutex(platform_lock);
        (void)MCMGR_TriggerEventForce(kMCMGR_RemoteRPMsgEvent, (uint16_t)RL_GET_Q_ID(vector_id));
        env_unlock_mutex(platform_lock);
#else
        /*
        * Only single RPMsg-Lite instance (LINK_ID) is defined for the Main processor (CM33) to SNC
        * (CM0+) communication. Extend this statement in case multiple instances of RPMsg-Lite are
        * needed.
        */
        switch (RL_GET_LINK_ID(vector_id))
        {
        case RL_PLATFORM_DA1470X_M33_SNC_LINK_ID:
                env_lock_mutex(platform_lock);
                /*
                 * Write directly into the Mailbox register, no need to wait until the content is cleared
                 * (consumed by the receiver side) because the same value of the virtqueue ID is written
                 * into this register when trigerring the ISR for the receiver side. The whole queue of
                 * received buffers for associated virtqueue is handled in the ISR then.
                 */
#if (MAIN_PROCESSOR_BUILD)
                /* Store pending interrupt - internal information for the RPMsg-Lite framework */
                isr_pending_t *isr_pending = (isr_pending_t *)snc_get_shared_space_addr(SNC_SHARED_SPACE_RPMSG_LITE_ISR_PENDING);
                isr_pending->core[RPMSG_PLATFORM_CORE_ID_SNC_PROCESSOR] = 1 << RL_GET_Q_ID(vector_id);

                /* Set the RPMsg-Lite interrupt in the SNC processor mailbox */
                mailbox_set_int(MAILBOX_ID_SNC_PROCESSOR, MAILBOX_INT_SNC_RPMSG_LITE);

                /* Set the hardware interrupt */
                CRG_XTAL->SET_SYS_IRQ_CTRL_REG = REG_MSK(CRG_XTAL, SET_SYS_IRQ_CTRL_REG, SYS2SNC_IRQ_BIT);
#elif (SNC_PROCESSOR_BUILD)
                /* Store pending interrupt - internal information for the RPMsg-Lite framework */
                isr_pending.core[RPMSG_PLATFORM_CORE_ID_MAIN_PROCESSOR] = 1 << RL_GET_Q_ID(vector_id);

                /* Set the RPMsg-Lite interrupt in the Main processor mailbox */
                mailbox_set_int(MAILBOX_ID_MAIN_PROCESSOR, MAILBOX_INT_MAIN_RPMSG_LITE);

                /* Set the hardware interrupt */
                CRG_XTAL->SET_SYS_IRQ_CTRL_REG = REG_MSK(CRG_XTAL, SET_SYS_IRQ_CTRL_REG, SNC2SYS_IRQ_BIT);
#endif /* PROCESSOR_BUILD */
                env_unlock_mutex(platform_lock);
                return;
        default:
                return;
        }
#endif
}

/**
 * platform_time_delay
 *
 * @param num_msec Delay time in ms.
 *
 * This is not an accurate delay, it ensures at least num_msec passed when return.
 */
void platform_time_delay(uint32_t num_msec)
{
        uint32_t loop;

        /* Recalculate the CPU frequency */
        SystemCoreClockUpdate();

        /* Calculate the CPU loops to delay, each loop has 3 cycles */
        loop = SystemCoreClock / 3U / 1000U * num_msec;

        /* There's some difference among toolchains, 3 or 4 cycles each loop */
        while (loop > 0U) {
                __NOP();
                loop--;
        }
}

/**
 * platform_in_isr
 *
 * Return whether CPU is processing IRQ
 *
 * @return True for IRQ, false otherwise.
 *
 */
int32_t platform_in_isr(void)
{
        return (((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0UL) ? 1 : 0);
}

/**
 * platform_interrupt_enable
 *
 * Enable peripheral-related interrupt
 *
 * @param vector_id Virtual vector ID that needs to be converted to IRQ number
 *
 * @return vector_id Return value is never checked.
 *
 */
int32_t platform_interrupt_enable(uint32_t vector_id)
{
        RL_ASSERT(0 < disable_counter);

        platform_global_isr_disable();
        disable_counter--;

        if (disable_counter == 0) {
#if (MAIN_PROCESSOR_BUILD)
                NVIC_EnableIRQ(SNC2SYS_IRQn);
#elif (SNC_PROCESSOR_BUILD)
                NVIC_EnableIRQ(SYS2SNC_IRQn);
#endif /* PROCESSOR_BUILD */
        }
        platform_global_isr_enable();
        return ((int32_t)vector_id);
}

/**
 * platform_interrupt_disable
 *
 * Disable peripheral-related interrupt.
 *
 * @param vector_id Virtual vector ID that needs to be converted to IRQ number
 *
 * @return vector_id Return value is never checked.
 *
 */
int32_t platform_interrupt_disable(uint32_t vector_id)
{
        RL_ASSERT(0 <= disable_counter);

        platform_global_isr_disable();
        /* virtqueues use the same NVIC vector
        if counter is set - the interrupts are disabled */
        if (disable_counter == 0) {
#if (MAIN_PROCESSOR_BUILD)
                NVIC_DisableIRQ(SNC2SYS_IRQn);
#elif (SNC_PROCESSOR_BUILD)
                NVIC_DisableIRQ(SYS2SNC_IRQn);
#endif /* PROCESSOR_BUILD */
        }
        disable_counter++;
        platform_global_isr_enable();
        return ((int32_t)vector_id);
}

/**
 * platform_map_mem_region
 *
 * Dummy implementation
 *
 */
void platform_map_mem_region(uint32_t vrt_addr, uint32_t phy_addr, uint32_t size, uint32_t flags)
{
}

/**
 * platform_cache_all_flush_invalidate
 *
 * Dummy implementation
 *
 */
void platform_cache_all_flush_invalidate(void)
{
}

/**
 * platform_cache_disable
 *
 * Dummy implementation
 *
 */
void platform_cache_disable(void)
{
}

/**
 * platform_vatopa
 *
 * Dummy implementation
 *
 */
uint32_t platform_vatopa(void *addr)
{
        return ((uint32_t)(char *)addr);
}

/**
 * platform_patova
 *
 * Dummy implementation
 *
 */
void *platform_patova(uint32_t addr)
{
#if (MAIN_PROCESSOR_BUILD)
        return ((void *)(char *)addr);
#elif (SNC_PROCESSOR_BUILD)
        return ((void *)(char *)(SHARED_RAM_BASE_ADDR_SNC_PROCESSOR + addr - SHARED_RAM_BASE_ADDR_MAIN_PROCESSOR));
#endif /* PROCESSOR_BUILD */
}

/**
 * platform_init
 *
 * platform/environment init
 */
int32_t platform_init(void)
{
#if (MAIN_PROCESSOR_BUILD)
        /* Register RPMsg-Lite mailbox handler to SNC2SYS interrupt handler */
        if (MAILBOX_ERROR_NONE != mailbox_register_snc2sys_int(rpmsg_lite_handler, MAILBOX_INT_MAIN_RPMSG_LITE)) {
                return -1;
        }
#elif (SNC_PROCESSOR_BUILD)
        /* Zero initialize */
        for (int i = 0 ; i < RPMSG_PLATFORM_CORE_ID_MAX ; i++) {
                isr_pending.core[i] = 0;
        }

        /* Make the SNC shared space defined variables visible to Main processor */
        snc_set_shared_space_addr((void *)&isr_pending, SNC_SHARED_SPACE_RPMSG_LITE_ISR_PENDING);

        /* Register RPMsg-Lite mailbox handler to SYS2SNC interrupt handler */
        if (MAILBOX_ERROR_NONE != mailbox_register_sys2snc_int(rpmsg_lite_handler, MAILBOX_INT_SNC_RPMSG_LITE)) {
                return -1;
        }
#endif /* PROCESSOR_BUILD */

        /* Create lock used in multi-instanced RPMsg */
        if (0 != env_create_mutex(&platform_lock, 1)) {
                return -1;
        }

        return 0;
}

/**
 * platform_deinit
 *
 * platform/environment deinit process
 */
int32_t platform_deinit(void)
{
#if (MAIN_PROCESSOR_BUILD)
        /* Unregister RPMsg-Lite handler from SNC2SYS interrupt handler */
        mailbox_unregister_snc2sys_int(0);
#elif (SNC_PROCESSOR_BUILD)
        /* Unregister RPMsg-Lite handler from SYS2SNC interrupt handler */
        mailbox_unregister_sys2snc_int(0);
#endif /* PROCESSOR_BUILD */

        /* Delete lock used in multi-instanced RPMsg */
        env_delete_mutex(platform_lock);
        platform_lock = ((void *)0);
        return 0;
}

void *platform_get_base_addr(void)
{
#if (MAIN_PROCESSOR_BUILD)
        return (void *) snc_get_shared_space_addr(SNC_SHARED_SPACE_RPMSG_LITE_BASE_ADDR);
#elif (SNC_PROCESSOR_BUILD)
        snc_set_shared_space_addr((void *) &snc_rpmsg_base_addr[0], SNC_SHARED_SPACE_RPMSG_LITE_BASE_ADDR);
        return (void *) &snc_rpmsg_base_addr[0];
#endif /* PROCESSOR_BUILD */
}

#endif /* dg_configUSE_RPMSG_LITE */
