/**
 ****************************************************************************************
 *
 * @file gap_cfg.h
 *
 * @brief Header file - GAPCFG.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 * Copyright (c) 2015-2018 Modified by Dialog Semiconductor
 *
 *
 ****************************************************************************************
 */


#ifndef GAP_CFG_H_
#define GAP_CFG_H_

/**
 ****************************************************************************************
 * @addtogroup GAPCFG Settings
 * @ingroup GAP
 * @brief Contains GAP-related configurable values
 *
 * The GAPCFG is a header file that contains defined values necessary
 * for GAP operations particularly values for GAP modes. These values
 * are changeable in order to suit a particular application.
 *
 * @{
 ****************************************************************************************
 */

#include "co_version.h"
#include "ble_stack_config_tables.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// Maximum time to remain advertising when in the Limited
/// Discover able mode: TGAP(lim_adv_timeout)
/// required value: 180s: (18000 for ke timer)
#define GAP_TMR_LIM_ADV_TIMEOUT                 gap_cfg_table_ptr->GAP_TMR_LIM_ADV_TIMEOUT_VAR// 0x4650

/// Minimum time to perform scanning when performing
/// the General Discovery procedure: TGAP(gen_disc_scan_min)
/// recommended value: 10.24s: (1024 for ke timer)
#define GAP_TMR_GEN_DISC_SCAN                   gap_cfg_table_ptr->GAP_TMR_GEN_DISC_SCAN_VAR//0x0300

/// Minimum time to perform scanning when performing the
/// Limited Discovery procedure: TGAP(lim_disc_scan_min)
/// recommended value: 10.24s: (1024 for ke timer)
#define GAP_TMR_LIM_DISC_SCAN                   gap_cfg_table_ptr->GAP_TMR_LIM_DISC_SCAN_VAR//0x0300

/// Maximum time interval between private address change in seconds
/// TGAP(private_addr_int)
/// spec recommended value: 15 minutes; PTS GAP IXIT default is 5s
/// 0x384 is 15 minutes; 0x0E10 is 60 minutes and the maximum allowed value
#define GAP_TMR_PRIV_ADDR_INT                   gap_cfg_table_ptr->GAP_TMR_PRIV_ADDR_INT_VAR//0x384

/// Timer used in connection parameter update procedure
/// TGAP(conn_param_timeout)
/// recommended value: 30 s: (3000 for ke timer)
#define GAP_TMR_CONN_PARAM_TIMEOUT              gap_cfg_table_ptr->GAP_TMR_CONN_PARAM_TIMEOUT_VAR//0x0BB8

/// Timer used in LE credit based connection procedure
/// TGAP(lecb_conn_timeout)
/// recommended value: 30 s: (3000 for ke timer)
#define GAP_TMR_LECB_CONN_TIMEOUT               gap_cfg_table_ptr->GAP_TMR_LECB_CONN_TIMEOUT_VAR//0x0BB8

/// Timer used in LE credit based disconnection procedure
/// TGAP(lecb_disconn_timeout)
/// recommended value: 30 s: (3000 for ke timer)
#define GAP_TMR_LECB_DISCONN_TIMEOUT            gap_cfg_table_ptr->GAP_TMR_LECB_DISCONN_TIMEOUT_VAR//0x0BB8

/// Low energy mask
#define GAP_EVT_MASK                            {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x9F, 0x00,  0x20}

#define GAP_LE_EVT_4_0_MASK                     0x1F


/// Maximal authorized MTU value
#define GAP_MAX_LE_MTU                          gap_cfg_table_ptr->GAP_MAX_LE_MTU_VAR //(512)

/// Maximum GAP device name size
#define GAP_MAX_NAME_SIZE                       (0x20)
/// @} GAPCFG
#endif // GAP_CFG_H_
