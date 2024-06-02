/**
 ****************************************************************************************
 *
 * @file mailbox.h
 *
 * @brief Mailbox header file
 *
 * Copyright (C) 2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef MAILBOX_H_
#define MAILBOX_H_

#if dg_configUSE_MAILBOX


#include <stdint.h>

/*
 * ENUMERATIONS
 *****************************************************************************************
 */

/**
 * \brief enum with error codes
 */
typedef enum {
        MAILBOX_ERROR_NONE                      =  0, /**< No error */
        MAILBOX_ERROR_REGISTRATION_FAILED       = -1, /**< Interrupt callback registration failed */
} MAILBOX_ERROR;

/**
 * \brief Enumeration with Main processor mailbox interrupts.
 *        The user may extend this list.
 *
 * \note  The enumeration must always start from number zero.
 *        The maximum supported interrupts are 32.
 */
typedef enum {
        /* RPMsg-Lite mailbox interrupt */
#if dg_configUSE_RPMSG_LITE
        MAILBOX_INT_MAIN_RPMSG_LITE,
#endif /* dg_configUSE_RPMSG_LITE */

        /* Add more mailbox interrupts */

        MAILBOX_INT_MAIN_MAX,                   /* Must not exceed 32 */
} MAILBOX_INT_MAIN;

/**
 * \brief Enumeration with SNC processor mailbox interrupts.
 *        The user may extend this list.
 *
 * \note  The enumeration must always start from number zero.
 *        The maximum supported interrupts are 32.
 */
typedef enum {
        /* RPMsg-Lite mailbox interrupt */
#if dg_configUSE_RPMSG_LITE
        MAILBOX_INT_SNC_RPMSG_LITE,
#endif /* dg_configUSE_RPMSG_LITE */

        /* Add more mailbox interrupts */

        MAILBOX_INT_SNC_MAX,                    /* Must not exceed 32 */
} MAILBOX_INT_SNC;

/*
 * DATA TYPE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Mailbox recipients ids
 */
typedef enum {
        MAILBOX_ID_MAIN_PROCESSOR = 0U,         /**< Main processor mailbox id */
        MAILBOX_ID_SNC_PROCESSOR,               /**< SNC processor mailbox id */
        MAILBOX_ID_MAX                          /**< Invalid processor mailbox id */
} MAILBOX_ID;

/*
 * Mailbox definition
 */
typedef struct {
        uint32_t core[MAILBOX_ID_MAX];
} mailbox_t;

/**
 * \brief Mailbox interrupt callback
 */
typedef void (*mailbox_interrupt_cb_t)(void);

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

/**
 * \brief Initialize the mailbox.
 */
void mailbox_init(void);

/**
 * \brief De-initialize the mailbox.
 */
void mailbox_deinit(void);

/**
 * \brief Set interrupt in the mailbox.
 *        The highest bit (MSB) in the mailbox interrupt word has the highest priority.
 *
 * \param [in] mailbox_id    the mailbox id
 * \param [in] mailbox_int   the mailbox interrupt number (\sa MAILBOX_INT_MAIN, MAILBOX_INT_SNC).
 *                           It is converted to the proper bit mask by the function.
 */
void mailbox_set_int(MAILBOX_ID mailbox_id, uint32_t mailbox_int);

/**
 * \brief Get the pending interrupts from the mailbox.
 *        The highest bit (MSB) in the mailbox interrupt word has the highest priority.
 *
 * \param [in] mailbox_id    the mailbox id
 *
 * \return the mailbox pending interrupts
 */
uint32_t mailbox_get_int(MAILBOX_ID mailbox_id);

/**
 * \brief Clear interrupt in the mailbox.
 *        The highest bit (MSB) in the mailbox interrupt word has the highest priority.
 *
 * \param [in] mailbox_id    the mailbox id
 * \param [in] mailbox_int   the mailbox interrupt number (\sa MAILBOX_INT_MAIN, MAILBOX_INT_SNC).
 *                           It is converted to the proper bit mask by the function.
 */
void mailbox_clear_int(MAILBOX_ID mailbox_id, uint32_t mailbox_int);

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief Register a callback function in the mailbox interrupt handler.
 *        The mailbox interrupt handler is called by the SNC2SYS hardware
 *        interrupt handler.
 *
 * \note Non-applicable in SNC processor execution context
 *
 * \param [in] cb       callback function to be registered
 * \param [in] index    the mailbox interrupt number (\sa MAILBOX_INT_MAIN) - acts
 *                      as the index in the array of the mailbox interrupt callbacks
 *
 * \return 0: callback registration succeeded, <0: callback registration failed
 */
MAILBOX_ERROR mailbox_register_snc2sys_int(mailbox_interrupt_cb_t cb, uint32_t index);

/**
 * \brief Unregister a callback function in the mailbox interrupt handler
 *
 * \note Non-applicable in SNC processor execution context
 *
 * \param [in] index    the mailbox interrupt number (\sa MAILBOX_INT_MAIN) - acts
 *                      as the index in the array of the mailbox interrupt callbacks
 */
void mailbox_unregister_snc2sys_int(uint32_t index);
#elif (SNC_PROCESSOR_BUILD)
/**
 * \brief Register a callback function in the mailbox interrupt handler.
 *        The mailbox interrupt handler is called by the SYS2SNC hardware
 *        interrupt handler.
 *
 * \note Non-applicable in Main processor execution context
 *
 * \param [in] cb       callback function to be registered
 * \param [in] index    the mailbox interrupt number (\sa MAILBOX_INT_SNC) - acts
 *                      as the index in the array of the mailbox interrupt callbacks
 *
 * \return 0: callback registration succeeded, <0: callback registration failed
 */
MAILBOX_ERROR mailbox_register_sys2snc_int(mailbox_interrupt_cb_t cb, uint32_t index);

/**
 * \brief Unregister a callback function in the mailbox interrupt handler
 *
 * \note Non-applicable in Main processor execution context
 *
 * \param [in] index    the mailbox interrupt number (\sa MAILBOX_INT_SNC) - acts
 *                      as the index in the array of the mailbox interrupt callbacks
 */
void mailbox_unregister_sys2snc_int(uint32_t index);
#endif /* PROCESSOR_BUILD */


#endif /* dg_configUSE_MAILBOX */

#endif /* MAILBOX_H_ */
