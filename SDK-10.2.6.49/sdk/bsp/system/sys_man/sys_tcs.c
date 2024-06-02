/**
****************************************************************************************
*
* @file sys_tcs.c
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

#define TCS_DATA sys_tcs_get_tcs_data_ptr()
#define TCS_ATTRIBUTES sys_tcs_get_tcs_attributes_ptr()

uint8_t sys_tcs_get_size(SYS_TCS_GID gid)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        return TCS_ATTRIBUTES[gid].size;
}

SYS_TCS_TYPE sys_tcs_get_value_type(SYS_TCS_GID gid)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        return TCS_ATTRIBUTES[gid].value_type;
}

void sys_tcs_get_custom_values(SYS_TCS_GID gid, uint32_t **values, uint8_t *size)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(TCS_ATTRIBUTES[gid].value_type == SYS_TCS_TYPE_TRIM_VAL);

        if (TCS_DATA == NULL) {
                // TCS is not initialize
                return;
        }

        if (size == NULL ) {
                // size is mandatory
                return;
        }

        if (TCS_ATTRIBUTES[gid].start == GID_EMPTY) {
                *size = 0;
        } else {
                *size = TCS_ATTRIBUTES[gid].size;
        }

        if (values) {
                if (*size == 0) {
                        *values = NULL;
                } else {
                        /*if size is not zero then start is different than GID_EMPTY for this GID
                         * so CS parsing for TCS data is done and TCS_DATA is valid */
                        *values = &TCS_DATA[TCS_ATTRIBUTES[gid].start];
                }
        }
}

void sys_tcs_apply_custom_values(SYS_TCS_GID gid, sys_tcs_custom_values_cb cb, void *user_data)
{
        uint32_t* values = NULL;
        uint8_t size = 0;
        if (cb) {
                sys_tcs_get_custom_values(gid, &values, &size);
                if (size != 0) {
                        cb(gid, user_data, values, size);
                }
        }
}

void sys_tcs_get_reg_pairs(SYS_TCS_GID gid, uint32_t **values, uint8_t *size)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(TCS_ATTRIBUTES[gid].value_type == SYS_TCS_TYPE_REG_PAIR);

        if (TCS_DATA == NULL) {
                // TCS is not initialized
                return;
        }

        if (size == NULL ) {
                // size is mandatory
                return;
        }

        *size = TCS_ATTRIBUTES[gid].size;

        if (values) {
                if (*size == 0) {
                        *values = NULL;
                } else {
                        /*if size is not zero for register pair entry
                         CS parsing for TCS data is done and TCS_DATA is valid */
                        *values = &TCS_DATA[TCS_ATTRIBUTES[gid].start];
                }
        }
}

uint32_t *sys_tcs_snc_get_reg_pair(SYS_TCS_GID gid)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(TCS_ATTRIBUTES[gid].value_type == SYS_TCS_TYPE_REG_PAIR);

        if (TCS_DATA == NULL) {
                // TCS is not initialized
                return (uint32_t*)1; //NO valid address;
        }

        if (TCS_ATTRIBUTES[gid].start == GID_EMPTY) {
                /* If there are no valid TCS values SNC will be aware of it from the size*/
                return (uint32_t*)1; //NO valid address
        }
        return &TCS_DATA[TCS_ATTRIBUTES[gid].start];
}

