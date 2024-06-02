/**
 ****************************************************************************************
 *
 * @file hw_aes_hash.c
 *
 * @brief Implementation of the AES/Hash Engine Low Level Driver.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "hw_aes_hash.h"
#include "hw_crypto.h"

#if dg_configUSE_HW_AES_HASH
#include "hw_otpc.h"
#include "hw_dma.h"

#define MODE_IS_AES(m)  (m <= HW_AES_CTR)
#endif /* dg_configUSE_HW_AES_HASH */

#if dg_configUSE_HW_AES || dg_configUSE_HW_HASH

void hw_aes_hash_set_input_data_addr(uint32_t inp_data_addr)
{
        uint32_t inp_phy_addr = black_orca_phy_addr(inp_data_addr);

        if (IS_OQSPIC_ADDRESS(inp_phy_addr)) {
                /* Peripherals access OQSPI through a different address range compared to the CPU */
                inp_phy_addr += (MEMORY_OQSPIC_S_BASE - MEMORY_OQSPIC_BASE);
        }

        AES_HASH->CRYPTO_FETCH_ADDR_REG = inp_phy_addr;
}

bool hw_aes_hash_set_output_data_addr(uint32_t out_data_addr)
{
        bool is_dst_addr_valid = false;
        uint32_t out_phy_addr = black_orca_phy_addr(out_data_addr);

#if (dg_configEXEC_MODE == MODE_IS_CACHED)
// When executing from XiP Flash the out_data_addr can only reside in SysRAM
        if (IS_SYSRAM_ADDRESS(out_phy_addr)) {
                is_dst_addr_valid = true;
        }
#else
// When executing from RAM the out_data_addr can reside either in SYSRAM (remapped or not) or in CACHERAM.
        /* cppcheck-suppress unsignedPositive */
        if (IS_SYSRAM_ADDRESS(out_phy_addr) || IS_REMAPPED_ADDRESS(out_phy_addr) ||
            IS_CACHERAM_ADDRESS(out_phy_addr)) {
                is_dst_addr_valid = true;
        }
#endif

        if (is_dst_addr_valid) {
                AES_HASH->CRYPTO_DEST_ADDR_REG = out_phy_addr;
        }

        ASSERT_WARNING(is_dst_addr_valid);
        return is_dst_addr_valid;
}

HW_AES_HASH_STATUS hw_aes_hash_get_status(void)
{
        bool clk_enabled = (bool) hw_aes_hash_clock_is_enabled();
        bool hash_enabled = (bool) REG_GETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL);

        if (clk_enabled && hash_enabled) {
                return HW_AES_HASH_STATUS_LOCKED_BY_HASH;
        } else if (clk_enabled && !hash_enabled) {
                return HW_AES_HASH_STATUS_LOCKED_BY_AES;
        }

        return HW_AES_HASH_STATUS_UNLOCKED;
}

#endif /* dg_configUSE_HW_AES || dg_configUSE_HW_HASH */

#if dg_configUSE_HW_AES || dg_configUSE_HW_HASH  || dg_configUSE_HW_AES_HASH

void hw_aes_hash_deinit(void)
{
        hw_aes_hash_disable_interrupt_source();
        hw_aes_hash_clear_interrupt_req();
        hw_crypto_disable_aes_hash_interrupt();
        hw_crypto_clear_pending_interrupt();
        hw_aes_hash_disable_clock();
}

#endif /* dg_configUSE_HW_AES || dg_configUSE_HW_HASH  || dg_configUSE_HW_AES_HASH */

#if dg_configUSE_HW_AES_HASH

static void hw_aes_hash_wait_on_inactive(void)
{
        while (!REG_GETF(AES_HASH, CRYPTO_STATUS_REG, CRYPTO_INACTIVE)) {
                ;
        }
}

