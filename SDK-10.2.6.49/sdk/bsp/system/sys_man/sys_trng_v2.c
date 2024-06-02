/**
 ****************************************************************************************
 *
 * @file sys_trng_v2.c
 *
 * @brief System true random number generation
 *
 * @note  It supports the following devices:
 *        - DA1470X
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if (MAIN_PROCESSOR_BUILD)

#if dg_configUSE_SYS_TRNG

#include <stdint.h>
#include <stdbool.h>
#include "bsp_defaults.h"
#include "sys_trng.h"
#include "hw_aes.h"
#include "sdk_crc16.h"

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */

typedef enum {
        INTRINSIC_AES_ECB_KEY_SZ_128 = 0x10,
        INTRINSIC_AES_ECB_KEY_SZ_192 = 0x18,
        INTRINSIC_AES_ECB_KEY_SZ_256 = 0x20,
} INTRINSIC_AES_ECB_KEY;

/*
 * VARIABLE DECLARATIONS
 *****************************************************************************************
 */

/*
 * Note: CMAC MEM1 (RAM10) is uninitialized at system startup.
 */
__IN_CMAC_MEM1_UNINIT static uint8_t sys_trng_seed[SYS_TRNG_SEED_SIZE] __ALIGNED(4);

__IN_CMAC_MEM1_UNINIT static uint32_t trng_id;

/*
 * LOCAL FUNCTION DECLARATIONS
 *****************************************************************************************
 */

static uint8_t aes_ecb_encrypt(const aes_handle_h aes_acc_handle, const uint8_t *const key,
                               const uint8_t key_size, const uint8_t *const message_block,
                               uint8_t *const data_out);

static SYS_TRNG_ERROR generate_irng_seed(uint8_t *sram_puf, const uint16_t sram_blocks,
                                         uint8_t *const random_seed);

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static uint8_t aes_ecb_encrypt(const aes_handle_h aes_acc_handle, const uint8_t *const key,
                               const uint8_t key_size, const uint8_t *const message_block,
                               uint8_t *const data_out)
{
        HW_AES_KEY_SIZE dlg_key_sz;

        // Check if we have input parameter in valid range
        if ((key == NULL) || (message_block == NULL) || (data_out == NULL)) {
                return 1;
        }

        // Check and convert key size parameter
        switch (key_size) {
        case INTRINSIC_AES_ECB_KEY_SZ_128:
                dlg_key_sz = HW_AES_KEY_SIZE_128;
                break;
        case INTRINSIC_AES_ECB_KEY_SZ_192:
                dlg_key_sz = HW_AES_KEY_SIZE_192;
                break;
        case INTRINSIC_AES_ECB_KEY_SZ_256:
                dlg_key_sz = HW_AES_KEY_SIZE_256;
                break;
        default:
                return 2;
        }

        hw_aes_config_t aes_cfg = {
                .mode             = HW_AES_MODE_ECB,
                .operation        = HW_AES_OPERATION_ENCRYPT,
                .key_size         = dlg_key_sz,
                .key_expand       = HW_AES_KEY_EXPAND_BY_HW,
                .output_data_mode = HW_AES_OUTPUT_DATA_MODE_ALL,
                .wait_more_input  = false,
                .callback         = NULL,
                .iv_cnt_ptr       = NULL,
                .keys_addr        = (uint32_t) key,
                .input_data_addr  = (uint32_t) message_block,
                .output_data_addr = (uint32_t) data_out,
                .input_data_len   = 16
        };

        hw_aes_init(&aes_cfg);
        hw_aes_start_operation(HW_AES_OPERATION_ENCRYPT);
        while (hw_aes_hash_is_active()) {
                ;
        }
        hw_aes_hash_disable_clock();

        return 0;
}

static SYS_TRNG_ERROR generate_irng_seed(uint8_t *sram_puf, const uint16_t sram_blocks,
                                         uint8_t *const random_seed)
{
        aes_ctx_t    aes_dialog;
        SYS_TRNG_ERROR ret_code;

        ASSERT_WARNING(((uint32_t) sram_puf & 0x00000003) == 0);
        ASSERT_WARNING(((uint32_t) random_seed & 0x00000003) == 0);
        ASSERT_WARNING(sram_blocks >= IRNG_MINIMUM_SRAM_PUF_BLOCKS);

        aes_dialog.aes = aes_ecb_encrypt;
        aes_dialog.aes_acc_handle = NULL;

        ret_code = (SYS_TRNG_ERROR) irng_get_random_seed(&aes_dialog, sram_puf, sram_blocks,
                                                         random_seed);
        return ret_code;
}


const uint8_t *sys_trng_get_seed(void)
{
        return sys_trng_seed;
}

bool sys_trng_can_run(void)
{
        if (trng_id != (uint32_t) crc16_calculate(sys_trng_seed, SYS_TRNG_SEED_SIZE)) {
                return true;
        }
        return false;
}

SYS_TRNG_ERROR sys_trng_init(void)
{
        SYS_TRNG_ERROR error = SYS_TRNG_ERROR_NONE;

        error = generate_irng_seed((uint8_t *) dg_configSYS_TRNG_ENTROPY_SRC_ADDR, SYS_TRNG_MEMORY_BLOCKS, sys_trng_seed);

        if (error == SYS_TRNG_ERROR_NONE) {
                trng_id = crc16_calculate(sys_trng_seed, SYS_TRNG_SEED_SIZE);
        }
        return error;
}

#endif /* dg_configUSE_SYS_TRNG */

#endif /* MAIN_PROCESSOR_BUILD */


