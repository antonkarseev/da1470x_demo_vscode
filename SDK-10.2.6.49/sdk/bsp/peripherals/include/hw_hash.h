/**
 * \addtogroup PLA_DRI_CRYPTO
 * \{
 * \addtogroup HASH
 * \{
 * \brief HASH Engine LLD API
 */

/**
 ****************************************************************************************
 *
 * @file hw_hash.h
 *
 * @brief Definition of API for the HASH Engine Low Level Driver.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_HASH_H_
#define HW_HASH_H_

#if dg_configUSE_HW_HASH

#include "hw_aes_hash.h"

/**
 * \brief       HASH engine error codes
 */
typedef enum {
        HW_HASH_ERROR_INVALID_INPUT_DATA_LEN = -2,
        HW_HASH_ERROR_CRYPTO_ENGINE_LOCKED = -1,
        HW_HASH_ERROR_NONE = 0
} HW_HASH_ERROR;

/**
 * \brief       HASH Type
 */
typedef enum {
        HW_HASH_TYPE_MD5 = 0,           /**< HASH Type MD5 */
        HW_HASH_TYPE_SHA_1 = 1,         /**< HASH Type SHA-1 */
        HW_HASH_TYPE_SHA_256_224 = 2,   /**< HASH Type SHA-256/224 */
        HW_HASH_TYPE_SHA_256 = 3,       /**< HASH Type SHA-256 */
        HW_HASH_TYPE_SHA_384 = 4,       /**< HASH Type SHA-384 */
        HW_HASH_TYPE_SHA_512 = 5,       /**< HASH Type SHA-512 */
        HW_HASH_TYPE_SHA_512_224 = 6,   /**< HASH Type SHA-512/224 */
        HW_HASH_TYPE_SHA_512_256 = 7    /**< HASH Type SHA-512/256 */
} HW_HASH_TYPE;

/**
 * \brief       The maximum allowed output length of the HASH engine in bytes for all supported HASH types.
 */
typedef enum {
        HW_HASH_OUTPUT_LEN_MAX_MD5 = 16,                /**< The maximum allowed output length for HASH Type MD5 */
        HW_HASH_OUTPUT_LEN_MAX_SHA_1 = 20,              /**< The maximum allowed output length for HASH Type SHA-1 */
        HW_HASH_OUTPUT_LEN_MAX_SHA_256_224 = 28,        /**< The maximum allowed output length for HASH Type SHA-256/224 */
        HW_HASH_OUTPUT_LEN_MAX_SHA_256 = 32,            /**< The maximum allowed output length for HASH Type SHA-256 */
        HW_HASH_OUTPUT_LEN_MAX_SHA_384 = 48,            /**< The maximum allowed output length for HASH Type SHA-384 */
        HW_HASH_OUTPUT_LEN_MAX_SHA_512 = 64,            /**< The maximum allowed output length for HASH Type SHA-512 */
        HW_HASH_OUTPUT_LEN_MAX_SHA_512_224 = 28,        /**< The maximum allowed output length for HASH Type SHA-512/224 */
        HW_HASH_OUTPUT_LEN_MAX_SHA_512_256 = 32,        /**< The maximum allowed output length for HASH Type SHA-512/256 */
} HW_HASH_OUTPUT_LENGTH_MAX;

/**
 * \brief       HASH engine configuration structure.
 *
 * There are some restrictions in terms of the acceptable values of the data_len with regards to
 * Input Data Mode (wait_more_input) indicated by the next table:
 *
 * wait_more_input = true | wait_more_input = false |
 * ---------------------- | ----------------------- |
 * multiple of 8          | no restrictions         |
 *
 * Moreover, there are restriction with regards to the maximum length of the output data. Please
 * refer to enumerator HW_HASH_OUTPUT_LENGTH_MAX for more information.
 *
 * \sa          HW_HASH_TYPE
 * \sa          HW_HASH_OUTPUT_LENGTH_MAX
 * \sa          hw_aes_hash_cb
 */
typedef struct {
        HW_HASH_TYPE    type : 3;               /**< HASH type */
        bool            wait_more_input;        /**< HASH input data mode */
        uint32_t        input_data_len;         /**< Number of input bytes to be processed */
        uint32_t        output_data_len;        /**< HASH output length */
        uint32_t        input_data_addr;        /**< HASH input data address */
        uint32_t        output_data_addr;       /**< HASH output data address */
        hw_aes_hash_cb  callback;               /**< HASH callback function */
} hw_hash_config_t;