static void hw_aes_hash_set_mode(const hw_aes_hash_setup* setup)
{
        uint32_t crypt0_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        switch (setup->mode) {
        case HW_AES_ECB:
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypt0_ctrl_reg, 0);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypt0_ctrl_reg, 0);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypt0_ctrl_reg, 0);
                break;
        case HW_AES_CBC:
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypt0_ctrl_reg, 0);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypt0_ctrl_reg, 3);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypt0_ctrl_reg, 0);
                AES_HASH->CRYPTO_MREG0_REG = setup->aesIvCtrblk_0_31;
                AES_HASH->CRYPTO_MREG1_REG = setup->aesIvCtrblk_32_63;
                AES_HASH->CRYPTO_MREG2_REG = setup->aesIvCtrblk_64_95;
                AES_HASH->CRYPTO_MREG3_REG = setup->aesIvCtrblk_96_127;
                break;
        case HW_AES_CTR:
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypt0_ctrl_reg, 0);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypt0_ctrl_reg, 2);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypt0_ctrl_reg, 0);
                AES_HASH->CRYPTO_MREG0_REG = setup->aesIvCtrblk_0_31;
                AES_HASH->CRYPTO_MREG1_REG = setup->aesIvCtrblk_32_63;
                AES_HASH->CRYPTO_MREG2_REG = setup->aesIvCtrblk_64_95;
                AES_HASH->CRYPTO_MREG3_REG = setup->aesIvCtrblk_96_127;
                break;
        case HW_HASH_MD5:
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypt0_ctrl_reg, 1);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypt0_ctrl_reg, 0);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypt0_ctrl_reg, 0);
                break;
        case HW_HASH_SHA_1:
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypt0_ctrl_reg, 1);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypt0_ctrl_reg, 0);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypt0_ctrl_reg, 1);
                break;
        case HW_HASH_SHA_256_224:
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypt0_ctrl_reg, 1);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypt0_ctrl_reg, 0);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypt0_ctrl_reg, 2);
                break;
        case HW_HASH_SHA_256:
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypt0_ctrl_reg, 1);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypt0_ctrl_reg, 0);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypt0_ctrl_reg, 3);
                break;
        case HW_HASH_SHA_384:
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypt0_ctrl_reg, 1);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypt0_ctrl_reg, 1);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypt0_ctrl_reg, 0);
                break;
        case HW_HASH_SHA_512:
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypt0_ctrl_reg, 1);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypt0_ctrl_reg, 1);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypt0_ctrl_reg, 1);
                break;
        case HW_HASH_SHA_512_224:
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypt0_ctrl_reg, 1);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypt0_ctrl_reg, 1);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypt0_ctrl_reg, 2);
                break;
        case HW_HASH_SHA_512_256:
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypt0_ctrl_reg, 1);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypt0_ctrl_reg, 1);
                REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypt0_ctrl_reg, 3);
                break;
        }

        AES_HASH->CRYPTO_CTRL_REG = crypt0_ctrl_reg;
}

static void hw_aes_hash_check_data_size(const hw_aes_hash_setup* setup)
{
        switch (setup->mode) {
        case HW_AES_ECB:
                // In ECB mode the dataSize needs to be a multiple of 16.
                ASSERT_ERROR(setup->dataSize % 0x10 == 0);
                break;
        case HW_AES_CBC:
        case HW_AES_CTR:
                // If more data is to come in CBC or CTR mode the dataSize needs to be a multiple of 16.
                if (setup->moreDataToCome) {
                        ASSERT_ERROR(setup->dataSize % 0x10 == 0);
                }
                break;
        case HW_HASH_MD5:
        case HW_HASH_SHA_1:
        case HW_HASH_SHA_256_224:
        case HW_HASH_SHA_256:
        case HW_HASH_SHA_384:
        case HW_HASH_SHA_512:
        case HW_HASH_SHA_512_224:
        case HW_HASH_SHA_512_256:
                // If more data is to come in hash mode the dataSize needs to be a multiple of 8.
                if (setup->moreDataToCome) {
                        ASSERT_ERROR(setup->dataSize % 0x8 == 0);
                }
                break;
        }
}

