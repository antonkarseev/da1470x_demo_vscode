/**
 ****************************************************************************************
 *
 * @file mailbox.c
 *
 * @brief Mailbox mechanism for inter-processor notifications
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_MAILBOX


#include <stdint.h>
#include <string.h>
#include "sdk_defs.h"
#include "mailbox.h"
#include "snc.h"
#include "hw_bsr.h"

/*
 * MACROS
 *****************************************************************************************
 */

#if (MAIN_PROCESSOR_BUILD)
#define MAILBOX_HW_MUTEX_GET(pos)        ({ while (!hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, pos)) {} })
#define MAILBOX_HW_MUTEX_PUT(pos)        (hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, pos))

#define MAILBOX_GET_INT()                (mailbox_get_int(MAILBOX_ID_MAIN_PROCESSOR))
#define MAILBOX_CLEAR_INT(index)         (mailbox_clear_int(MAILBOX_ID_MAIN_PROCESSOR, index))

#define MAILBOX_INT_MAX                  MAILBOX_INT_MAIN_MAX
#define MAILBOX_INT_CB(index)            (mailbox_int_main_cb[index])
#elif (SNC_PROCESSOR_BUILD)
#define MAILBOX_HW_MUTEX_GET(pos)        ({ while (!hw_bsr_try_lock(HW_BSR_MASTER_SNC, pos)) {} })
#define MAILBOX_HW_MUTEX_PUT(pos)        (hw_bsr_unlock(HW_BSR_MASTER_SNC, pos))

#define MAILBOX_GET_INT()                (mailbox_get_int(MAILBOX_ID_SNC_PROCESSOR))
#define MAILBOX_CLEAR_INT(index)         (mailbox_clear_int(MAILBOX_ID_SNC_PROCESSOR, index))

#define MAILBOX_INT_MAX                  MAILBOX_INT_SNC_MAX
#define MAILBOX_INT_CB(index)            (mailbox_int_snc_cb[index])
#endif /* PROCESSOR_BUILD */

/*
 * DATA DEFINITIONS
 *****************************************************************************************
 */

#if (SNC_PROCESSOR_BUILD)
/* Mailbox definition */
__SNC_SHARED static mailbox_t mailbox;
#endif /* SNC_PROCESSOR_BUILD */

#if (MAIN_PROCESSOR_BUILD)
/*
 * Callbacks for handling the Main processor mailbox interrupts
 */
__RETAINED static mailbox_interrupt_cb_t mailbox_int_main_cb[MAILBOX_INT_MAIN_MAX];
#elif (SNC_PROCESSOR_BUILD)
/*
 * Callbacks for handling the SNC processor mailbox interrupts
 */
__RETAINED static mailbox_interrupt_cb_t mailbox_int_snc_cb[MAILBOX_INT_SNC_MAX];
#endif /* PROCESSOR_BUILD */

__RETAINED static uint32_t bsr_pos;

/*
 * LOCAL FUNCTION DECLARATIONS
 *****************************************************************************************
 */

/* Mailbox interrupt handler */
static void mailbox_handler(void);

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

void mailbox_init(void)
{
        bsr_pos = HW_BSR_PERIPH_ID_MAILBOX;

#if (MAIN_PROCESSOR_BUILD)
        /* Zero initialize the mailbox interrupt callbacks */
        for (int i = 0 ; i < MAILBOX_INT_MAX ; i++) {
                mailbox_int_main_cb[i] = NULL;
        }

        /* Register the mailbox handler to the SNC2SYS hardware interrupt handler */
        snc_register_snc2sys_int(mailbox_handler);
#elif (SNC_PROCESSOR_BUILD)
        /* Zero initialize the mailbox */
        for (int i = 0 ; i < MAILBOX_ID_MAX ; i++) {
                mailbox.core[i] = 0;
        }

        /* Zero initialize the mailbox interrupt callbacks */
        for (int i = 0 ; i < MAILBOX_INT_MAX ; i++) {
                mailbox_int_snc_cb[i] = NULL;
        }

        /* Register the mailbox handler to the SYS2SNC hardware interrupt handler */
        snc_register_sys2snc_int(mailbox_handler);

        /* Store the address of the SNC defined variable */
        snc_set_shared_space_addr((void *)&mailbox, SNC_SHARED_SPACE_MAILBOX);
#endif /* PROCESSOR_BUILD */
}

void mailbox_deinit(void)
{
#if (MAIN_PROCESSOR_BUILD)
        /* Unregister the mailbox handler from the SNC2SYS hardware interrupt handler */
        snc_unregister_snc2sys_int();
#elif (SNC_PROCESSOR_BUILD)
        /* Unregister the mailbox handler from the SYS2SNC hardware interrupt handler */
        snc_unregister_sys2snc_int();
#endif /* PROCESSOR_BUILD */
}

