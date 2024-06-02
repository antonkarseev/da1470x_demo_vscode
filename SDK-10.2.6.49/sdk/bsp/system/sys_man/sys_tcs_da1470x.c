/**
****************************************************************************************
*
* @file sys_tcs_da1470x.c
*
* @brief TCS Handler
*
* Copyright (C) 2020-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#include "sys_tcs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../peripherals/src/hw_sys_internal.h"

#if dg_configUSE_HW_GPADC
#include "hw_gpadc.h"
#endif

#ifdef CONFIG_USE_SNC
#include "snc.h"
#endif

#define CS_START_CMD            0xA5A5A5A5
#define CS_BOOTER_VAL           0xE6000000
#define CS_MIN_FW_VAL           0xEB000000
#define CS_SDK_VAL              0xE9000000
#define CS_STOP_CMD             0x00000000
#define CS_EMPTY_VAL            0xFFFFFFFF

#define CS_SDK_VAL_GID_MASK     0x000000FF
#define CS_SDK_VAL_LEN_MASK     0x0000FF00
#define CS_SDK_VAL_SETID_MASK   0x00FF0000

#define OTP_CS_ADDRESS          0x00000C00

#define CS_MAX_SIZE             (TCS_DATA_SIZE * 4) // 256 entries of 4 bytes

#define MAX_REG_ADDR            0x5100155C

#define CS_TESTPROGRAM_VERSION_REG_PAIR_ADJUSTMENTS_THRESHOLD   ( 0xF43D0 )
#define CS_TESTPROGRAM_VERSION_DIE_TEMP_CALIBRATION_THRESHOLD   ( 0xF4434 )
#define CS_DIE_TEMP_CALIBRATION_CORRECTED                       ( 0x9C4 )




#if defined(CONFIG_USE_SNC) || defined(CONFIG_USE_BLE)
#define SYS_TCS_RETAINED                __RETAINED_SHARED
#else
#define SYS_TCS_RETAINED                __RETAINED
#endif


#ifdef CONFIG_USE_SNC
/**
 * Shared environment with SNC
 */
typedef struct {
        volatile uint32_t *tcs_data;
        volatile uint32_t *tcs_data_size;
        volatile sys_tcs_attr_t *tcs_attributes;
} sys_tcs_shared_env_t;

#if (MAIN_PROCESSOR_BUILD)
__RETAINED_SHARED static sys_tcs_shared_env_t sys_tcs_shared_env;
#endif /* MAIN_PROCESSOR_BUILD */
#endif /* CONFIG_USE_SNC */

#if (MAIN_PROCESSOR_BUILD)
typedef struct {
        uint32_t reg_address;
        bool trimmed;
} reg_trimmed_t;

/* Static allocation for tcs_data */
SYS_TCS_RETAINED static uint32_t __tcs_data[TCS_DATA_SIZE];
SYS_TCS_RETAINED static uint32_t tcs_data_size;
#elif (SNC_PROCESSOR_BUILD)
__RETAINED static uint32_t tcs_data_size;
#endif /* PROCESSOR_BUILD */
__RETAINED static uint32_t *tcs_data;

#if (MAIN_PROCESSOR_BUILD)

#if (TEST_CS_IN_CONST_TABLE == 1)
static const uint32_t untrimmed_cs_info[] = {
        0xA5A5A5A5,
        0xE90001C0,
        0x00000009,
        0xE90002C1,
        0x00000000,
        0x00000000,
        0xE90001C2,
        0x00000000,
        0xE90001C3,
        0x00000000,
        0x50000050,
        0x00009020,
        0x50000044,
        0x001244B2,
        0x500000F8,
        0x00019834,
        0x5005042C,
        0x371DCD95,
        0xE9000C06,
        0x40003078,
        0x00C631B0,
        0x4000307C,
        0x00C631AE,
        0x40003064,
        0x22211D20,
        0x40003094,
        0x05050505,
        0x40003044,
        0x00020100,
        0x40003048,
        0x00020100,
        0xE9000A07,
        0x40003820,
        0x66660011,
        0x40003838,
        0x00442203,
        0x40003864,
        0x8F090F09,
        0x40003810,
        0x73737373,
        0x40003818,
        0x99944CDC,
        0xE9000280,
        0x4000304C,
        0x00000300,
        0xE9000282,
        0x4000304C,
        0x00000300,
        0xE9000244,
        0x0000CD04,
        0x0000CD04,
        0xE9000345,
        0x0000A97A,
        0x0000A97A,
        0x0000A97A,
        0xFFFFFFFF,
};
#endif

