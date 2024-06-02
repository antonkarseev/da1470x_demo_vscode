/**
 * \addtogroup PLA_DRI_CRYPTO
 * \{
 * \addtogroup AES_HASH AES / HASH
 * \{
 * \brief AES/Hash Engine
 */

/**
 ****************************************************************************************
 *
 * @file hw_aes_hash.h
 *
 * @brief Definition of API for the AES/HASH Engine Low Level Driver.
 *
 * Copyright (C) 2015-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_AES_HASH_H_
#define HW_AES_HASH_H_

#include <stdbool.h>
#include "sdk_defs.h"


#if dg_configUSE_HW_AES || dg_configUSE_HW_HASH || dg_configUSE_HW_AES_HASH

/**
 * \brief       AES/HASH engine status
 */
typedef enum {
        HW_AES_HASH_STATUS_UNLOCKED = 0,
        HW_AES_HASH_STATUS_LOCKED_BY_AES  = 1,
        HW_AES_HASH_STATUS_LOCKED_BY_HASH = 2,
} HW_AES_HASH_STATUS;

/**
 * \brief       Masks of AES/HASH Engine Interrupt sources
 *
 * Use these enumerator values to detect which interrupt source triggered the Crypto_Handler by
 * masking the status variable of the IRQ callback with them, as indicated by the next example:
 *
 * \code
 *
 * void aes_hash_callback(uint32_t status)
 * {
 *      bool active;
 *      bool waiting_for_input;
 *
 *      active = !(status & HW_AES_HASH_IRQ_MASK_INACTIVE);
 *      waiting_for_input = !(status & HW_AES_HASH_IRQ_MASK_WAITING_FOR_INPUT);
 *      ...
 * \endcode
 */

typedef enum {
        HW_AES_HASH_IRQ_MASK_INACTIVE = REG_MSK(AES_HASH, CRYPTO_STATUS_REG, CRYPTO_INACTIVE),
        HW_AES_HASH_IRQ_MASK_WAITING_FOR_INPUT = REG_MSK(AES_HASH, CRYPTO_STATUS_REG, CRYPTO_WAIT_FOR_IN),
} HW_AES_HASH_IRQ_MASK;

/**
 * \brief       Set AES/HASH engine input data mode
 *
 * \param [in]  wait_more_input  If true, the AES/HASH engine expects more input data to be received,
 *                               thus when the current input data has been processed, it waits for
 *                               incoming data by setting the corresponding flag (CRYPTO_WAIT_FOR_IN).
 *                               If false, the current input data is considered as the last one and
 *                               the output data is written to the memory.
 */
__STATIC_INLINE void hw_aes_hash_set_input_data_mode(bool wait_more_input)
{
        REG_SETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_MORE_IN, wait_more_input);
}

/**
 * \brief       Get AES/HASH engine input data mode
 *
 * \return      true if the AES/HASH engine expects more input data to be received, otherwise false.
 */
__STATIC_INLINE bool hw_aes_hash_get_input_data_mode(void)
{
        return REG_GETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_MORE_IN);
}

/**
 * \brief       Set the input data length
 *
 * \param [in]  len Input data length
 */
__STATIC_INLINE void hw_aes_hash_set_input_data_len(uint32_t len)
{
        AES_HASH->CRYPTO_LEN_REG = len;
}

/**
 * \brief       Get the input data length
 *
 * \return      Input data length
 */
__STATIC_INLINE uint32_t hw_aes_hash_get_input_data_len(void)
{
        return AES_HASH->CRYPTO_LEN_REG;
}

/**
 * \brief Check whether the AES/Hash Engine is waiting for more input data or not.
 *
 * \return true if the AES/Hash engine is waiting more data, otherwise false.
 *
 */
__STATIC_INLINE bool hw_aes_hash_waiting_for_input_data(void)
{
        return REG_GETF(AES_HASH, CRYPTO_STATUS_REG, CRYPTO_WAIT_FOR_IN) == 0;
}

/**
 * \brief       Set the address of the Input Data
 *
 * \param [in]  inp_data_addr Address of the Input Data
 */
void hw_aes_hash_set_input_data_addr(uint32_t inp_data_addr);