void mailbox_set_int(MAILBOX_ID mailbox_id, uint32_t mailbox_int)
{
#if (MAIN_PROCESSOR_BUILD)
        ASSERT_ERROR(mailbox_int < MAILBOX_INT_SNC_MAX);

        mailbox_t *mailbox = (mailbox_t *)snc_get_shared_space_addr(SNC_SHARED_SPACE_MAILBOX);

        /* Protect access on shared variable */
        MAILBOX_HW_MUTEX_GET(bsr_pos);
        mailbox->core[mailbox_id] |= (1 << mailbox_int);
        MAILBOX_HW_MUTEX_PUT(bsr_pos);
#elif (SNC_PROCESSOR_BUILD)
        ASSERT_ERROR(mailbox_int < MAILBOX_INT_MAIN_MAX);

        /* Protect access on shared variable */
        MAILBOX_HW_MUTEX_GET(bsr_pos);
        mailbox.core[mailbox_id] |= (1 << mailbox_int);
        MAILBOX_HW_MUTEX_PUT(bsr_pos);
#endif /* PROCESSOR_BUILD */
}

uint32_t mailbox_get_int(MAILBOX_ID mailbox_id)
{
#if (MAIN_PROCESSOR_BUILD)
        mailbox_t *mailbox = (mailbox_t *)snc_get_shared_space_addr(SNC_SHARED_SPACE_MAILBOX);
        return mailbox->core[mailbox_id];
#elif (SNC_PROCESSOR_BUILD)
        return mailbox.core[mailbox_id];
#endif /* PROCESSOR_BUILD */
}

void mailbox_clear_int(MAILBOX_ID mailbox_id, uint32_t mailbox_int)
{
        ASSERT_ERROR(mailbox_int < MAILBOX_INT_MAX);

#if (MAIN_PROCESSOR_BUILD)
        mailbox_t *mailbox = (mailbox_t *)snc_get_shared_space_addr(SNC_SHARED_SPACE_MAILBOX);

        /* Protect access on shared variable */
        MAILBOX_HW_MUTEX_GET(bsr_pos);
        mailbox->core[mailbox_id] &= ~(1 << mailbox_int);
        MAILBOX_HW_MUTEX_PUT(bsr_pos);
#elif (SNC_PROCESSOR_BUILD)
        /* Protect access on shared variable */
        MAILBOX_HW_MUTEX_GET(bsr_pos);
        mailbox.core[mailbox_id] &= ~(1 << mailbox_int);
        MAILBOX_HW_MUTEX_PUT(bsr_pos);
#endif /* PROCESSOR_BUILD */
}

#if (MAIN_PROCESSOR_BUILD)
MAILBOX_ERROR mailbox_register_snc2sys_int(mailbox_interrupt_cb_t cb, uint32_t index)
{
        ASSERT_ERROR(index < MAILBOX_INT_MAX);

        int32_t ret = MAILBOX_ERROR_REGISTRATION_FAILED;
        GLOBAL_INT_DISABLE();
        if (mailbox_int_main_cb[index] == NULL) {
                mailbox_int_main_cb[index] = cb;
                ret = MAILBOX_ERROR_NONE;
        }
        NVIC_ClearPendingIRQ(SNC2SYS_IRQn);
        snc_clear_snc2sys_int();
        GLOBAL_INT_RESTORE();
        NVIC_EnableIRQ(SNC2SYS_IRQn);

        return ret;
}

void mailbox_unregister_snc2sys_int(uint32_t index)
{
        NVIC_DisableIRQ(SNC2SYS_IRQn);
        NVIC_ClearPendingIRQ(SNC2SYS_IRQn);
        mailbox_int_main_cb[index] = NULL;
}
#elif (SNC_PROCESSOR_BUILD)
MAILBOX_ERROR mailbox_register_sys2snc_int(mailbox_interrupt_cb_t cb, uint32_t index)
{
        ASSERT_ERROR(index < MAILBOX_INT_MAX);

        int32_t ret = MAILBOX_ERROR_REGISTRATION_FAILED;
        GLOBAL_INT_DISABLE();
        if (mailbox_int_snc_cb[index] == NULL) {
                mailbox_int_snc_cb[index] = cb;
                ret = MAILBOX_ERROR_NONE;
        }
        NVIC_ClearPendingIRQ(SYS2SNC_IRQn);
        snc_clear_sys2snc_int();
        GLOBAL_INT_RESTORE();
        NVIC_EnableIRQ(SYS2SNC_IRQn);

        return ret;
}

void mailbox_unregister_sys2snc_int(uint32_t index)
{
        NVIC_DisableIRQ(SYS2SNC_IRQn);
        NVIC_ClearPendingIRQ(SYS2SNC_IRQn);
        mailbox_int_snc_cb[index] = NULL;
}
#endif /* PROCESSOR_BUILD */

static void mailbox_handler(void)
{
        /* Get the pending mailbox interrupts word */
        volatile uint32_t interrupt = MAILBOX_GET_INT();

        while (interrupt)
        {
                /* Get the index of the mailbox interrupt callback.
                 * The highest indexed callback has the highest priority.
                 */
                uint8_t index = 32 - (uint8_t) __builtin_clz(interrupt) - 1;

                ASSERT_ERROR(index < MAILBOX_INT_MAX);

                if (MAILBOX_INT_CB(index) != NULL) {
                        /* Clear mailbox interrupt bit */
                        MAILBOX_CLEAR_INT(index);

                        /* Call the registered callback in the mailbox */
                        (MAILBOX_INT_CB(index))();
                } else {
                        ASSERT_ERROR(0);
                }

                /* Get the updated pending mailbox interrupts word */
                interrupt = MAILBOX_GET_INT();
        }
}


#endif /* dg_configUSE_MAILBOX */
