/**
 ****************************************************************************************
 *
 * @file attm_cfg.h
 *
 * @brief Header file - ATTMCFG.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 * Copyright (c) 2015-2018 Modified by Dialog Semiconductor
 *
 *
 ****************************************************************************************
 */


#ifndef ATTM_CFG_H_
#define ATTM_CFG_H_

/**
 ****************************************************************************************
 * @addtogroup ATTMCFG Settings
 * @ingroup ATTM
 * @brief ATTM Configuration file
 *
 * The ATTMCFG is the attribute configuration holder for @ref ATTM "ATTM".
 *
 * @{
 ****************************************************************************************
 */
 
/*
 * INCLUDES
 ****************************************************************************************
 */
 
#include "rwip_config.h"
#include "gap_cfg.h"

/*
 * DEFINES
 ****************************************************************************************
 */
 
/// Maximum Transmission Unit
#define ATT_DEFAULT_MTU                                 (23)

/// 30 seconds transaction timer
#define ATT_TRANS_RTX                                   get_cfg_setting(att_trans_rtx_pos)
/// Acceptable encryption key size - strict access
#define ATT_SEC_ENC_KEY_SIZE                            get_cfg_setting(att_sec_enc_key_size_pos)

/// Maximum attribute value length
#define ATT_MAX_VALUE                                   (GAP_MAX_LE_MTU)

/// Macro used to convert CPU integer define to LSB first 16-bits UUID
#define ATT_UUID_16(uuid)                               (uuid)
/// @} ATTMCFG

#endif // ATTM_CFG_H_
