/**
****************************************************************************************
*
* @file sys_boot.c
*
* @brief System Boot Handler source file
*
* Copyright (C) 2021-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#if MAIN_PROCESSOR_BUILD

#include <string.h>
#include "sdk_defs.h"
#include "sys_boot.h"

/* Declaring the safe location of the booter status in uninitialized memory section
 * to avoid overwriting when CopyTable and ZeroTable are initialized.
 */
__RETAINED_UNINIT uint32_t secure_boot_status_report;

#define BOOTER_STATUS_SAFE_ADDR                 ((uint32_t *)&secure_boot_status_report)
#define BOOTER_STATUS_ADDR                      ((volatile uint32_t *) 0x20182064UL)
#define BOOTER_STATUS_ORIGIN_Msk                (0x0000FFFFUL)
#define BOOTER_STATUS_OTP_Msk                   (0xFFFF0000UL)
#define SYS_BOOTER_GETF(addr, mask)             ((*(uint32_t *)(addr) & (mask)) >> (__builtin_ctz(mask)))

#ifdef SYS_BOOT_EXPOSE_STATIC_FUNCTIONS
# define __STATIC__
#else
# define __STATIC__      static
#endif

void sys_boot_secure_copy_boot_result(void)
{
        *BOOTER_STATUS_SAFE_ADDR = *BOOTER_STATUS_ADDR;
}

__STATIC_INLINE uint16_t _sys_boot_get_previous_boot_origin(void)
{
        uint16_t val = SYS_BOOTER_GETF(BOOTER_STATUS_SAFE_ADDR, BOOTER_STATUS_ORIGIN_Msk);

        switch (val) {
        case SYS_BOOT_NO_BOOTING:
        case SYS_BOOT_UART_BOOT:
        case SYS_BOOT_OQSPI_ACTIVE:
        case SYS_BOOT_OQSPI_UPDATE:
                return val;
        default:
                return SYS_BOOT_ORIGIN_INVALID;
        }
}

__STATIC_INLINE uint16_t _sys_boot_get_previous_boot_otp_status(void)
{
        uint16_t val = SYS_BOOTER_GETF(BOOTER_STATUS_SAFE_ADDR, BOOTER_STATUS_OTP_Msk);

        switch (val) {
        case SYS_BOOT_OTP_PASS:
        case SYS_BOOT_OTP_FAIL:
                return val;
        default:
                return SYS_BOOT_OTP_INVALID;
        }
}

SYS_BOOT_ORIGIN sys_boot_get_previous_boot_origin(void)
{
        return (SYS_BOOT_ORIGIN)_sys_boot_get_previous_boot_origin();
}

SYS_BOOT_OTP_PRORAMMING_STATUS sys_boot_get_previous_boot_otp_status(void)
{
        return (SYS_BOOT_OTP_PRORAMMING_STATUS)_sys_boot_get_previous_boot_otp_status();
}
#endif /* MAIN_PROCESSOR_BUILD */

#if dg_configUSE_SYS_BOOT

#include "oqspi_automode.h"
#include "sdk_crc16.h"

// This union typedef is used to handle the bytes of a uint16_t more efficiently
typedef union {
        uint16_t var;
        uint8_t  arr[2];
} uint16_union_t;

typedef __PACKED_STRUCT {
        uint8_t identifier[2];          /**< Identifier (Pp) */
        uint8_t fw_img_active[4];       /**< Active Firmware image address */
        uint8_t fw_img_update[4];       /**< Update Firmware image address (if available,
                                             otherwise equal to Active Firmware image address) */
        uint8_t flash_burstcmda[4];     /**< BURSTCMDA register */
        uint8_t flash_burstcmdb[4];     /**< BURSTCMDB register */
        uint8_t flash_ctrlmode[4];      /**< CTRLMODE register */
        uint8_t type_flash_conf[2];     /**< Type of Flash Configuration */
        uint8_t flash_conf_len[2];      /**< Length of flash configuration  */
} product_header_fixed_t;

__STATIC__ uint16_t get_flash_conf_len(uint32_t product_header_addr)
{
        ASSERT_ERROR((product_header_addr == PRIMARY_PRODUCT_HEADER_BASE) ||
                     (product_header_addr == BACKUP_PRODUCT_HEADER_BASE));

        uint16_union_t flash_config_len;
        uint32_t flash_conf_len_addr = product_header_addr + offsetof(product_header_fixed_t, flash_conf_len);

        oqspi_automode_read(flash_conf_len_addr, flash_config_len.arr, sizeof(flash_config_len.arr));

        return (flash_config_len.var);
}

__STATIC__ uint32_t get_product_header_len(uint16_t flash_conf_len)
{
        uint32_t ph_len = 0;

        ph_len = sizeof(product_header_fixed_t) + flash_conf_len + sizeof(uint16_t);

        return (ph_len);
}

__RETAINED_CODE __STATIC__ void restore_product_header(uint32_t src, uint32_t dst, uint32_t len)
{
        uint8_t ph[len];
        size_t offset = 0;

        oqspi_automode_erase_flash_sector(dst);
        oqspi_automode_read(src, ph, len);

        while (offset < len) {
                offset += oqspi_automode_write_flash_page((dst + offset),
                                                          ((const uint8_t *) ph + offset),
                                                          (len - offset));
        }
}