__STATIC_INLINE uint32 hw_aes_hash_construct_word(const uint8 *data)
{
        if ((uint32)data & 0x3) {
                uint32 internal_buf;
                uint8 *p = (uint8 *)&internal_buf + 3;
                unsigned int i;

                for (i = 0; i < 4; i++) {
                        *(p--) = *(data++);
                }

                return internal_buf;
        }
        else {
                return SWAP32(*(uint32 *)data);
        }
}

bool hw_aes_hash_is_key_revoked(uint8_t idx)
{
        if (idx < HW_OTP_MAX_PAYLOAD_ENTRIES) {
                return hw_otpc_word_read((MEMORY_OTP_USER_DATA_KEYS_INDEX_START / 4) + idx)
                                         ? true : false;
        } else {
                return false;
        }
}

uint32_t hw_aes_hash_keys_address_get(uint8_t idx)
{
        if (!hw_aes_hash_is_key_revoked(idx)) {
                return 0;
        }

        return (MEMORY_OTP_BASE_P + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_START +
                ((uint32_t)idx * HW_OTP_USER_DATA_KEY_SIZE));
}

void hw_aes_hash_nvm_keys_load(hw_aes_key_size key_size, const uint32 *nvm_keys_addr)
{
        unsigned int key_wrds;

        /* Check if key address is from OTP/eFlash User Data Encryption Keys */
        if (WITHIN_RANGE(nvm_keys_addr, MEMORY_OTP_BASE + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_START,
                                        MEMORY_OTP_BASE + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_END) ||
            WITHIN_RANGE(nvm_keys_addr, MEMORY_OTP_BASE_P + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_START,
                                        MEMORY_OTP_BASE_P + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_END)) {

                /* Key expansion is performed by the crypto engine */
                REG_SET_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_AES_KEXP);
                key_wrds = (key_size == HW_AES_256) ? 8 : (key_size == HW_AES_192) ? 6 : 4;

                /* If secure DMA channel is enabled */
                if (hw_dma_is_aes_key_protection_enabled()) {

                        DMA_setup aes_dma_chnl_setup;

                        /* init DMA channel */
                        aes_dma_chnl_setup.channel_number = HW_DMA_SECURE_DMA_CHANNEL;
                        aes_dma_chnl_setup.bus_width = HW_DMA_BW_WORD;
                        aes_dma_chnl_setup.irq_enable = HW_DMA_IRQ_STATE_DISABLED;
                        aes_dma_chnl_setup.irq_nr_of_trans = 0;
                        aes_dma_chnl_setup.dreq_mode = HW_DMA_DREQ_START;
                        aes_dma_chnl_setup.burst_mode = HW_DMA_BURST_MODE_DISABLED;
                        aes_dma_chnl_setup.a_inc = HW_DMA_AINC_TRUE;
                        aes_dma_chnl_setup.b_inc = HW_DMA_BINC_TRUE;
                        aes_dma_chnl_setup.circular = HW_DMA_MODE_NORMAL;
                        aes_dma_chnl_setup.dma_prio = HW_DMA_PRIO_7;
                        aes_dma_chnl_setup.dma_idle = HW_DMA_IDLE_BLOCKING_MODE;
                        aes_dma_chnl_setup.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
                        aes_dma_chnl_setup.dma_req_mux = HW_DMA_TRIG_NONE;
                        aes_dma_chnl_setup.src_address = (uint32_t) nvm_keys_addr;
                        aes_dma_chnl_setup.dest_address = (uint32_t) &AES_HASH->CRYPTO_KEYS_START;
                        aes_dma_chnl_setup.length = key_wrds;
                        aes_dma_chnl_setup.callback = NULL;
                        aes_dma_chnl_setup.user_data = NULL;

                        /* transfer key from OTP to CryptoEngine*/

                        hw_otpc_enter_mode(HW_OTPC_MODE_READ);
                        hw_dma_channel_initialization(&aes_dma_chnl_setup);
                        hw_dma_channel_enable(HW_DMA_SECURE_DMA_CHANNEL, HW_DMA_STATE_ENABLED);

                        /* wait for transaction to finish */
                        while (hw_dma_is_channel_active(HW_DMA_SECURE_DMA_CHANNEL)) {
                                ;
                        }
                } else {
                        hw_otpc_read((uint32_t*) &AES_HASH->CRYPTO_KEYS_START,
                        hw_otpc_address_to_cell_offset((uint32_t) nvm_keys_addr), key_wrds);
                }
        }
}