SYS_TCS_RETAINED static sys_tcs_attr_t tcs_attributes[SYS_TCS_GROUP_MAX];
        /*
                                                    // GID 0 is not handled
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SYS (0x01)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SNC (0x02)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_MEM (0x03)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_TMR (0x04)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_AUDIO (0x05)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD (0x06)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SYNTH (0x07)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_GPU (0x08)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_CTRL (0x09)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // unknown to SDK TCS group (0x0A)
        ...
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // unknown to SDK TCS group (0x1F)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_BD_ADDR (0x20)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // unknown to SDK TCS group (0x21)
        ...
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // unknown to SDK TCS group (0x3F)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_SD_ADC_SINGLE_MODE (0x40)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_SD_ADC_DIFF_MODE (0x41)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_GP_ADC_SINGLE_MODE (0x42)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_GP_ADC_DIFF_MODE (0x43)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_TEMP_SENS_25C (0x44)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_TEMP_SENS_RD_BG_CH_25C (0x45)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_BUCK_TRIM (0x46)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_RCHS_64MHZ (0x47)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_RCLP_32KHZ (0x48)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // unknown to SDK TCS group (0x49)
        ...
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // unknown to SDK TCS group (0x7F)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD_MODE1 (0x80)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SYNTH_MODE1 (0x81)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD_MODE2 (0x82)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SYNTH_MODE2 (0x83)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD_COEFF (0x84)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // unknown to SDK TCS group (0x85)
        ...
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // unknown to SDK TCS group (0xBF)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_CHIP_ID (0xC0)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_PROD_INFO (0xC1)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_PROD_WAFER (0xC2)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_TESTPROGRAM_VERSION (0xC3)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // unknown to SDK TCS group (0xC4)
        ...
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // unknown to SDK TCS group (0xDF)
                                                    // GIDs 0xE0 - 0xFF are not handled
        */

#elif (SNC_PROCESSOR_BUILD)
__RETAINED static sys_tcs_attr_t *tcs_attributes;
#endif /* PROCESSOR_BUILD */

static uint32_t get_testprogram_version_from_cs(void)
{
        uint32_t *values;
        uint8_t size;

        sys_tcs_get_custom_values(SYS_TCS_GROUP_TESTPROGRAM_VERSION, &values, &size);
        if (values != NULL) {
                /* Make sure that the TESTPROGRAM_VERSION TCS group comprises only one trim value. */
                ASSERT_WARNING(size == 1);
                return *values;
        } else {
                return 0;
        }
}

#if (MAIN_PROCESSOR_BUILD)

