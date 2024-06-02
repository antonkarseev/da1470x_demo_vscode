/**
 ****************************************************************************************
 *
 * @file sys_background_flash_ops.c
 *
 * @brief Background flash operations implementation
 *
 * Copyright (C) 2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS

#include "osal.h"
#include "hw_oqspi.h"
#include "oqspi_automode.h"
#include "../memory/src/oqspi_automode_internal.h"
#include "sys_background_flash_ops_internal.h"

/**
 * \brief Determine whether a background flash operation is Erase or Write.
 */
typedef enum {
        BACKGROUND_OP_ERASE = 0,
        BACKGROUND_OP_WRITE,
} BACKGROUND_OP;

/**
 * \brief Status of background flash operation
 */
typedef enum {
        BACKGROUND_OP_STATUS_RESUMED = 0,
        BACKGROUND_OP_STATUS_SUSPENDED,
} BACKGROUND_OP_STATUS;


/**
 * \brief Essential information for an erase or write flash operation
 */
typedef struct {
        uint32_t addr;                          /**< Address of the flash memory to be erased or written. */
        const uint8_t *buf;                     /**< Pointer to the buffer of data to be written. */
        uint32_t size;                          /**< Number of bytes to be erased or written. */
        uint32_t written;                       /**< Number of written bytes by write operation. */
} flash_op_t;

/**
 * \brief Background flash operation recursive structure.
 */
typedef struct background_flash_ops {
        OS_TASK handle;                         /**< The handle of the registered operation. */
        BACKGROUND_OP operation;                /**< Determines whether this is an erase or write operation */
        BACKGROUND_OP_STATUS status;            /**< The status of the operation (suspended or resumed) */
        flash_op_t flash_op;                    /**< Flash operation essentials */
        struct background_flash_ops *next;      /**< Pointer to the next background operation. */
} background_flash_ops_t;

static __RETAINED background_flash_ops_t *backops_pending;
static background_flash_ops_t *backops_active;
static __RETAINED OS_MUTEX backops_mutex;

/**
 * \brief Register a background flash operation.
 *
 * \param [in]  handle    The handle of the task, which registers the operation.
 * \param [in]  flash_op  Essential information regarding the flash operation.
 * \param [out] operation Pointer to a pointer of a structure, which is allocated by this function,
 *                        and will be freed by the caller later on.
 *
 * \returns     True, if the operation has been registered successfully, otherwise false.
 */

static bool register_operation(OS_TASK handle, flash_op_t flash_op, void **operation)
{
        background_flash_ops_t *op;

        if (backops_mutex == NULL) {
                return false;
        }

        op = (background_flash_ops_t *) OS_MALLOC(sizeof(background_flash_ops_t));
        ASSERT_ERROR(op != NULL);

        *operation = (void *)op;

        OS_MUTEX_GET(backops_mutex, OS_MUTEX_FOREVER);

        op->handle = handle;
        op->operation = (flash_op.buf == NULL) ? BACKGROUND_OP_ERASE : BACKGROUND_OP_WRITE;
        op->status = BACKGROUND_OP_STATUS_RESUMED;
        op->flash_op.addr = flash_op.addr;
        op->flash_op.buf = flash_op.buf;
        op->flash_op.size = flash_op.size;
        op->flash_op.written = 0;
        op->next = NULL;

        if (backops_pending) {

                background_flash_ops_t *p = backops_pending;

                while (p->next != NULL) {
                        p = p->next;
                }

                p->next = op;
        } else {
                backops_pending = op;
        }

        OS_MUTEX_PUT(backops_mutex);

        return true;
}

/**
 * \brief Update the status of the ongoing write operation.
 *
 * \param[out] bool True, if the operation is still in progress, otherwise false.
 *
 * \returns True, if an interrupt is pending, otherwise false.
 */
__RETAINED_CODE static bool update_write_operation_status(bool *in_progress)
{
        bool pending_irq = false;

        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_PAGE_PROG_WL);

        do {
                if ((NVIC->ISER[0] & NVIC->ISPR[0]) || (NVIC->ISER[1] & NVIC->ISPR[1])) {
                        pending_irq = true;
                        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_PAGE_PROG_WL_IRQ);
                }

                *in_progress = oqspi_automode_int_is_busy();

        } while (!pending_irq && *in_progress);

        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_PAGE_PROG_WL_IRQ);
        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_PAGE_PROG_WL);

        return pending_irq;
}

/**
 * \brief Handle a write operation.
 *
 * \returns True, if the operation was completed, otherwise false (i.e. an interrupt is pending).
 */
static __RETAINED_CODE bool handle_pending_write_operation(void)
{
        background_flash_ops_t *op;
        bool pending_irq;
        bool in_progress;
        bool write_completed = false;

        op = backops_pending;

        do {
                op->flash_op.written += oqspi_automode_int_flash_write_page((op->flash_op.addr + op->flash_op.written),
                                                                            (op->flash_op.buf + op->flash_op.written),
                                                                            (op->flash_op.size - op->flash_op.written));

                /* Check if the operation has finished or if an interrupt is pending. */
                pending_irq = update_write_operation_status(&in_progress);

                if (!in_progress && (op->flash_op.written == op->flash_op.size)) {
                        // Notify the waiting task without delay
                        write_completed = true;
                }
        } while (!pending_irq && (op->flash_op.written < op->flash_op.size));

        return write_completed;
}