void hw_aes_hash_keys_load(hw_aes_key_size key_size, const uint8 *aes_keys,
                           hw_aes_hash_key_exp_t key_exp)
{
        unsigned int key_wrds;
        volatile uint32 *kmem_ptr = &AES_HASH->CRYPTO_KEYS_START;

        if (key_exp == HW_AES_DO_NOT_PERFORM_KEY_EXPANSION) {
                /* Key expansion is performed by the software */
                REG_CLR_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_AES_KEXP);
                key_wrds = (key_size == HW_AES_256) ? 60 : (key_size == HW_AES_192) ? 52 : 44;
        } else {
                /* Key expansion is performed by the crypto engine */
                REG_SET_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_AES_KEXP);
                key_wrds = (key_size == HW_AES_256) ? 8 : (key_size == HW_AES_192) ? 6 : 4;
        }

        do {
                *(kmem_ptr++) = hw_aes_hash_construct_word(aes_keys);
                aes_keys += 4;
                key_wrds--;
        } while (key_wrds > 0);
}

void hw_aes_hash_init(hw_aes_hash_setup *setup)
{
        hw_aes_hash_check_data_size(setup);

        hw_aes_hash_enable_clock();

        hw_aes_hash_set_mode(setup);

        uint32_t crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_MORE_IN, crypto_ctrl_reg,
                setup->moreDataToCome);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_OUT_LEN, crypto_ctrl_reg,
                (setup->hashOutLength - 1));
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ENCDEC, crypto_ctrl_reg,
                setup->aesDirection);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_AES_KEXP, crypto_ctrl_reg,
                setup->aesKeyExpand);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_AES_KEY_SZ, crypto_ctrl_reg,
                setup->aesKeySize);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_OUT_MD, crypto_ctrl_reg,
                !setup->aesWriteBackAll);
        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;

        if (MODE_IS_AES(setup->mode)) {
                if (WITHIN_RANGE(setup->aesKeys, MEMORY_OTP_BASE + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_START,
                                                 MEMORY_OTP_BASE + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_END) ||
                    WITHIN_RANGE(setup->aesKeys, MEMORY_OTP_BASE_P + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_START,
                                                 MEMORY_OTP_BASE_P + MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_END)) {
                        hw_aes_hash_nvm_keys_load(setup->aesKeySize, (uint32 *) setup->aesKeys);
                } else {
                        hw_aes_hash_keys_load(setup->aesKeySize, (uint8 *)setup->aesKeys, !setup->aesKeyExpand);
                }
        }

        hw_aes_hash_cfg_dma((const uint8 *)setup->sourceAddress, (uint8 *)setup->destinationAddress,
                            (unsigned int)setup->dataSize);

        if (setup->enableInterrupt) {
                hw_aes_hash_enable_interrupt_source();
                hw_crypto_enable_aes_hash_interrupt((hw_crypto_cb) setup->callback);
        } else {
                hw_aes_hash_disable_interrupt_source();
                hw_crypto_disable_aes_hash_interrupt();
        }
}

void hw_aes_hash_restart(const uint32 sourceAddress, const uint32 dataSize,
        const bool moreDataToCome)
{
        hw_aes_hash_cfg_dma((const uint8 *)sourceAddress, NULL, (unsigned int)dataSize);
        REG_SETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_MORE_IN, moreDataToCome);
        hw_aes_hash_start();
}