static void init_tcs_attributes_array()
{
        // below array contains the GID with type SYS_TCS_TYPE_REG_PAIR
        uint8_t reg_pair_gids[] = {
                0x00,                                   /* reserved GID (0x0) */
                SYS_TCS_GROUP_PD_SYS,                   /* SYS_TCS_GROUP_PD_SYS (0x01) */
                SYS_TCS_GROUP_PD_SNC,                   /* SYS_TCS_GROUP_PD_SNC (0x02) */
                SYS_TCS_GROUP_PD_MEM,                   /* SYS_TCS_GROUP_PD_MEM (0x03) */
                SYS_TCS_GROUP_PD_TMR,                   /* SYS_TCS_GROUP_PD_TMR (0x04) */
                SYS_TCS_GROUP_PD_AUDIO,                 /* SYS_TCS_GROUP_PD_AUDIO (0x05) */
                SYS_TCS_GROUP_PD_RAD,                   /* SYS_TCS_GROUP_PD_RAD (0x06) */
                SYS_TCS_GROUP_PD_SYNTH,                 /* SYS_TCS_GROUP_PD_SYNTH (0x07) */
                SYS_TCS_GROUP_PD_GPU,                   /* SYS_TCS_GROUP_PD_GPU (0x08) */
                SYS_TCS_GROUP_PD_CTRL,                  /* SYS_TCS_GROUP_PD_CTRL (0x09) */
                SYS_TCS_GROUP_SD_ADC_SINGLE_MODE,       /* SYS_TCS_GROUP_SD_ADC_SINGLE_MODE (0x40) */
                SYS_TCS_GROUP_SD_ADC_DIFF_MODE,         /* SYS_TCS_GROUP_SD_ADC_DIFF_MODE (0x41) */
                SYS_TCS_GROUP_PD_RAD_MODE1,             /* SYS_TCS_GROUP_PD_RAD_MODE1 (0x80) */
                SYS_TCS_GROUP_PD_SYNTH_MODE1,           /* SYS_TCS_GROUP_PD_SYNTH_MODE1 (0x81) */
                SYS_TCS_GROUP_PD_RAD_MODE2,             /* SYS_TCS_GROUP_PD_RAD_MODE2 (0x82) */
                SYS_TCS_GROUP_PD_SYNTH_MODE2,           /* SYS_TCS_GROUP_PD_SYNTH_MODE2 (0x83) */
        };

        uint32_t i = 0;
        for (i = 0; i < SYS_TCS_GROUP_MAX; i++) {
                //Initialize start to point to not configured GID
                tcs_attributes[i].start = GID_EMPTY;
        }

        for (i = 0; i < ARRAY_LENGTH(reg_pair_gids); i++) {
                  // configure register address value pair type.
                  tcs_attributes[reg_pair_gids[i]].value_type = SYS_TCS_TYPE_REG_PAIR;
        }
}

typedef enum {
        TCS_OTP = 0,                    /* CS in OTP */
        TCS_OQSPI,                      /* CS in Flash */
#if (TEST_CS_IN_CONST_TABLE == 1)
        TCS_IN_CONST_TABLE,
#endif
} SYS_TCS_SOURCE;

static uint32_t fetch_tcs_entry(SYS_TCS_SOURCE source, uint32_t address)
{
        uint32_t cs_value = CS_EMPTY_VAL;

        if (source == TCS_OTP) {
                cs_value = *(volatile uint32_t*)(MEMORY_OTP_BASE + OTP_CS_ADDRESS + address);
        } else if (source == TCS_OQSPI) {
                cs_value = *(volatile uint32_t*)(MEMORY_OQSPIC_S_BASE + address);
        }
#if (TEST_CS_IN_CONST_TABLE == 1)
        else if (source == TCS_IN_CONST_TABLE) {
                cs_value = *((uint32_t*)( (uint32_t)untrimmed_cs_info + address));
        }
#endif

        return cs_value;
}

static void store_tcs(uint32_t address, uint8_t gid_len, SYS_TCS_SOURCE source)
{
        int i = 0;
        uint16_t index;
        ASSERT_ERROR(tcs_data);

        //address --> GID header
        uint32_t value = fetch_tcs_entry(source, address);
        SYS_TCS_GID gid = (uint8_t)(value & CS_SDK_VAL_GID_MASK);

        if (gid >= SYS_TCS_GROUP_MAX) {
                return;
        }

        SYS_TCS_TYPE type = sys_tcs_get_value_type(gid);

        if (type == SYS_TCS_TYPE_TRIM_VAL) {
                //SYS_TCS_TYPE_TRIM_VAL could have different sizes during parsing
                //it is acceptable multiple instances with same GID to have different sizes,
                //only the newest will be finally stored. We cannot identify the newest but
                //at least the ones that fit will be stored until the newest is parsed
                if (gid_len != tcs_attributes[gid].size) {
                        return;
                }
        }

        /* start of storing TCS entries */

        index = tcs_attributes[gid].start;

        //for SYS_TCS_TYPE_REG_PAIR search tcs_data to find empty slot
        //for SYS_TCS_TYPE_TRIM_VAL fragmentation is not supported and always the newest entries are stored
        if (type == SYS_TCS_TYPE_REG_PAIR) {
                uint16_t gid_start = index;
                uint8_t gid_size = tcs_attributes[gid].size;

                //search tcs_data to find empty slot
                while (tcs_data[index] != 0) {
                        /* check if the index is inside the allocated space in tcs_data[] for this GID */
                        ASSERT_WARNING(index < (gid_start + gid_size));
                        if (index >= (gid_start + gid_size)) {
                                return;
                        }
                        //go to next register address
                        index += 2;
                }
        }

        while (i < gid_len) { //4 bytes entries
                address += 4;
                tcs_data[index] = fetch_tcs_entry(source, address);
                index++;
                i++;

        }
}

