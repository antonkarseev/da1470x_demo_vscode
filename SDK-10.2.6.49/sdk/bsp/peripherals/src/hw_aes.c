/**
 ****************************************************************************************
 *
 * @file hw_aes.c
 *
 * @brief Implementation of the AES Engine Low Level Driver.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_AES

#include "hw_aes.h"
#include "hw_crypto.h"
#include "hw_dma.h"
#include "hw_otpc.h"

static bool is_key_address_within_valid_nvm_range(uint32_t key_addr)
{
        if (WITHIN_RANGE(key_addr, (MEMORY_OTP_BASE + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_START),
                                   (MEMORY_OTP_BASE + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_END)) ||
            WITHIN_RANGE(key_addr, (MEMORY_OTP_BASE_P + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_START),
                                   (MEMORY_OTP_BASE_P + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_END))) {
                return true;
        }

        return false;
}

static uint8_t calculate_key_words_len(HW_AES_KEY_SIZE key_size, HW_AES_KEY_EXPAND key_exp)
{
        switch (key_exp) {
        case HW_AES_KEY_EXPAND_BY_HW:
                return (key_size == HW_AES_KEY_SIZE_256) ? 8 :
                       (key_size == HW_AES_KEY_SIZE_192) ? 6 : 4;
        case HW_AES_KEY_EXPAND_BY_SW:
                return (key_size == HW_AES_KEY_SIZE_256) ? 60 :
                       (key_size == HW_AES_KEY_SIZE_192) ? 52 : 44;
        default:
                ASSERT_WARNING(0);
                return 0;
        }
}

// Transfer the AES Key from NVM to Crypto engine using the secure DMA channel
static void secure_key_transfer_from_nvm(uint32_t key_src_addr, uint8_t key_words)
{
        DMA_setup aes_dma_setup;

        /* Init DMA channel */
        aes_dma_setup.channel_number = HW_DMA_SECURE_DMA_CHANNEL;
        aes_dma_setup.bus_width = HW_DMA_BW_WORD;
        aes_dma_setup.irq_enable = HW_DMA_IRQ_STATE_DISABLED;
        aes_dma_setup.irq_nr_of_trans = 0;
        aes_dma_setup.dreq_mode = HW_DMA_DREQ_START;
        aes_dma_setup.burst_mode = HW_DMA_BURST_MODE_DISABLED;
        aes_dma_setup.a_inc = HW_DMA_AINC_TRUE;
        aes_dma_setup.b_inc = HW_DMA_BINC_TRUE;
        aes_dma_setup.circular = HW_DMA_MODE_NORMAL;
        aes_dma_setup.dma_prio = HW_DMA_PRIO_7;
        aes_dma_setup.dma_idle = HW_DMA_IDLE_BLOCKING_MODE;
        aes_dma_setup.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
        aes_dma_setup.dma_req_mux = HW_DMA_TRIG_NONE;
        aes_dma_setup.src_address = key_src_addr;
        aes_dma_setup.dest_address = (uint32_t) &AES_HASH->CRYPTO_KEYS_START;
        aes_dma_setup.length = (dma_size_t) key_words;
        aes_dma_setup.callback = NULL;
        aes_dma_setup.user_data = NULL;

        /* Transfer key from NVM to CryptoEngine */
        hw_dma_channel_initialization(&aes_dma_setup);
        hw_dma_channel_enable(HW_DMA_SECURE_DMA_CHANNEL, HW_DMA_STATE_ENABLED);

        /* wait for transaction to finish */
        while (hw_dma_is_channel_active(HW_DMA_SECURE_DMA_CHANNEL)) {
                ;
        }
}

static void non_secure_key_transfer_from_nvm(uint32_t key_src_addr, uint8_t key_words)
{
        volatile uint32_t *key_dst_ptr = &AES_HASH->CRYPTO_KEYS_START;

        uint32_t otp_cell_offset = hw_otpc_address_to_cell_offset(key_src_addr);
        hw_otpc_read((uint32_t *) key_dst_ptr, otp_cell_offset, key_words);
}

bool hw_aes_check_input_data_len_restrictions(void)
{
        HW_AES_MODE aes_mode = hw_aes_get_mode();
        bool wait_more_input = hw_aes_hash_get_input_data_mode();
        uint32_t data_len = hw_aes_hash_get_input_data_len();

        switch (aes_mode) {
        case HW_AES_MODE_ECB:
                if (data_len % 0x10) {
                        ASSERT_WARNING(0);
                        return false;
                }
                break;
        case HW_AES_MODE_CBC:
        case HW_AES_MODE_CTR:
                if (wait_more_input && (data_len % 0x10)) {
                        ASSERT_WARNING(0);
                        return false;
                }
                break;
        default:
                ASSERT_WARNING(0);
                return false;
        }

        return true;
}