/**
 * \brief       Set the address of the Output Data
 *
 * \param [in]  out_data_addr The output data address. When executing from XiP Flash this address
 *                            can explicitly reside in SYSRAM while execution from RAM allows to set
 *                            this address either in SYSRAM (remapped or not) or in CACHERAM.
 *
 * \return      true if the out_data_addr is acceptable, otherwise false.
 */
bool hw_aes_hash_set_output_data_addr(uint32_t out_data_addr);

/**
 * \brief       Get the status of the AES/HASH engine.
 *
 * \return      The status of the AES/HASH engine
 *
 * \sa          HW_AES_HASH_STATUS
 */
HW_AES_HASH_STATUS hw_aes_hash_get_status(void);
#endif /* dg_configUSE_HW_AES || dg_configUSE_HW_HASH */

#if dg_configUSE_HW_AES || dg_configUSE_HW_HASH || dg_configUSE_HW_AES_HASH
/**
 * \brief AES/Hash callback
 *
 * This function is called by the AES/Hash driver when the interrupt is fired.
 *
 */
typedef void (*hw_aes_hash_cb)(uint32_t status);

/**
 * \brief       Enable AES/HASH engine clock
 *
 */
__STATIC_INLINE void hw_aes_hash_enable_clock(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief       Disable AES/HASH engine clock
 *
 */
__STATIC_INLINE void hw_aes_hash_disable_clock(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief       Check whether the AES/HASH engine clock is enabled or not.
 *
 * \return      true if enabled, otherwise false.
 */
__STATIC_INLINE bool hw_aes_hash_clock_is_enabled(void)
{
        return REG_GETF(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE);
}

/**
 * \brief AES/Hash is active.
 *
 * Check whether the AES/Hash engine is active or not.
 *
 * \return true if the AES/Hash engine is active, otherwise false.
 *
 */
__STATIC_INLINE bool hw_aes_hash_is_active(void)
{
        return REG_GETF(AES_HASH, CRYPTO_STATUS_REG, CRYPTO_INACTIVE) == 0;
}

/**
 * \brief Start AES/HASH engine operation
 *
 * Start an AES/HASH operation depending on the configuration of the AES/HASH Engine.
 *
 */
__STATIC_INLINE void hw_aes_hash_start(void)
{
        AES_HASH->CRYPTO_START_REG = 1;
}

/**
 * \brief Enable AES/HASH engine interrupt source
 *
 * This function enables AES/HASH engine interrupt source.
 *
 * \note All the available HW engines share the same interrupt Handler (Crypto_Handler). In order
 *       to enable the Crypto interrupt you need to use also the hw_crypto_enable_aes_hash_interrupt().
 */
__STATIC_INLINE void hw_aes_hash_enable_interrupt_source(void)
{
        REG_SET_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_IRQ_EN);
}

/**
 * \brief Disable AES/HASH engine interrupt source
 *
 * This function disables AES/HASH engine interrupt source.
 *
 * \note AES/HASH engine and ECC engine are common sources of CRYPTO system interrupt. This
 *       function does not disable the CRYPTO interrupt itself. Use hw_crypto_disable_aes_hash_interrupt()
 *       in order to disable the CRYPTO interrupt.
 */
__STATIC_INLINE void hw_aes_hash_disable_interrupt_source(void)
{
        REG_CLR_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_IRQ_EN);
}

/**
 * \brief Clear AES/HASH engine pending interrupt
 *
 * This function clears AES/HASH engine pending interrupt request.
 *
 * \note AES/HASH engine and ECC engine are common sources of CRYPTO system interrupt. This
 *       function does not clear pending CRYPTO interrupt. Use hw_crypto_clear_pending_interrupt()
 *       in order to clear pending CRYPTO interrupt.
 */
__STATIC_INLINE void hw_aes_hash_clear_interrupt_req(void)
{
        AES_HASH->CRYPTO_CLRIRQ_REG = 0x1;
}

/**
 * \brief De-initialize AES/HASH crypto engine
 *
 * This function disables the AES/HASH engine interrupt, clears any pending interrupt request and
 * disables the AES/HASH engine clock.
 */
void hw_aes_hash_deinit(void);

#endif /* dg_configUSE_HW_AES || dg_configUSE_HW_HASH || dg_configUSE_HW_AES_HASH */

#if dg_configUSE_HW_AES_HASH