/* The calculated size in bytes, that is the number
 * of entries * sizeof(int)
 * returns the size in bytes
 */
static uint16_t get_size_of_cs(SYS_TCS_SOURCE source)
{
        uint32_t address = 0;
        uint16_t max_size = CS_MAX_SIZE;
        uint16_t size = 0;
        uint32_t value = fetch_tcs_entry(source, address);

        if (value != CS_START_CMD ) {
                return 0;
        }

        //check next entry
        address += 4;
        while (address < max_size) {
                value = fetch_tcs_entry(source, address);

                if ((value == CS_STOP_CMD) || (value == CS_EMPTY_VAL)) {
                        break; // End of CS
                } else if (value <= MAX_REG_ADDR) { //address value pair parsed by bootrom, skip this value
                        address += 0x4;
                } else if ((value == CS_BOOTER_VAL) || (value == CS_MIN_FW_VAL)) { //skip booter value and minimun FW version value
                         address += 0x4;
                } else if ((value & 0xFF000000) == CS_SDK_VAL) {  // SDK value
                        uint8_t tcs_len = (value & CS_SDK_VAL_LEN_MASK) >> 8;
                        SYS_TCS_GID gid = (uint8_t)(value & CS_SDK_VAL_GID_MASK);
                        uint8_t setid = (value & CS_SDK_VAL_SETID_MASK) >> 16;
                        address += tcs_len * 4; //skip next tcs values.

                        if (gid >= SYS_TCS_GROUP_MAX || setid != 0x00) {
                                address += 0x4; //skip this entry
                                continue;
                        }

                        if (sys_tcs_get_value_type(gid) == SYS_TCS_TYPE_TRIM_VAL) {
                                /*always keep the last found size */
                                if (tcs_attributes[gid].size != tcs_len) {
                                        /* update with new size */
                                        size -= 4 * tcs_attributes[gid].size;
                                        size += 4 * tcs_len;
                                        tcs_attributes[gid].size = tcs_len;
                                }
                        } else {
                                //check that SYS_TCS_TYPE_REG_PAIR values are of an even number
                                if (sys_tcs_get_value_type(gid) == SYS_TCS_TYPE_REG_PAIR) {
                                        ASSERT_ERROR((tcs_len & 0x01) == 0);
                                }

                                size += 4 * tcs_len; //size should be in bytes
                                tcs_attributes[gid].size += tcs_len;
                        }
                }
                // go to next word this value is not related to TCS
                address += 0x4;
        }
        return size;
}

static void store_cs_attributes(SYS_TCS_SOURCE source, uint16_t size)
{
        uint32_t address = 0x4; //skip CS_START_CMD

        while (address < size) {
                uint32_t value = fetch_tcs_entry(source, address);

                if ((value == CS_STOP_CMD) || (value == CS_EMPTY_VAL)) {
                        break; // End of CS
                } else if ((value == CS_BOOTER_VAL) || (value == CS_MIN_FW_VAL)) { //skip booter value and minimun FW version value
                        address += 0x4;
                } else if (value <= MAX_REG_ADDR) { // address - value pair
                        address += 0x4;
                } else if ((value & 0xFF000000) == CS_SDK_VAL) {  // SDK value
                        uint8_t gid_len = (value & CS_SDK_VAL_LEN_MASK) >> 8;
                        uint8_t setid = (value & CS_SDK_VAL_SETID_MASK) >> 16;
                        /* store entries with SET ID 0x00 */
                        if (setid == 0x00) {
                                store_tcs(address, gid_len, source);
                        }
                        address += gid_len*4; //skip next tcs values.
                }
                address += 0x4; //advance address by 4 bytes
        }
}

