/**
 ****************************************************************************************
 *
 * @file hw_aes.c
 *
 * @brief Implementation of the HASH Engine Low Level Driver.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_HASH

#include "hw_hash.h"
#include "hw_crypto.h"

bool hw_hash_check_input_data_len_restrictions(void)
{
        bool wait_more_input = hw_aes_hash_get_input_data_mode();
        uint32_t data_len = hw_aes_hash_get_input_data_len();

        if (wait_more_input && (data_len % 0x08)) {
                ASSERT_WARNING(0);
                return false;
        }

        return true;
}

HW_HASH_ERROR hw_hash_init(const hw_hash_config_t *hash_cfg)
{
        HW_AES_HASH_STATUS status;

        // Critical section to avoid race condition
        GLOBAL_INT_DISABLE();
        status = hw_aes_hash_get_status();

        if (status != HW_AES_HASH_STATUS_LOCKED_BY_AES) {
                // Use direct register access instead of the hw_aes_hash_enable_clock()
                // to avoid nested critical section due to the function call.
                REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE);
                hw_hash_set_type(hash_cfg->type);
        }
        GLOBAL_INT_RESTORE();

        // This check has to take place outside the critical section, because an else condition
        // would end up to return from the function without restoring the interrupts.
        if (status == HW_AES_HASH_STATUS_LOCKED_BY_AES) {
                return HW_HASH_ERROR_CRYPTO_ENGINE_LOCKED;
        }

        hw_aes_hash_set_input_data_mode(hash_cfg->wait_more_input);
        hw_aes_hash_set_input_data_len(hash_cfg->input_data_len);
        hw_hash_set_output_data_len(hash_cfg->type, hash_cfg->output_data_len);
        hw_aes_hash_set_input_data_addr(hash_cfg->input_data_addr);
        hw_aes_hash_set_output_data_addr(hash_cfg->output_data_addr);

        if (hash_cfg->callback == NULL) {
                hw_aes_hash_disable_interrupt_source();
                hw_crypto_disable_aes_hash_interrupt();
        } else {
                hw_aes_hash_enable_interrupt_source();
                hw_crypto_enable_aes_hash_interrupt((hw_crypto_cb) hash_cfg->callback);
        }

        if (!hw_hash_check_input_data_len_restrictions()) {
                return HW_HASH_ERROR_INVALID_INPUT_DATA_LEN;
        }

        return HW_HASH_ERROR_NONE;
}

#endif /* dg_configUSE_HW_HASH */

