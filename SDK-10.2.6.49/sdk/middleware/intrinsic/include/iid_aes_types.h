/*! \copyright Copyright 2020 Intrinsic ID B.V. All rights reserved.\n
    This text contains proprietary, confidential information of Intrinsic ID B.V.,
    and may be used, copied, distributed and/or published only pursuant to the
    terms of a valid license agreement with Intrinsic ID B.V.\n
    This copyright notice must be retained as part of this text at all times.
*/

#ifndef _IID_AES_TYPES_H_
#define _IID_AES_TYPES_H_

typedef struct aes_handle_s aes_handle_t;
typedef aes_handle_t *aes_handle_h;

typedef struct aes_ctx_s aes_ctx_t;

/*! \brief      Function pointer type
 *  \details    Creates a type, named aes_acc_t, for a pointer to a function which does the AES cipher operation
 *              as described in FIPS 197. Note that this is equivalent to the AES ECB encryption operation as described
 *              in NIST SP800-38A. The function definition needs to match the following definition.
 *
 *  \param[in]      aes_acc_handle A handle to a structure to hold AES HW accelerator device context information.
 *
 *  \param[in]      *key A pointer to the buffer containing the AES key.
 *
 *  \param[in]      key_size The size in bytes of the key used for AES cipher operation. AES supports key sizes of 16, 24 and 32 bytes
 *
 *  \param[in]      *message_block The pointer to the buffer which holds the input data, with
 *                  a size in bytes equal to the standard defined AES block size of 16 bytes.
 *
 *  \param[out]     *data_out The pointer to a buffer with a size in bytes equal to the standard defined AES block size of
 *                  16 bytes, where output of the AES cipher operation will be stored.
 *
 *  \returns \ref   IID_SUCCESS if success, otherwise another return code.
 */

typedef iid_return_t (*aes_acc_t)(
        const  aes_handle_h         aes_acc_handle,
        const  uint8_t *   const    key,
        const  uint8_t              key_size,
        const  uint8_t *   const    message_block,
               uint8_t *   const    data_out);

/*! \brief      AES Context Structure
 *  \details    The structure aes_ctx_s holds two elements -
 *              aes_acc_handle: A handle to a device specific user defined aes accelerator environment.
 *              aes: A function pointer of type aes_acc_t, pointing to the function accessing and using the AES accelerator.
 *
 *              The AES device handle to be set by the integrator, if required by the AES accelerator implementation.
 */
struct aes_ctx_s {
       aes_handle_h     aes_acc_handle;
       aes_acc_t        aes;
};

#endif /* _IID_AES_TYPES_H_ */