static bool parse_cs_for_booter_reg_pair(SYS_TCS_SOURCE source, uint8_t num, reg_trimmed_t *reg, uint32_t *reg_values, uint16_t block_sz);

static void adjust_booter_reg_pair_settings_on_top_of_CS(SYS_TCS_SOURCE source)
{
        if (hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_2798)) {
                uint32_t test_program_version = get_testprogram_version_from_cs();
                if ((test_program_version <= CS_TESTPROGRAM_VERSION_REG_PAIR_ADJUSTMENTS_THRESHOLD)
                        && (test_program_version != 0)) {
                        reg_trimmed_t reg = {
                                .reg_address = 0x5005042C,
                                .trimmed = false,
                        };
                        uint32_t reg_cs_value;
                        parse_cs_for_booter_reg_pair(source, 1, &reg ,
                                &reg_cs_value, CS_MAX_SIZE);
                        if (reg.trimmed) {
                                uint8_t low = RAW_GETF(&reg_cs_value, 0x3C000UL);
                                uint8_t high = RAW_GETF(&reg_cs_value, 0x3C0000UL);
                                RAW_SETF(&reg_cs_value, 0x3C000UL, high);
                                RAW_SETF(&reg_cs_value, 0x3C0000UL, low);
                                *((uint32_t *) reg.reg_address) = reg_cs_value;
                        }
                }
        }
}
#endif /* MAIN_PROCESSOR_BUILD */

void sys_tcs_get_trim_values_from_cs(void)
{
#if (MAIN_PROCESSOR_BUILD)
        uint32_t address = 0;
        uint16_t size = 0;
#if (TEST_CS_IN_CONST_TABLE == 1)
        SYS_TCS_SOURCE source = TCS_IN_CONST_TABLE;
#else
        SYS_TCS_SOURCE source = TCS_OTP;
#endif

#ifdef CONFIG_USE_SNC
        /* initialize SNC TCS shared space
         * In case there is no CS or no TCS entry in CS
         * tcs_data_size is zero and SNC will be aware that
         * there are no TCS entries.
         */
        sys_tcs_shared_env.tcs_data_size = &tcs_data_size;
        /* tcs_attributes is always available */
        sys_tcs_shared_env.tcs_attributes = tcs_attributes;
        snc_set_shared_space_addr(&sys_tcs_shared_env, SNC_SHARED_SPACE_SYS_TCS);
#endif

        #if (dg_configUSE_SYS_TCS == 0)
                return;
        #endif

        init_tcs_attributes_array();

        uint32_t value;
#if (TEST_CS_IN_CONST_TABLE == 1)
        //locate start of CS in test tcs ram
        value = fetch_tcs_entry(source, address);

        if (value != CS_START_CMD) {
                SYS_TCS_SOURCE source = TCS_OTP;
#endif
                value = fetch_tcs_entry(source, address);
                if (value != CS_START_CMD) {
                        //No start command found, try to locate CS in flash
                        source = TCS_OQSPI;
                        value = fetch_tcs_entry(source, address);
                        if (value != CS_START_CMD) {
#ifdef CONFIG_USE_BLE
                                // CS is mandatory for BLE projects.
                                ASSERT_WARNING(0);
#endif
                                return; // no CS found;
                        }
                }
#if (TEST_CS_IN_CONST_TABLE == 1)
        }
#endif

        address += 0x4;
        size = get_size_of_cs(source);

        /* The calculated size in bytes, taking into account that one entry is 4 bytes */
        ASSERT_ERROR(size < CS_MAX_SIZE);

        /* Static allocation for tcs_data */
        tcs_data = __tcs_data;

        //convert sizes to offsets in the tcs table and
        //set the sizes of the GID attributes
        uint8_t gid_offset = 0;
        for (uint16_t gid = 0; gid < SYS_TCS_GROUP_MAX; gid++) {
                if (tcs_attributes[gid].size != 0) {
                        tcs_attributes[gid].start = gid_offset;
                        gid_offset += tcs_attributes[gid].size;
                }
        }
        /* store the tcs_data used size*/
        tcs_data_size = gid_offset;
        store_cs_attributes(source, CS_MAX_SIZE);

        adjust_booter_reg_pair_settings_on_top_of_CS(source);

#ifdef CONFIG_USE_SNC
        /* CS is found send __tcs_data array to SNC. */
        sys_tcs_shared_env.tcs_data = __tcs_data;
#endif
#elif (SNC_PROCESSOR_BUILD)
        {
                sys_tcs_shared_env_t *shared_env = snc_get_shared_space_addr(SNC_SHARED_SPACE_SYS_TCS);

                tcs_data_size = *((uint32_t *)snc_convert_sys2snc_addr((void *)shared_env->tcs_data_size));
                if (tcs_data_size == 0) {
                        tcs_data = NULL;
                } else {
                        tcs_data = snc_convert_sys2snc_addr((void *)shared_env->tcs_data);
                }
                tcs_attributes = snc_convert_sys2snc_addr((void *)shared_env->tcs_attributes);
        }
#endif /* MAIN_PROCESSOR_BUILD */
}