#define HW_AES_HASH_MAX_PAYLOAD_ENTRIES       (16)
#define HW_AES_HASH_NVM_USER_DATA_KEY_SIZE    (0x20)

/**
 * \brief       AES key sizes. Possible values: HW_AES_128, HW_AES_192, HW_AES_256.
 * \deprecated  This enum is deprecated, consider using HW_AES_KEY_SIZE instead.
 */
DEPRECATED_MSG("This enum is deprecated, consider using HW_AES_KEY_SIZE instead.")
typedef enum {
        HW_AES_128 = 0,
        HW_AES_192 = 1,
        HW_AES_256 = 2
} hw_aes_key_size;

/**
 * \brief       AES direction. Possible values: HW_AES_DECRYPT, HW_AES_ENCRYPT.
 * \deprecated  This enum is deprecated, consider using HW_AES_OPERATION instead.
 */
DEPRECATED_MSG("This enum is deprecated, consider using HW_AES_OPERATION instead.")
typedef enum {
        HW_AES_DECRYPT = 0,
        HW_AES_ENCRYPT = 1
} hw_aes_direction;

/**
 * \brief       AES/Hash modes
 * \deprecated  This enum is deprecated, consider using HW_AES_MODE and/or HW_HASH_TYPE instead.
 */
DEPRECATED_MSG("This enum is deprecated, consider using HW_AES_MODE and/or HW_HASH_TYPE instead.")
typedef enum {
        HW_AES_ECB,
        HW_AES_CBC,
        HW_AES_CTR,
        HW_HASH_MD5,
        HW_HASH_SHA_1,
        HW_HASH_SHA_256_224,
        HW_HASH_SHA_256,
        HW_HASH_SHA_384,
        HW_HASH_SHA_512,
        HW_HASH_SHA_512_224,
        HW_HASH_SHA_512_256
} hw_aes_hash_mode;

/**
 * \brief       Key expansion modes.
 *
 * Possible values HW_AES_PERFORM_KEY_EXPANSION, HW_AES_DO_NOT_PERFORM_KEY_EXPANSION
 *
 * \deprecated  This enum is deprecated, consider using HW_AES_KEY_EXPAND instead.
 */
DEPRECATED_MSG("This enum is deprecated, consider using HW_AES_KEY_EXPAND instead.")
typedef enum {
        HW_AES_PERFORM_KEY_EXPANSION = 0,       /**< Key expansion is performed by the engine */
        HW_AES_DO_NOT_PERFORM_KEY_EXPANSION     /**< Key expansion is performed by the software */
} hw_aes_hash_key_exp_t;

/**
 * \brief AES/Hash setup structure.
 *
 * \deprecated  This struct is deprecated, consider using hw_aes_config_t and/or hw_hash_config_t instead.
 */
DEPRECATED_MSG("This struct is deprecated, consider using hw_aes_config_t and/or hw_hash_config_t instead.")
typedef struct {
        hw_aes_hash_mode mode; /**< AES/Hash mode. */
        hw_aes_direction aesDirection; /**< AES direction. Only used when the mode is an AES mode.*/
        hw_aes_key_size aesKeySize; /**< AES key size. Only used when the mode is an AES mode. */
        bool aesKeyExpand; /**< When true the key expansion process is execute. When false the key
         expansion process is not executed. The user should write the AES keys in CRYPTO_RAM.
         Only used when the mode is an AES mode. */
        uint32 aesKeys; /**< The start address of the buffer containing the AES key. */
        uint32 aesIvCtrblk_0_31; /**< In CBC mode IV[31:0] and in CTR mode the initial value of the
         32 bits counter. Only used when the mode is an AES CBC/CTR mode. */
        uint32 aesIvCtrblk_32_63; /**< In CBC mode IV[63:32] and in CTR[63:32].
         Only used when the mode is an AES CBC/CTR mode. */
        uint32 aesIvCtrblk_64_95; /**< In CBC mode IV[95:64] and in CTR[95:64].
         Only used when the mode is an AES CBC/CTR mode. */
        uint32 aesIvCtrblk_96_127; /**< In CBC mode IV[127:96] and in CTR[127:96].
         Only used when the mode is an AES CBC/CTR mode. */
        bool aesWriteBackAll; /**< When true all the AES resulting data is written to memory.
         When false only the final block of the AES resulting data is written to memory.
         Only used when the mode is an AES mode. */
        uint8 hashOutLength; /**< The number of bytes of the hash result to be saved to memory.
         Only used when mode is a Hash mode. */
        bool moreDataToCome; /**< When false this is the last data block. When true more data is to
         come. */
        uint32 sourceAddress; /**< The physical address of the input data that needs to be processed. */
        uint32 destinationAddress; /**< The physical address (RAM only) where the resulting data needs
         to be written. If NULL the register is not written.*/
        uint32 dataSize; /**< The number of bytes that need to be processed. If this number is not
         a multiple of a block size, the data is automatically extended with zeros. */
        bool enableInterrupt; /**< When true the callback function is called after the operation
         has ended. */
        hw_aes_hash_cb callback; /**< The callback function that is called when enable interrupt
         is true. */
} hw_aes_hash_setup;