__STATIC__ uint16_t crc16_read(uint32_t addr, uint16_t crc_offset)
{
        ASSERT_ERROR((addr != PRIMARY_PRODUCT_HEADER_BASE) || (addr != BACKUP_PRODUCT_HEADER_BASE));

        uint16_union_t ph_crc;

        oqspi_automode_read((addr + crc_offset), &ph_crc.arr[0], sizeof(ph_crc));

        return ph_crc.var;
}

__STATIC__ uint16_t crc16_calc(uint32_t addr, uint16_t crc_offset)
{
        uint8_t buf[crc_offset];
        uint16_t crc;

        oqspi_automode_read(addr, buf, crc_offset);
        crc = crc16_calculate(buf, crc_offset);

        return crc;
}

static bool equalize_image_pointers(uint32_t ph_len)
{
        SYS_BOOT_ORIGIN boot_origin = sys_boot_get_previous_boot_origin();
        uint32_t active = offsetof(product_header_fixed_t, fw_img_active);
        uint32_t update = offsetof(product_header_fixed_t, fw_img_update);
        uint8_t ph[ph_len];
        uint16_union_t crc;

        oqspi_automode_read(PRIMARY_PRODUCT_HEADER_BASE, ph, ph_len);

        // If the Active Image Address is not equal to Update Image Address
        if (memcmp((const uint8_t *) (ph + active), (const uint8_t *) (ph + update), 4)) {
                // Equalize the active image pointer with the update image pointer or vice
                // versa depending on the origin of the previous boot of the device.
                switch (boot_origin) {
                case SYS_BOOT_OQSPI_ACTIVE:
                        memcpy((uint8_t *) (ph + update), (const uint8_t *) (ph + active), 4);
                        break;
                case SYS_BOOT_OQSPI_UPDATE:
                        memcpy((uint8_t *) (ph + active), (const uint8_t *) (ph + update), 4);
                        break;
                default:
                        return false;
                }

                // Re-calculate the CRC of the new product header
                crc.var = crc16_calculate(ph, (ph_len - sizeof(crc)));
                ph[ph_len - 2] = crc.arr[0];
                ph[ph_len - 1] = crc.arr[1];

                // Update both Primary and Backup product header with ph[]
                oqspi_automode_erase_flash_sector(PRIMARY_PRODUCT_HEADER_BASE);
                oqspi_automode_write_flash_page(PRIMARY_PRODUCT_HEADER_BASE, ph, ph_len);

                oqspi_automode_erase_flash_sector(BACKUP_PRODUCT_HEADER_BASE);
                oqspi_automode_write_flash_page(BACKUP_PRODUCT_HEADER_BASE, ph, ph_len);
        }

        return true;
}

void sys_boot_restore_product_headers(void)
{
        bool pph_repaired = false;
        uint16_t flash_conf_len;
        uint16_t crc_offset;
        uint32_t ph_len;

        uint16_t pph_crc = 0xFFFF;
        uint16_t pph_crc_calc = 0xF5F5;

        flash_conf_len = get_flash_conf_len(PRIMARY_PRODUCT_HEADER_BASE);

        crc_offset = sizeof(product_header_fixed_t) + flash_conf_len;
        ph_len = get_product_header_len(flash_conf_len);

        // Read and calculate Primary Product Header's CRC
        pph_crc = crc16_read(PRIMARY_PRODUCT_HEADER_BASE, crc_offset);
        pph_crc_calc = crc16_calc(PRIMARY_PRODUCT_HEADER_BASE, crc_offset);

        // If the Primary Product Header is corrupted, repair it by copying the Backup Product Header
        // on it. Afterwards check again the CRC of the repaired Primary Product Header.
        while (pph_crc != pph_crc_calc) {
                // Use the Flash config length of the Backup Product Header
                flash_conf_len = get_flash_conf_len(BACKUP_PRODUCT_HEADER_BASE);
                crc_offset = sizeof(product_header_fixed_t) + flash_conf_len;
                ph_len = get_product_header_len(flash_conf_len);

                restore_product_header(BACKUP_PRODUCT_HEADER_BASE, PRIMARY_PRODUCT_HEADER_BASE, ph_len);

                pph_crc = crc16_read(PRIMARY_PRODUCT_HEADER_BASE, crc_offset);
                pph_crc_calc = crc16_calc(PRIMARY_PRODUCT_HEADER_BASE, crc_offset);
                pph_repaired = true;
        }

        if (!pph_repaired) {
                uint16_t bph_crc = 0xFFFF;
                uint16_t bph_crc_calc = 0xF5F5;

                // Read and calculate Backup Product Header's CRC
                bph_crc = crc16_read(BACKUP_PRODUCT_HEADER_BASE, crc_offset);
                bph_crc_calc = crc16_calc(BACKUP_PRODUCT_HEADER_BASE, crc_offset);

                // If the Backup Product Header is corrupted or NOT equal to the CRC of the Primary
                // product header, repair it by copying the Primary Product Header on it. Afterwards
                // check again the CRC of the repaired Backup Product Header.
                while (bph_crc != bph_crc_calc || bph_crc != pph_crc) {
                        restore_product_header(PRIMARY_PRODUCT_HEADER_BASE,
                                               BACKUP_PRODUCT_HEADER_BASE, ph_len);

                        bph_crc = crc16_read(BACKUP_PRODUCT_HEADER_BASE, crc_offset);
                        bph_crc_calc = crc16_calc(BACKUP_PRODUCT_HEADER_BASE, crc_offset);
                }
        }

        ASSERT_WARNING(equalize_image_pointers(ph_len));
}

#endif /* dg_configUSE_SYS_BOOT */