#if (MAIN_PROCESSOR_BUILD)

static bool parse_cs_for_booter_reg_pair(SYS_TCS_SOURCE source, uint8_t num, reg_trimmed_t *reg, uint32_t *reg_values, uint16_t block_sz)
{
        uint32_t address = 0;
        uint32_t value = fetch_tcs_entry(source, address);

        if (value != CS_START_CMD) {
                /* search for start command to continue parsing*/
                return false;
        }

        while (address < block_sz) {
                if ((value == CS_STOP_CMD) || (value == CS_EMPTY_VAL)) {
                        break; // End of CS
                } else if ((value == CS_BOOTER_VAL) || (value == CS_MIN_FW_VAL)) { //skip booter value and minimun FW version value
                        address += 0x4;
                } else if (value <= MAX_REG_ADDR) { // address - value pair
                        address += 0x4;
                        for (uint8_t i = 0; i < num; i++) {
                                if (value == reg[i].reg_address) {
                                        reg[i].trimmed = true;
                                        if (reg_values != NULL) {
                                                reg_values[i] = fetch_tcs_entry(source, address);
                                        }
                                        break;
                                }
                        }
                } else if ((value & 0xFF000000) == CS_SDK_VAL) {  // SDK value
                        address += ((value & CS_SDK_VAL_LEN_MASK) >> 8) * 4; //skip next tcs values.
                }
                address += 0x4; //advance address by 4 bytes
                value = fetch_tcs_entry(source, address);
        }
        return true;
}

#endif /* MAIN_PROCESSOR_BUILD */

bool sys_tcs_reg_pairs_in_cs(const uint32_t *reg_address, uint8_t num, bool *trimmed_reg)
{
#if (MAIN_PROCESSOR_BUILD)
        uint8_t i = 0;
        bool ret = true;

        reg_trimmed_t regs[num];

        ASSERT_ERROR(reg_address);
        ASSERT_ERROR(trimmed_reg);

        if (num == 0) {
                return false;
        }

        for (i = 0; i < num; i++ ) {
                regs[i].reg_address = reg_address[i];
                regs[i].trimmed = trimmed_reg[i];
        }

        parse_cs_for_booter_reg_pair(TCS_OTP, num, regs, NULL, CS_MAX_SIZE);

        for (i = 0; i < num; i++ ) {
                trimmed_reg[i] = regs[i].trimmed;
                if (trimmed_reg[i] == false) {
                        ret = false;
                }
        }
        return ret;
#elif (SNC_PROCESSOR_BUILD)
        return true;
#endif /* PROCESSOR_BUILD */
}

__RETAINED_HOT_CODE void sys_tcs_apply_reg_pairs(SYS_TCS_GID gid)
{
        if (tcs_data == NULL) {
                return;
        }
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(tcs_attributes[gid].value_type == SYS_TCS_TYPE_REG_PAIR);

        uint8_t start = tcs_attributes[gid].start;
        int size = (int)tcs_attributes[gid].size;

        while (size > 0) {
                *(uint32_t *)tcs_data[start] = tcs_data[start+1];
                size -= 2;
                start += 2;
        }
}