void sys_background_flash_ops_init(void)
{
        if (!backops_mutex) {
                OS_MUTEX_CREATE(backops_mutex);
                ASSERT_WARNING(backops_mutex);
        }
}

__RETAINED_CODE bool sys_background_flash_ops_handle(void)
{
        bool op_completed = false;

        backops_active = backops_pending;

        if (backops_active != NULL) {

                switch (backops_active->status) {
                case BACKGROUND_OP_STATUS_SUSPENDED:
                        oqspi_automode_int_resume();
                        backops_active->status = BACKGROUND_OP_STATUS_RESUMED;

                        if (backops_active->operation == BACKGROUND_OP_WRITE) {
                                bool pending_irq, in_progress;

                                pending_irq = update_write_operation_status(&in_progress);

                                if (!in_progress) {
                                        if (backops_active->flash_op.written == backops_active->flash_op.size) {
                                                // Notify the waiting task without delay
                                                op_completed = true;
                                        } else if (!pending_irq) {
                                                // More data to write...
                                                op_completed = handle_pending_write_operation();
                                        }
                                }
                        }

                        break;
                case BACKGROUND_OP_STATUS_RESUMED:
                        if (backops_active->operation == BACKGROUND_OP_ERASE) {
                                oqspi_automode_erase_flash_sector(backops_active->flash_op.addr);
                        } else {                        // program
                                op_completed = handle_pending_write_operation();
                        }

                        break;
                default:
                        break;
                }
        }

        return op_completed;
}

__RETAINED_CODE void sys_background_flash_ops_suspend(void)
{
        if (backops_active != NULL) {
                // Disable the interrupts as long as the OQSPIC remains in manual access mode
                GLOBAL_INT_DISABLE();

                DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_SUSPEND);

                oqspi_automode_int_suspend();

                if (oqspi_automode_int_is_suspended()) {
                        backops_active->status = BACKGROUND_OP_STATUS_SUSPENDED;
                } else {
                        if (backops_active->operation == BACKGROUND_OP_ERASE) {
                                backops_active->flash_op.written = 1;
                        }
                }

                oqspi_automode_int_enter_auto_access_mode();

                // Re-enable the interrupts since the OQSPIC switched back to auto access mode
                GLOBAL_INT_RESTORE();

                DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_SUSPEND);
        }
}

__RETAINED_CODE void sys_background_flash_ops_notify(void)
{
        background_flash_ops_t *op;

        op = backops_pending;

        if (op && (op->status == BACKGROUND_OP_STATUS_RESUMED)) {
                if (((op->operation == BACKGROUND_OP_ERASE) && (op->flash_op.written == 1)) ||
                    ((op->operation == BACKGROUND_OP_WRITE) && (op->flash_op.written == op->flash_op.size))) {
                        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_TASK_NOTIFY);

                        // If the write pending operation has been completed
                        if ((op->operation == BACKGROUND_OP_WRITE) && (op->flash_op.written != 0)) {
                                op->flash_op.size = op->flash_op.written;
                        }

                        backops_pending = op->next;
                        OS_TASK_RESUME(op->handle);

                        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_TASK_NOTIFY);
                }
        }
}

__RETAINED_HOT_CODE bool sys_background_flash_ops_is_pending(void)
{
        return backops_pending != NULL;
}

__RETAINED_CODE void sys_background_flash_ops_erase_sector(uint32_t addr)
{
        OS_TASK handle;
        void *op;
        flash_op_t flash_op = {
                .addr = addr,
                .buf = NULL,
                .size = OQSPI_FLASH_SECTOR_SIZE,
        };

        handle = OS_GET_CURRENT_TASK();

        if (register_operation(handle, flash_op, &op)) {
                /* Block until the erase operation to be completed, and the
                 * sys_background_flash_ops_notify() resumes the task. */
                OS_TASK_SUSPEND(handle);
                OS_FREE(op);
        } else {
                /* The function was called before initializing the background operations, i.e. before
                 * calling sys_background_flash_ops_init(), which is called by pm_system_init() */
                ASSERT_WARNING(0);
        }
}

uint32_t sys_background_flash_ops_write_page(uint32_t addr, const uint8_t *src, uint32_t size)
{
        ASSERT_WARNING(size > 0);

        OS_TASK handle;
        uint32_t written;
        void *op;
        flash_op_t flash_op = {
                .addr = addr,
                .buf = src,
                .size = size,
        };

        handle = OS_GET_CURRENT_TASK();

        /* Register a background operation to program this sector */
        if (register_operation(handle, flash_op, &op)) {
                /* Block until the write operation to be completed, and the
                 * sys_background_flash_ops_notify() resumes the task. */
                OS_TASK_SUSPEND(handle);
                OS_FREE(op);
                written = size;
        } else {
                /* The function was called before initializing the background operations, i.e. before
                 * calling sys_background_flash_ops_init(), which is called by pm_system_init() */
                ASSERT_WARNING(0);
                written = 0;
        }

        return written;
}

#endif /* dg_configUSE_SYS_BACKGROUND_FLASH_OPS */