/**
 * \brief AES/Hash initialize.
 *
 * This function sets up an AES/HASH engine operation. All the configuration details
 * are included in the setup input structure. The operation can then start by calling
 * hw_aes_hash_start
 *
 * \param [in] setup The setup structure with setup values.
 *
 * \note There are some restrictions in the value of dataSize of the setup structure depending on
 *       the mode. This function will do appropriate checking using assertions. The following
 *       table shows what the value of dataSize should be:
 *
 * mode                | moreDataToCome = true | moreDataToCome = false
 * ------------------- | --------------------- | ----------------------
 * HW_AES_ECB          | multiple of 16        | multiple of 16
 * HW_AES_CBC          | multiple of 16        | no restrictions
 * HW_AES_CTR          | multiple of 16        | no restrictions
 * HW_HASH_MD5         | multiple of 8         | no restrictions
 * HW_HASH_SHA_1       | multiple of 8         | no restrictions
 * HW_HASH_SHA_256_224 | multiple of 8         | no restrictions
 * HW_HASH_SHA_256     | multiple of 8         | no restrictions
 * HW_HASH_SHA_384     | multiple of 8         | no restrictions
 * HW_HASH_SHA_512     | multiple of 8         | no restrictions
 * HW_HASH_SHA_512_224 | multiple of 8         | no restrictions
 * HW_HASH_SHA_512_256 | multiple of 8         | no restrictions
 *
 * \sa hw_aes_hash_start
 *
 * \deprecated  This function is deprecated, consider using hw_aes_init and/or hw_hash_init instead.
 *
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_init and/or hw_hash_init instead.")
void hw_aes_hash_init(hw_aes_hash_setup *setup);

/**
 * \brief AES/Hash restart.
 *
 * This function restarts the AES/Hash engine. This function can be used when the engine waits for
 * more input data.
 *
 * \param [in] sourceAddress The start address of the data that needs to be processed.
 * \param [in] dataSize The number of bytes that need to be processed. If this number is not
 * a multiple of a block size, the data is automatically extended with zeros.
 * \param [in] moreDataToCome When false this is the last data block. When true more data is to
 * come.
 *
 * \deprecated  This function is deprecated, consider using hw_aes_init and hw_aes_hash_start instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_init and hw_aes_hash_start instead.")
void hw_aes_hash_restart(const uint32 sourceAddress, const uint32 dataSize, const bool moreDataToCome);

/**
 * \brief AES/Hash disable.
 *
 * This function disables the AES/HASH engine and its interrupt request signal.
 *
 * \param [in] waitTillInactive When true the AES/HASH engine is disabled after any pending operation
 * finishes. When false the AES/Hash is disabled immediately.
 *
 * \deprecated  This function is deprecated, consider using hw_aes_hash_is_active,
 *              hw_aes_hash_disable_interrupt_source, hw_aes_hash_clear_interrupt_req and
 *              hw_aes_hash_disable_clock instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_hash_is_active, hw_aes_hash_disable_interrupt_source, hw_aes_hash_clear_interrupt_req and hw_aes_hash_disable_clock instead.")
void hw_aes_hash_disable(const bool waitTillInactive);

/**
 * \brief Mark next input block as being last
 *
 * This function is used to configure the engine so as to consider the next input block
 * as the last of the operation. When the operation finishes, the engine's status
 * becomes "inactive".
 *
 * \deprecated  This function is deprecated, consider using hw_aes_hash_set_input_data_mode instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_hash_set_input_data_mode instead.")
__STATIC_INLINE void hw_aes_hash_mark_input_block_as_last(void)
{
        REG_CLR_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_MORE_IN);
}

/**
 * \brief Mark next input block as not being last
 *
 * This function is used to configure the engine so as to expect more input blocks
 * after the operation. When the operation finishes, the engine's status
 * becomes "waiting for input".
 *
 * \deprecated  This function is deprecated, consider using hw_aes_hash_set_input_data_mode instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_hash_set_input_data_mode instead.")
__STATIC_INLINE void hw_aes_hash_mark_input_block_as_not_last(void)
{
        REG_SET_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_MORE_IN);
}

/**
 * \brief Configure engine for AES ECB encryption/decryption
 *
 * This function configures the engine for AES ECB encryption/decryption
 *
 * \param[in] key_size The size of the key used in the encryption/decryption
 *
 * \warning AES ECB is not recommended for use in cryptographic protocols.
 *
 * \deprecated This function is deprecated, consider using hw_aes_set_mode and hw_aes_set_key_size instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_set_mode and hw_aes_set_key_size instead.")
__STATIC_INLINE void hw_aes_hash_cfg_aes_ecb(hw_aes_key_size key_size)
{
        uint32 crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_AES_KEY_SZ, crypto_ctrl_reg, key_size);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief Configure engine for AES CTR encryption/decryption
 *
 * This function configures the engine for AES CTR encryption/decryption
 *
 * \param[in] key_size The size of the key used in the encryption/decryption
 *
 * \deprecated This function is deprecated, consider using hw_aes_set_mode and hw_aes_set_key_size instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_set_mode and hw_aes_set_key_size instead.")
__STATIC_INLINE void hw_aes_hash_cfg_aes_ctr(hw_aes_key_size key_size)
{
        uint32 crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, 2);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_AES_KEY_SZ, crypto_ctrl_reg, key_size);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief Configure engine for AES CBC encryption/decryption
 *
 * This function configures the engine for AES CBC encryption/decryption
 *
 * \param[in] key_size The size of the key used in the encryption/decryption
 *
 * \deprecated This function is deprecated, consider using hw_aes_set_mode and hw_aes_set_key_size instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_set_mode and hw_aes_set_key_size instead.")
__STATIC_INLINE void hw_aes_hash_cfg_aes_cbc(hw_aes_key_size key_size)
{
        uint32 crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, 3);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_AES_KEY_SZ, crypto_ctrl_reg, key_size);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief Configure engine for MD5 hash
 *
 * This function configures the engine to perform MD5 hashing
 *
 * \param[in] result_size The size in bytes of the result that the engine will write
 *                        to the output memory. Accepted values are 1 to 16. Out of
 *                        range values are adjusted to the closest limit.
 *
 * \deprecated This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_data_len instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_data_len instead.")
__STATIC_INLINE void hw_aes_hash_cfg_md5(unsigned int result_size)
{
        uint32_t crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_OUT_LEN, crypto_ctrl_reg,
                (result_size > 16)? 15: (result_size == 0)? 0 : (result_size - 1));
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, 0);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief Configure engine for SHA1 hash
 *
 * This function configures the engine to perform SHA1 hashing
 *
 * \param[in] result_size The size in bytes of the result that the engine will write
 *                        to the output memory. Accepted values are 1 to 20. Out of
 *                        range values are adjusted to the closest limit.
 *
 * \deprecated This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.")
__STATIC_INLINE void hw_aes_hash_cfg_sha1(unsigned int result_size)
{
        uint32_t crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_OUT_LEN, crypto_ctrl_reg,
                (result_size > 20)? 19: (result_size == 0)? 0 : (result_size - 1));
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, 1);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief Configure engine for SHA-224 hash
 *
 * This function configures the engine to perform SHA-224 hashing
 *
 * \param[in] result_size The size in bytes of the result that the engine will write
 *                        to the output memory. Accepted values are 1 to 28. Out of
 *                        range values are adjusted to the closest limit.
 *
 * \deprecated This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.")
__STATIC_INLINE void hw_aes_hash_cfg_sha_224(unsigned int result_size)
{
        uint32_t crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_OUT_LEN, crypto_ctrl_reg,
                (result_size > 28)? 27: (result_size == 0)? 0 : (result_size - 1));
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, 2);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief Configure engine for SHA-256 hash
 *
 * This function configures the engine to perform SHA-256 hashing
 *
 * \param[in] result_size The size in bytes of the result that the engine will write
 *                        to the output memory. Accepted values are 1 to 32. Out of
 *                        range values are adjusted to the closest limit.
 *
 * \deprecated This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.")
__STATIC_INLINE void hw_aes_hash_cfg_sha_256(unsigned int result_size)
{
        uint32_t crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_OUT_LEN, crypto_ctrl_reg,
                (result_size > 32)? 31: (result_size == 0)? 0 : (result_size - 1));
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, 3);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief Configure engine for SHA-384 hash
 *
 * This function configures the engine to perform SHA-384 hashing
 *
 * \param[in] result_size The size in bytes of the result that the engine will write
 *                        to the output memory. Accepted values are 1 to 48. Out of
 *                        range values are adjusted to the closest limit.
 *
 * \deprecated This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.")
__STATIC_INLINE void hw_aes_hash_cfg_sha_384(unsigned int result_size)
{
        uint32_t crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_OUT_LEN, crypto_ctrl_reg,
                (result_size > 48)? 47: (result_size == 0)? 0 : (result_size - 1));
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, 0);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief Configure engine for SHA-512 hash
 *
 * This function configures the engine to perform SHA-512 hashing
 *
 * \param[in] result_size The size in bytes of the result that the engine will write
 *                        to the output memory. Accepted values are 1 to 64. Out of
 *                        range values are adjusted to the closest limit.
 *
 * \deprecated This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.")
__STATIC_INLINE void hw_aes_hash_cfg_sha_512(unsigned int result_size)
{
        uint32_t crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_OUT_LEN, crypto_ctrl_reg,
                (result_size > 64)? 63: (result_size == 0)? 0 : (result_size - 1));
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, 1);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief Configure engine for SHA-512/224 hash
 *
 * This function configures the engine to perform SHA-512/224 hashing
 *
 * \param[in] result_size The size in bytes of the result that the engine will write
 *                        to the output memory. Accepted values are 1 to 28. Out of
 *                        range values are adjusted to the closest limit.
 *
 * \deprecated This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.")
__STATIC_INLINE void hw_aes_hash_cfg_sha_512_224(unsigned int result_size)
{
        uint32_t crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_OUT_LEN, crypto_ctrl_reg,
                (result_size > 28)? 27: (result_size == 0)? 0 : (result_size - 1));
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, 2);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief Configure engine for SHA-512/256 hash
 *
 * This function configures the engine to perform SHA-512/256 hashing
 *
 * \param[in] result_size The size in bytes of the result that the engine will write
 *                        to the output memory. Accepted values are 1 to 32. Out of
 *                        range values are adjusted to the closest limit.
 *
 * \deprecated This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_hash_set_type and hw_hash_set_output_len instead.")
__STATIC_INLINE void hw_aes_hash_cfg_sha_512_256(unsigned int result_size)
{
        uint32_t crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_OUT_LEN, crypto_ctrl_reg,
                (result_size > 32)? 31: (result_size == 0)? 0 : (result_size - 1));
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, 3);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief Store initialization vector in AES/HASH engine memory
 *
 * This function stores the initialization vector (IV) that is necessary for AES CBC mode.
 *
 * \sa hw_aes_store_ic
 *
 * \param[in] iv The address of the buffer containing the initialization vector
 *
 * \deprecated  This function is deprecated, consider using hw_aes_set_init_vector instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_set_init_vector instead.")
void hw_aes_hash_store_iv(const uint8 *iv);

/**
 * \brief Store counter initialization in AES/HASH engine memory
 *
 * This function stores the counter initialization that is necessary for AES CTR mode.
 *
 * \sa hw_aes_store_iv
 *
 * \param[in] ic The address of the buffer containing the counter initialization
 *
 * \deprecated  This function is deprecated, consider using hw_aes_set_init_vector instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_set_init_vector instead.")
void hw_aes_hash_store_ic(const uint8 *ic);

/**
 * \brief Check whether the Encryption Key has been revoked properly
 *
 * \param [in] idx OTP memory cell_offset where the key is located
 *
 * \return true if key is valid, otherwise false
 *
 * \deprecated  This function is deprecated, consider using hw_otpc_is_aes_key_revoked instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_otpc_is_aes_key_revoked instead.")
bool hw_aes_hash_is_key_revoked(uint8_t idx);

/**
 * \brief Get User Data Encryption Key memory address
 *
 * Returns address of User Data Encryption Key for given key index within the memory that the keys
 * are stored
 *
 * \param [in] idx key index
 *
 * \return key memory address or 0 if key has been revoked
 *
 * \deprecated  This function is deprecated, consider using hw_aes_get_key_address instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_otpc_get_aes_key_address instead.")
uint32_t hw_aes_hash_keys_address_get(uint8_t idx);

/**
 * \brief Loads the keys from NVM memory to crypto engine
 *
 * This function loads the encryption/decryption keys from NVM to crypto engine
 * and performs key expansion by using the crypto engine. Moreover, it checks whether the
 * Aes Key read protection is enabled and if this is the case it transfers the keys by using
 * Secure DMA Channel. Otherwise, it performs a direct NVM reading.
 * NVM stands for Non-Volatile Memory, e.g. OTP memory for DA1468X/DA1469X.
 *
 * \param[in] key_size The size of encryption/decryption key
 * \param[in] nvm_keys_addr The NVM address which contains the keys
 *
 * \deprecated  This function is deprecated, consider using hw_aes_load_keys instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_load_keys instead.")
void hw_aes_hash_nvm_keys_load(hw_aes_key_size key_size, const uint32 *nvm_keys_addr);

/**
 * \brief Loads the keys from given array to crypto engine
 *
 * This function loads the encryption/decryption keys from *aes_keys to crypto engine
 * and performs key expansion either by using the crypto engine or by software depending
 * on the state of key_exp.
 *
 * \param[in] key_size The size of encryption/decryption key
 * \param[in] aes_keys The array which contains the keys
 * \param[in] key_exp  Defines whether the key expansion will be performed by the engine
 *                     or the software
 *
 * \deprecated  This function is deprecated, consider using hw_aes_load_keys instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_load_keys instead.")
void hw_aes_hash_keys_load(hw_aes_key_size key_size, const uint8 *aes_keys,
                           hw_aes_hash_key_exp_t key_exp);

/**
 * \brief Configure DMA for data manipulation
 *
 * This function configures the DMA machine with the source and destination buffers.
 *
 * \param[in] src The physical address of the buffer containing the input data for the operation
 * \param[in] dst The physical address (RAM or Cache RAM only) of the buffer where the output of the
 *                operation will be stored. The dst address must be NULL when configuring the DMA
 *                while the engine is waiting for more input data.
 * \param[in] len The length of the input data
 *
 * \deprecated  This function is deprecated, consider using hw_aes_hash_set_input_data_addr,
 *              hw_aes_hash_set_output_data_addr and hw_aes_hash_set_input_data_len instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_hash_set_input_data_addr, hw_aes_hash_set_output_data_addr and hw_aes_hash_set_input_data_len instead.")
void hw_aes_hash_cfg_dma(const uint8 *src, uint8 *dst, unsigned int len);

/**
 * \brief Start AES encryption
 *
 * This function starts an AES encryption. AES mode, key and input/output data should be configured
 * before calling this function.
 *
 * \sa hw_aes_hash_init
 * \sa hw_aes_hash_cfg_aes_ecb
 * \sa hw_aes_hash_cfg_aes_ctr
 * \sa hw_aes_hash_cfg_aes_cbc
 * \sa hw_aes_hash_nvm_keys_load
 * \sa hw_aes_hash_cfg_dma
 *
 * \deprecated  This function is deprecated, consider using hw_aes_start_operation instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_start_operation instead.")
__STATIC_INLINE void hw_aes_hash_encrypt(void)
{
        REG_SET_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ENCDEC);

        AES_HASH->CRYPTO_START_REG = 1;
}

/**
 * \brief Start AES decryption
 *
 * This function starts an AES decryption. AES mode, key and input/output data should be configured
 * before calling this function.
 *
 * \sa hw_aes_hash_init
 * \sa hw_aes_hash_cfg_aes_ecb
 * \sa hw_aes_hash_cfg_aes_ctr
 * \sa hw_aes_hash_cfg_aes_cbc
 * \sa hw_aes_hash_nvm_keys_load
 * \sa hw_aes_hash_cfg_dma
 *
 * \deprecated  This function is deprecated, consider using hw_aes_start_operation instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_start_operation instead.")
__STATIC_INLINE void hw_aes_hash_decrypt(void)
{
        REG_CLR_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ENCDEC);

        AES_HASH->CRYPTO_START_REG = 1;
}

/**
 * \brief Set output mode to write all
 *
 * This function configures the AES/HASH engine to write back to memory all the
 * resulting data.
 *
 * \note Only applicable to AES operations.
 *
 * \deprecated  This function is deprecated, consider using hw_aes_set_output_data_mode instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_set_output_data_mode instead.")
__STATIC_INLINE void hw_aes_hash_output_mode_write_all(void)
{
        REG_CLR_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_OUT_MD);
}

/**
 * \brief Set output mode to write final
 *
 * This function configures the AES/HASH engine to write back to memory only the
 * the last block of the resulting data.
 *
 * \note Only applicable to AES operations.
 *
 * \deprecated  This function is deprecated, consider using hw_aes_set_output_data_mode instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_set_output_data_mode instead.")
__STATIC_INLINE void hw_aes_hash_output_mode_write_final(void)
{
        REG_SET_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_OUT_MD);
}

/**
 * \brief Checks if the aes key read protection is enabled
 *
 * \return true if the read protection is enabled otherwise false.
 *
 * \deprecated  This function is deprecated, consider using hw_dma_is_aes_key_protection_enabled instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_dma_is_aes_key_protection_enabled instead.")
__STATIC_INLINE bool hw_aes_hash_is_aes_key_read_protection_enabled(void)
{
        return (REG_GETF(CRG_TOP, SECURE_BOOT_REG, PROT_AES_KEY_READ));
}

/**
 * \brief Check input data size restriction
 *
 * This function checks the restrictions of input data length. It returns 0 if the restrictions are not
 * violated, -1 otherwise. It checks the configured values at the time it is called so it should be
 * used just before starting an operation. The function can be useful for debugging. The following
 * table summarizes the restrictions for the input data length.
 *
 * ALGORITHM           | NOT LAST DATA BLOCK   | LAST DATA BLOCK
 * ------------------- | --------------------- | ----------------
 * HW_AES_ECB          | multiple of 16        | multiple of 16
 * HW_AES_CBC          | multiple of 16        | no restrictions
 * HW_AES_CTR          | multiple of 16        | no restrictions
 * HW_HASH_MD5         | multiple of 8         | no restrictions
 * HW_HASH_SHA_1       | multiple of 8         | no restrictions
 * HW_HASH_SHA_256_224 | multiple of 8         | no restrictions
 * HW_HASH_SHA_256     | multiple of 8         | no restrictions
 * HW_HASH_SHA_384     | multiple of 8         | no restrictions
 * HW_HASH_SHA_512     | multiple of 8         | no restrictions
 * HW_HASH_SHA_512_224 | multiple of 8         | no restrictions
 * HW_HASH_SHA_512_256 | multiple of 8         | no restrictions
 *
 * \deprecated  This function is deprecated, consider using hw_aes_check_input_data_len_restrictions
 *              and/or hw_hash_check_input_data_len_restrictions instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_check_input_data_len_restrictions and/or hw_hash_check_input_data_len_restrictions instead.")
int hw_aes_hash_check_restrictions(void);

/**
 * \brief AES/Hash is waiting for more data.
 *
 * This function tells if the AES/Hash engine is waiting for more data or not.
 *
 * \return True if the AES/Hash engine is waiting more data and false when it is not.
 *
 * \deprecated  This function is deprecated, consider using hw_aes_hash_waiting_for_input_data instead.
 */
DEPRECATED_MSG("This function is deprecated, consider using hw_aes_hash_waiting_for_input_data instead.")
__STATIC_INLINE bool hw_aes_hash_wait_for_in(void)
{
        return REG_GETF(AES_HASH, CRYPTO_STATUS_REG, CRYPTO_WAIT_FOR_IN) == 1;
}

#endif /* dg_configUSE_HW_AES_HASH */


#endif /* HW_AES_HASH_H_ */
/**
 * \}
 * \}
 */