void hw_aes_hash_disable(const bool waitOnFinish)
{
        if (waitOnFinish) {
                hw_aes_hash_wait_on_inactive();
        }

        hw_aes_hash_disable_interrupt_source();
        hw_aes_hash_clear_interrupt_req();
        hw_aes_hash_disable_clock();
        hw_aes_hash_mark_input_block_as_last();
}

void hw_aes_hash_cfg_dma(const uint8 *src, uint8 *dst, unsigned int len)
{
        /* Source address setting */
        uint32_t src_address = black_orca_phy_addr((uint32_t)src);

        if (IS_OQSPIC_ADDRESS(src_address)) {
                /* Peripherals access QSPI through a different address range compared to the CPU */
                src_address += (MEMORY_OQSPIC_S_BASE - MEMORY_OQSPIC_BASE);
        }

        AES_HASH->CRYPTO_FETCH_ADDR_REG = src_address;

        /* Destination address setting */
        if (dst) {
                unsigned int remap_type = REG_GETF(CRG_TOP, SYS_CTRL_REG, REMAP_ADR0);

                if (IS_SYSRAM_ADDRESS(dst) ||
                    /* cppcheck-suppress unsignedPositive */
                    (IS_REMAPPED_ADDRESS(dst) && (remap_type == 0x3))) {
                        AES_HASH->CRYPTO_DEST_ADDR_REG = black_orca_phy_addr((uint32)dst);
#if dg_configEXEC_MODE != MODE_IS_CACHED
                } else if (IS_CACHERAM_ADDRESS(dst)) {
                        AES_HASH->CRYPTO_DEST_ADDR_REG = black_orca_phy_addr((uint32)dst);
#endif
                } else {
                        /*
                         * Destination address can only reside in RAM or Cache RAM, but in case of remapped
                         * address, REMAP_ADR0 cannot be 0x6 (Cache Data RAM)
                         */
                        ASSERT_ERROR(0);
                }
        }

        /* Data length setting */
        AES_HASH->CRYPTO_LEN_REG = (uint32)len;
}

static void hw_aes_hash_store_in_mode_dependent_regs(const uint8 *buf)
{
        AES_HASH->CRYPTO_MREG0_REG = hw_aes_hash_construct_word(buf + 12);
        AES_HASH->CRYPTO_MREG1_REG = hw_aes_hash_construct_word(buf + 8);
        AES_HASH->CRYPTO_MREG2_REG = hw_aes_hash_construct_word(buf + 4);
        AES_HASH->CRYPTO_MREG3_REG = hw_aes_hash_construct_word(buf + 0);
}

void hw_aes_hash_store_iv(const uint8 *iv)
{
        hw_aes_hash_store_in_mode_dependent_regs(iv);
}

void hw_aes_hash_store_ic(const uint8 *ic)
{
        hw_aes_hash_store_in_mode_dependent_regs(ic);
}

int hw_aes_hash_check_restrictions(void)
{
        unsigned int more_in = AES_HASH->CRYPTO_CTRL_REG & AES_HASH_CRYPTO_CTRL_REG_CRYPTO_MORE_IN_Msk;

        if (AES_HASH->CRYPTO_CTRL_REG & AES_HASH_CRYPTO_CTRL_REG_CRYPTO_HASH_SEL_Msk) {
                /* Hash */
                if (more_in && (AES_HASH->CRYPTO_LEN_REG & 0x07)) {
                        /* multiple of 8 restriction */
                        return -1;
                }
        }
        else {
                /* AES */
                if (AES_HASH->CRYPTO_LEN_REG & 0x0F) {
                        if (more_in) {
                                /* multiple of 16 restriction */
                                return -1;
                        }
                        unsigned int algorithm_mode = AES_HASH->CRYPTO_CTRL_REG & AES_HASH_CRYPTO_CTRL_REG_CRYPTO_ALG_MD_Msk;
                        if (algorithm_mode == 0x00 || algorithm_mode == 0x04) {
                                /* ECB mode - all blocks must be multiple of 16 */
                                return -1;
                        }
                }
        }
        return 0;
}

#endif /* dg_configUSE_HW_AES_HASH */

