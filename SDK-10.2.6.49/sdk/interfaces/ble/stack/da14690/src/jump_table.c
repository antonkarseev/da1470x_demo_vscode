/**
 ****************************************************************************************
 *
 * @file jump_table.c
 *
 * @brief Heaps and configuration table setup.
 *
 * Copyright (C) 2017-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifdef CONFIG_USE_BLE
/*
 * INCLUDES
 ****************************************************************************************
 */
#include <string.h>
#include "co_version.h"
#include "ble_stack_config.h"
#include "sdk_defs.h"
#include "cmsis_compiler.h"
#include "co_version.h"
#include "rwip.h"
#include "arch.h"
#include "ble_stack_config_tables.h"

#define RWIP_HEAP_HEADER                (12 / sizeof(uint32_t))         // header size in uint32_t
#define RWIP_CALC_HEAP_LEN(len)         ((((len) + (sizeof(uint32_t) - 1)) / sizeof(uint32_t)) + RWIP_HEAP_HEADER)

#define SIZEOF_GAPC_ENV_TAG             (248 /* sizeof(struct gapc_env_tag)  */ + KE_HEAP_MEM_RESERVED)
#define SIZEOF_GATTC_ENV_TAG            (148 /* sizeof(struct gattc_env_tag) */ + KE_HEAP_MEM_RESERVED)
#define SIZEOF_L2CC_ENV_TAG             (28  /* sizeof(struct l2cc_env_tag)  */ + KE_HEAP_MEM_RESERVED)

#define RWIP_HEAP_ENV_SIZE_JT           ( (SIZEOF_GAPC_ENV_TAG + SIZEOF_GATTC_ENV_TAG + SIZEOF_L2CC_ENV_TAG) * \
                                           BLE_CONNECTION_MAX_USER)
#define RWIP_HEAP_DB_SIZE_JT            (dg_configBLE_STACK_DB_HEAP_SIZE)
#define RWIP_HEAP_MSG_SIZE_JT           (256 * (BLE_CONNECTION_MAX_USER + 1))
#define RWIP_HEAP_NON_RET_SIZE_JT       (1024)

uint32_t rwip_heap_env[RWIP_CALC_HEAP_LEN(RWIP_HEAP_ENV_SIZE_JT)];
uint32_t rwip_heap_db[RWIP_CALC_HEAP_LEN(RWIP_HEAP_DB_SIZE_JT)];
uint32_t rwip_heap_msg[RWIP_CALC_HEAP_LEN(RWIP_HEAP_MSG_SIZE_JT)];
uint32_t rwip_heap_non_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_NON_RET_SIZE_JT)];

const uint32_t rom_cfg_table[] =
{
        [rwip_heap_env_addr_pos]                        = (uint32_t) &rwip_heap_env[0],
        [rwip_heap_env_size_pos]                        = (uint32_t) RWIP_HEAP_ENV_SIZE_JT ,
        [rwip_heap_msg_addr_pos]                        = (uint32_t) &rwip_heap_msg[0],
        [rwip_heap_msg_size_pos]                        = (uint32_t) RWIP_HEAP_MSG_SIZE_JT ,
        [rwip_heap_non_ret_addr_pos]                    = (uint32_t) &rwip_heap_non_ret[0],
        [rwip_heap_non_ret_size_pos]                    = (uint32_t) RWIP_HEAP_NON_RET_SIZE_JT ,
        #if (BLE_HOST_PRESENT)
        [rwip_heap_db_addr_pos]                         = (uint32_t) &rwip_heap_db[0],
        [rwip_heap_db_size_pos]                         = (uint32_t) RWIP_HEAP_DB_SIZE_JT,
        #endif /* (BLE_HOST_PRESENT) */

        #if (BLE_EMB_PRESENT)
        [man_id_pos]                                    = 0x00D2, //  Dialog Semi Id
        [ea_timer_prog_delay_pos]                       = 1,
        [ea_clock_corr_lat_pos]                         = 2,
        [ea_be_used_dft_pos]                            = 2,
        [start_margin_pos]                              = 2,
        [test_mode_margin_pos]                          = 4,
        [bw_used_slave_dft_pos]                         = 3,
        [bw_used_adv_dft_pos]                           = 6,
        [rwble_prog_latency_dft_pos]                    = 1,
        [rwble_asap_latency_pos]                        = 2,
        [rwble_priority_adv_ldc_pos]                    = RWBLE_PRIORITY_ADV_LDC_DEFAULT,
        [rwble_priority_scan_pos]                       = RWBLE_PRIORITY_SCAN_DEFAULT,
        [rwble_priority_mconnect_pos]                   = RWBLE_PRIORITY_MCONNECT_DEFAULT,
        [rwble_priority_sconnect_pos]                   = RWBLE_PRIORITY_SCONNECT_DEFAULT,
        [rwble_priority_adv_hdc_pos]                    = RWBLE_PRIORITY_ADV_HDC_DEFAULT,
        [rwble_priority_init_pos]                       = RWBLE_PRIORITY_INIT_DEFAULT,
        [rwble_priority_max_pos]                        = RWBLE_PRIORITY_MAX_DEFAULT,
        [lld_evt_abort_cnt_duration_pos]                = 485,
        [ea_check_halfslot_boundary_pos]                = 624,
        [ea_check_slot_boundary_pos]                    = 614,
        [lld_rx_irq_thres_pos]                          = (BLE_RX_BUFFER_CNT / 2),
        [llm_adv_interval_min_noncon_disc_pos]          = LLM_ADV_INTERVAL_MIN,
        [hci_acl_data_packet_num_pos]                   = BLE_ACL_DATA_PACKET_NUM,
        [hci_acl_data_packet_size_pos]                  = BLE_ACL_DATA_PACKET_SIZE,
        [hci_lmp_ll_vers_pos]                           = RWBLE_SW_VERSION_MAJOR,
        [hci_vers_lmp_ll_subvers_pos]                   = CO_SUBVERSION_BUILD(RWBLE_SW_VERSION_MINOR, RWBLE_SW_VERSION_BUILD),
        #endif /* (BLE_EMB_PRESENT) */

        #if (BLE_HOST_PRESENT)
        #if (BLE_APP_PRESENT)
            [app_main_task_pos]                         = TASK_APP,
        #else
            #if (GTL_ITF)
            [app_main_task_pos]                         = TASK_GTL,
            #endif /* (GTL_ITF) */
        #endif /* (BLE_APP_PRESENT) */

        [gap_lecb_cnx_max_pos]                          = 10,
        [gapm_scan_filter_size_pos]                     = 10,
        [smpc_rep_attempts_timer_def_val_pos]           = 200,
        [smpc_rep_attempts_timer_max_val_pos]           = 3000,
        [smpc_rep_attempts_timer_mult_pos]              = 2,
        [smpc_timeout_timer_duration_pos]               = 3000,
        [att_trans_rtx_pos]                             = 0x0BB8,
        [att_sec_enc_key_size_pos]                      = 0x10,
        #endif /* (BLE_HOST_PRESENT) */

        #if ((BLE_HOST_PRESENT) || (BLE_EMB_PRESENT))
        [nb_links_user_pos]                             = BLE_CONNECTION_MAX_USER,
        #endif /* ((BLE_HOST_PRESENT) || (BLE_EMB_PRESENT)) */

        #if ((GTL_ITF) || (TL_ITF))
        [max_tl_pending_packets_adv_pos]                = 50,
        [max_tl_pending_packets_pos]                    = 60,
        #endif /* ((GTL_ITF) || (TL_ITF)) */
};

#endif /* CONFIG_USE_BLE */