uint32_t sys_tcs_snc_get_reg_pair_num_of_entries(SYS_TCS_GID gid)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(tcs_attributes[gid].value_type == SYS_TCS_TYPE_REG_PAIR);

        if (tcs_data == NULL) {
                // TCS is not initialized
                return 0;
        }

        return tcs_attributes[gid].size / 2;
}

__RETAINED_CODE sys_tcs_attr_t* sys_tcs_get_tcs_attributes_ptr(void)
{
        return tcs_attributes;
}

__RETAINED_CODE uint32_t* sys_tcs_get_tcs_data_ptr(void)
{
        return tcs_data;
}

uint32_t sys_tcs_get_tcs_data_size()
{
        return tcs_data_size;
}

void sys_tcs_custom_values_system_cb(SYS_TCS_GID gid, void *user_data, uint32_t *val, uint8_t len)
{
#if dg_configUSE_HW_GPADC
        uint16_t val_hi = (*val & 0xFFFF0000) >> 16;
        int16_t val_lo = *val & 0xFFFF;
        uint16_t adc_val;
        uint16_t ambient_temp;

        switch (gid) {
        case SYS_TCS_GROUP_GP_ADC_SINGLE_MODE:
                hw_gpadc_store_se_gain_error(hw_gpadc_calculate_single_ended_gain_error(val_lo, (int16_t)val_hi));
                hw_gpadc_store_se_offset_error(hw_gpadc_calculate_single_ended_offset_error(val_lo, (int16_t)val_hi));
                break;
        case SYS_TCS_GROUP_GP_ADC_DIFF_MODE:
                hw_gpadc_store_diff_gain_error(hw_gpadc_calculate_differential_gain_error(val_lo, (int16_t)val_hi));
                hw_gpadc_store_diff_offset_error(hw_gpadc_calculate_differential_offset_error(val_lo, (int16_t)val_hi));
                break;
        case SYS_TCS_GROUP_TEMP_SENS_25C:
                if (len == 1) {
                        adc_val = val_lo;
                        ambient_temp = val_hi;
                } else if (len == 2) {
                        adc_val = (uint16_t) *val;
                        ambient_temp = (uint16_t) *(val + 1);
                } else {
                        /* Should never reach this point */
                        break;
                }

                if (hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_2798) &&
                    get_testprogram_version_from_cs() <= CS_TESTPROGRAM_VERSION_DIE_TEMP_CALIBRATION_THRESHOLD) {
                        /* Overriding the OTP stored calibration point */
                        ambient_temp = CS_DIE_TEMP_CALIBRATION_CORRECTED;
                }

                hw_gpadc_store_ambient_calibration_point(adc_val, ambient_temp);
                break;
        default:
                break;
        }
#endif /* dg_configUSE_HW_GPADC */
}

#if (dg_configUSE_HW_GPADC == 1)
__WEAK bool hw_gpadc_get_trimmed_offsets_from_cs(uint8_t mode, uint16_t *offp, uint16_t *offn)
{
        uint32_t *values;
        uint8_t size;

        switch (mode) {
        case HW_GPADC_INPUT_MODE_SINGLE_ENDED:
                sys_tcs_get_custom_values(SYS_TCS_GROUP_GP_ADC_SINGLE_MODE, &values, &size);
                break;
        case HW_GPADC_INPUT_MODE_DIFFERENTIAL:
                sys_tcs_get_custom_values(SYS_TCS_GROUP_GP_ADC_DIFF_MODE, &values, &size);
                break;
        default:
                return false;
        }

        if (size != 2) {
                return false;
        }

        *offn = ((*(values + 1) & 0xFFFF0000) >> 16) & REG_MSK(GPADC, GP_ADC_OFFN_REG, GP_ADC_OFFN);
        *offp = ( *(values + 1) & 0xFFFF) & REG_MSK(GPADC, GP_ADC_OFFP_REG, GP_ADC_OFFP);
        return true;
}
#endif /* dg_configUSE_HW_GPADC */