/**
 * \brief       Set HASH type
 *
 * \param [in]  hash_type HASH type
 *
 * \sa          HW_HASH_TYPE
 */
__STATIC_INLINE void hw_hash_set_type(HW_HASH_TYPE hash_type)
{
        ASSERT_WARNING((hash_type >= HW_HASH_TYPE_MD5) && (hash_type <= HW_HASH_TYPE_SHA_512_256));

        uint8_t crypto_alg = hash_type & 0x03;
        uint8_t crypto_alg_md = (hash_type & 0x04) >> 2;
        uint32_t crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 1);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, crypto_alg);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, crypto_alg_md);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief       Set HASH output length
 *
 * There are restriction with regards to the maximum length of the output data. If the selected
 * output_data_len exceeds the maximum allowed length determined by HW_HASH_OUTPUT_LEN_MAX this
 * function automatically equalizes the output length with the maximum allowed value.
 *
 * \param [in]  hash_type HASH type
 * \param [in]  output_data_len HASH output length
 *
 * \sa          HW_HASH_TYPE
 * \sa          HW_HASH_OUTPUT_LEN_MAX
 */
__STATIC_INLINE void hw_hash_set_output_data_len(HW_HASH_TYPE hash_type, uint8_t output_data_len)
{
        __RETAINED_CONST_INIT static const uint8_t out_len_max[] = {
                                                                HW_HASH_OUTPUT_LEN_MAX_MD5,
                                                                HW_HASH_OUTPUT_LEN_MAX_SHA_1,
                                                                HW_HASH_OUTPUT_LEN_MAX_SHA_256_224,
                                                                HW_HASH_OUTPUT_LEN_MAX_SHA_256,
                                                                HW_HASH_OUTPUT_LEN_MAX_SHA_384,
                                                                HW_HASH_OUTPUT_LEN_MAX_SHA_512,
                                                                HW_HASH_OUTPUT_LEN_MAX_SHA_512_224,
                                                                HW_HASH_OUTPUT_LEN_MAX_SHA_512_256 };

        if (output_data_len > out_len_max[hash_type]) {
                ASSERT_WARNING(0);
                output_data_len = out_len_max[hash_type];
        }

        REG_SETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_OUT_LEN, (output_data_len - 1));
}

/**
 * \brief       Check if the restrictions of the input data length are fulfilled
 *
 * There are some restrictions in terms of the acceptable values of the data_len with regards to
 * Input Data Mode, indicated by the next table:
 *
 * wait_more_input = true | wait_more_input = false |
 * ---------------------- | ----------------------- |
 * multiple of 8          | no restrictions         |
 *
 * \return      true if the restrictions are fulfilled, otherwise false
 */
bool hw_hash_check_input_data_len_restrictions(void);

/**
 * \brief       HASH engine initialization function.
 *
 * Configure the HASH engine provided that the crypto engine is NOT locked by the AES engine.
 * If the function returns HW_HASH_ERROR_NONE, the operation can be started by calling the
 * hw_aes_hash_start().
 *
 * \param [in]  hash_cfg Configuration structure for HASH engine.
 *
 * \return      HW_HASH_ERROR_NONE if the HASH engine has been successfully initialized,
 *              otherwise an error code.
 *
 * \warning     When HASHing has been completed, the hw_aes_hash_deinit() should be called in order
 *              for the crypto engine to be unlocked from HASH. This is mandatory in case that both
 *              AES and HASH are used by the same application. The two blocks make use of the same
 *              hardware accelerator, thus they are mutually exclusive and cannot be used simultaneously.
 *              The functions hw_aes_init(), hw_hash_init() and hw_aes_hash_deinit() incorporate a
 *              mechanism which ensures mutual exclusion and prevents race condition, provided that
 *              the user doesn't call the functions hw_aes_hash_disable_clock(), hw_aes_hash_enable_clock(),
 *              hw_aes_set_mode() and hw_hash_set_type(). The aforementioned functions affect some
 *              AES/HASH register fields which are used by this mechanism and might violate it.
 *              Therefore, it is highly recommended to use the corresponding init/deinit functions
 *              instead.
 *
 * \sa          hw_hash_config_t
 * \sa          HW_HASH_ERROR
 */
HW_HASH_ERROR hw_hash_init(const hw_hash_config_t *hash_cfg);

#endif /* dg_configUSE_HW_HASH */

#endif /* HW_HASH_H_ */
/**
 * \}
 * \}
 */