void hw_aes_set_init_vector(const uint8_t* iv_cnt_ptr)
{
        AES_HASH->CRYPTO_MREG0_REG = SWAP32(__UNALIGNED_UINT32_READ((const uint8_t *) (iv_cnt_ptr + 12)));
        AES_HASH->CRYPTO_MREG1_REG = SWAP32(__UNALIGNED_UINT32_READ((const uint8_t *) (iv_cnt_ptr + 8)));
        AES_HASH->CRYPTO_MREG2_REG = SWAP32(__UNALIGNED_UINT32_READ((const uint8_t *) (iv_cnt_ptr + 4)));
        AES_HASH->CRYPTO_MREG3_REG = SWAP32(__UNALIGNED_UINT32_READ(iv_cnt_ptr));
}

void hw_aes_load_keys(uint32_t key_src_addr, HW_AES_KEY_SIZE key_size, HW_AES_KEY_EXPAND key_exp)
{
        uint8_t key_wrds;

        key_wrds = calculate_key_words_len(key_size, key_exp);

        if (is_key_address_within_valid_nvm_range(key_src_addr)) {
                /* Key expansion has to be performed by the engine */
                ASSERT_WARNING(key_exp == HW_AES_KEY_EXPAND_BY_HW);

                if (!hw_otpc_is_active()) {
                        hw_otpc_init();
                }

                hw_otpc_enter_mode(HW_OTPC_MODE_READ);
                if (hw_dma_is_aes_key_protection_enabled()) {
                        secure_key_transfer_from_nvm(key_src_addr, key_wrds);
                } else {
                        non_secure_key_transfer_from_nvm(key_src_addr, key_wrds);
                }
        } else {
                volatile uint32_t *key_dst_ptr = &AES_HASH->CRYPTO_KEYS_START;

                do {
                        *key_dst_ptr = SWAP32(__UNALIGNED_UINT32_READ(key_src_addr));
                        ++key_dst_ptr;
                        key_src_addr += sizeof(uint32_t);
                        key_wrds--;
                } while (key_wrds > 0);
        }
}

HW_AES_ERROR hw_aes_init(const hw_aes_config_t *aes_cfg)
{
        HW_AES_HASH_STATUS status;

        // Critical section to avoid race condition
        GLOBAL_INT_DISABLE();
        status = hw_aes_hash_get_status();

        if (status != HW_AES_HASH_STATUS_LOCKED_BY_HASH) {
                // Use direct register access instead of the hw_aes_hash_enable_clock()
                // to avoid nested critical section due to the function call.
                REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE);
                hw_aes_set_mode(aes_cfg->mode);
        }
        GLOBAL_INT_RESTORE();

        // This check has to take place outside the critical section, because an else condition
        // would end up to return from the function without restoring the interrupts.
        if (status == HW_AES_HASH_STATUS_LOCKED_BY_HASH) {
                return HW_AES_ERROR_CRYPTO_ENGINE_LOCKED;
        }

        hw_aes_set_operation(aes_cfg->operation);
        hw_aes_set_key_size(aes_cfg->key_size);
        hw_aes_set_key_expansion(aes_cfg->key_expand);
        hw_aes_set_output_data_mode(aes_cfg->output_data_mode);
        hw_aes_hash_set_input_data_mode(aes_cfg->wait_more_input);

        if (aes_cfg->mode == HW_AES_MODE_CBC || aes_cfg->mode == HW_AES_MODE_CTR) {
                hw_aes_set_init_vector(aes_cfg->iv_cnt_ptr);
        }

        hw_aes_load_keys(aes_cfg->keys_addr, aes_cfg->key_size, aes_cfg->key_expand);
        hw_aes_hash_set_input_data_addr(aes_cfg->input_data_addr);
        hw_aes_hash_set_output_data_addr(aes_cfg->output_data_addr);
        hw_aes_hash_set_input_data_len(aes_cfg->input_data_len);

        if (aes_cfg->callback == NULL) {
                hw_aes_hash_disable_interrupt_source();
                hw_crypto_disable_aes_hash_interrupt();
        } else {
                hw_aes_hash_enable_interrupt_source();
                hw_crypto_enable_aes_hash_interrupt((hw_crypto_cb) aes_cfg->callback);
        }

        if (!hw_aes_check_input_data_len_restrictions()) {
                return HW_AES_ERROR_INVALID_INPUT_DATA_LEN;
        }

        return HW_AES_ERROR_NONE;
}

#endif /* dg_configUSE_HW_AES */
