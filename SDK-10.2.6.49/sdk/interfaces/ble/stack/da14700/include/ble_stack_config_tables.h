/**
 ****************************************************************************************
 *
 * @file ble_stack_config_tables.h
 *
 * @brief BLE stack config tables header file.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef _BLE_STACK_CONFIG_TABLES_H
#define _BLE_STACK_CONFIG_TABLES_H

#include "rwip_config.h"
#include <stdlib.h>
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>   // boolean definition

/* --------------------------------------------------------------------------------------------------------------------
 * Variable hooks
 --------------------------------------------------------------------------------------------------------------------*/
enum rom_cfg_var_pos
{
        rwip_heap_env_addr_pos,
        rwip_heap_env_size_pos,
        rwip_heap_msg_addr_pos,
        rwip_heap_msg_size_pos,
        rwip_heap_non_ret_addr_pos,
        rwip_heap_non_ret_size_pos,
#if (BLE_HOST_PRESENT)
        rwip_heap_db_addr_pos,
        rwip_heap_db_size_pos,
#endif /* (BLE_HOST_PRESENT) */

#if (BLE_EMB_PRESENT)
        man_id_pos,                             // Manufacturer Id
        ea_timer_prog_delay_pos,                //
        ea_clock_corr_lat_pos,                  //
        ea_be_used_dft_pos,                     // EA minimum reserved bandwidth per connection
        start_margin_pos,                       //
        test_mode_margin_pos,                   //
        bw_used_slave_dft_pos,                  // Minimum allowed value is 3 slots
        bw_used_adv_dft_pos,                    //
        rwble_prog_latency_dft_pos,             //
        rwble_asap_latency_pos,                 //
        rwble_priority_adv_ldc_pos,             //
        rwble_priority_scan_pos,                //
        rwble_priority_mconnect_pos,            //
        rwble_priority_sconnect_pos,            //
        rwble_priority_adv_hdc_pos,             //
        rwble_priority_init_pos,                //
        rwble_priority_max_pos,                 //
        lld_evt_abort_cnt_duration_pos,         //
        ea_check_halfslot_boundary_pos,         //
        ea_check_slot_boundary_pos,             //
        lld_rx_irq_thres_pos,                   //
        llm_adv_interval_min_noncon_disc_pos,   //
        hci_acl_data_packet_num_pos,            // Number of HCI ACL Tx buffers (legacy BLE_TX_DESC_DATA)
        hci_acl_data_packet_size_pos,           // Size of each HCI ACL Tx buffer (legacy LE_LENGTH_EXT_SUPPORTED_MAXTXOCTETS)
        hci_lmp_ll_vers_pos,                    // HCI version, LMP version, LL version, as per specification
        hci_lmp_ll_subversion_pos,              // HCI, LMP and LL subversion (Implementation defined)
#endif /* (BLE_EMB_PRESENT) */

#if (BLE_HOST_PRESENT)
        app_main_task_pos,
        gap_lecb_cnx_max_pos,                   //
        gapm_scan_filter_size_pos,              //
        smpc_rep_attempts_timer_def_val_pos,    //
        smpc_rep_attempts_timer_max_val_pos,    //
        smpc_rep_attempts_timer_mult_pos,       //
        smpc_timeout_timer_duration_pos,        //
        att_trans_rtx_pos,                      //
        att_sec_enc_key_size_pos,               //
#endif /* (BLE_HOST_PRESENT) */

#if (BLE_HOST_PRESENT) || (BLE_EMB_PRESENT)
        nb_links_user_pos,                      //
#endif /* ((BLE_HOST_PRESENT) || (BLE_EMB_PRESENT)) */

#if ((GTL_ITF) || (TL_ITF))
        max_tl_pending_packets_adv_pos,         //
        max_tl_pending_packets_pos,             //
#endif /* (GTL_ITF) */
};

#define get_cfg_setting(x)  ((rom_cfg_table_ptr != NULL) ? rom_cfg_table_ptr[x] : 0)

/* ROM build usage (dg_cfgCMAC_ROM == 1):
 * - The <rom_cfg_table_ptr> variable is defined (but not initialized) in <ble_stack_config_tables.c>, which is part of the ROM code.
 * - The code that uses the ROM code (RAM-build), should initialize <rom_cfg_table_ptr> by defining the BLE-stack configuration table
 *   locally and assign the address of that table to the ROM variable <rom_cfg_table_ptr>:

 * Non-ROM build usage (dg_cfgCMAC_ROM == 0):
 *  - The <rom_cfg_table_ptr> variable is defined in <ble_stack_config_tables.c> and points to a default BLE-stack configuration table,
 *     named <rom_cfg_table>, which is defined as a WEAK symbol.
 *  - The <rom_cfg_table> can be overridden by a specific platform (if needed) by providing a new definition for rom_cfg_table[].
 */
extern uint32_t *rom_cfg_table_ptr;

#if BLE_HOST_PRESENT
typedef struct {
    uint16_t GAP_TMR_LIM_ADV_TIMEOUT_VAR;
    uint16_t GAP_TMR_GEN_DISC_SCAN_VAR;
    uint16_t GAP_TMR_LIM_DISC_SCAN_VAR;
    uint16_t GAP_TMR_PRIV_ADDR_INT_VAR; /// cannot be greater than GAP_TMR_PRIV_ADDR_INT_MAX
    uint16_t GAP_TMR_CONN_PARAM_TIMEOUT_VAR;
    uint16_t GAP_TMR_LECB_CONN_TIMEOUT_VAR;
    uint16_t GAP_TMR_LECB_DISCONN_TIMEOUT_VAR;
    uint16_t GAP_MAX_LE_MTU_VAR;
} gap_cfg_table_t;

/* ROM build usage (dg_cfgCMAC_ROM == 1):
 * - The <gap_cfg_table_ptr> variable is defined (but not initialized) in <ble_stack_config_tables.c>, which is part of the ROM code.
 * - The code that uses the ROM code (RAM-build), should initialize <gap_cfg_table_ptr> by defining the GAP configuration table
 *   locally and assign the address of that table to the ROM variable <gap_cfg_table_ptr>:

 * Non-ROM build usage (dg_cfgCMAC_ROM == 0):
 *  - The <gap_cfg_table_ptr> variable is defined in <ble_stack_config_tables.c> and points to a default GAP configuration table,
 *     named <gap_cfg_table>, which is defined as a WEAK symbol.
 *  - The <gap_cfg_table> can be overridden by a specific platform (if needed) by providing a new definition for gap_cfg_table[].
 */
extern gap_cfg_table_t *gap_cfg_table_ptr;
#endif // BLE_HOST_PRESENT

#endif // _BLE_STACK_CONFIG_TABLES_H
