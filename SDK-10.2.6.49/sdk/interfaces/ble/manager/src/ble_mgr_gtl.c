/**
 ****************************************************************************************
 *
 * @file ble_mgr_gtl.c
 *
 * @brief Helper library for GTL handling in BLE Manager
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include "osal.h"
#include "co_version.h"
#include "ble_stack_config.h"

#include "ble_mgr.h"
#include "ble_mgr_config.h"
#include "ble_mgr_gtl.h"
#include "ble_mgr_common.h"
#include "ble_mgr_gap.h"
#include "ble_mgr_gatts.h"
#include "ble_mgr_gattc.h"
#include "ble_mgr_l2cap.h"

#include "gapc_task.h"
#include "gapm_task.h"
#include "gattc_task.h"
#include "l2cc_task.h"

#define WAITQUEUE_MAXLEN (5)

typedef struct {
        uint16_t                conn_idx;
        uint16_t                msg_id;
        uint16_t                ext_id;
        ble_gtl_waitqueue_cb_t  cb;
        void                    *param;
} waitqueue_element_t;

__RETAINED static struct {
        waitqueue_element_t     queue[WAITQUEUE_MAXLEN];
        uint8_t                 len;
} waitqueue;

void *ble_hci_alloc(uint8_t hci_msg_type, uint16_t len)
{
        ble_mgr_common_stack_msg_t *blemsg = NULL;

        if ((hci_msg_type > 0) && (hci_msg_type <= BLE_HCI_EVT_MSG)) {
                blemsg = OS_MALLOC(sizeof(ble_mgr_common_stack_msg_t) + len);
        }
        else {
                goto done;
        }

        blemsg->hdr.op_code = BLE_MGR_COMMON_STACK_MSG;
        blemsg->msg_type = hci_msg_type;
        switch (hci_msg_type) {
        case BLE_HCI_CMD_MSG:
                blemsg->hdr.msg_len = HCI_CMD_HEADER_LENGTH + len;
                break;
        case BLE_HCI_ACL_MSG:
                blemsg->hdr.msg_len = HCI_ACL_HEADER_LENGTH + len;
                break;
        case BLE_HCI_SCO_MSG:
                blemsg->hdr.msg_len = HCI_SCO_HEADER_LENGTH + len;
                break;
        case BLE_HCI_EVT_MSG:
                blemsg->hdr.msg_len = HCI_EVT_HEADER_LENGTH + len;
                break;
        }

        memset(blemsg->msg.gtl.param, 0, len);
done:
        return blemsg;
}

void *ble_gtl_alloc(uint16_t msg_id, uint16_t dest_id, uint16_t len)
{
        ble_mgr_common_stack_msg_t *blemsg = OS_MALLOC(sizeof(ble_mgr_common_stack_msg_t) + len);

        blemsg->hdr.op_code = BLE_MGR_COMMON_STACK_MSG;
        blemsg->msg_type = BLE_GTL_MSG;
        blemsg->hdr.msg_len = GTL_MSG_HEADER_LENGTH + len;
        blemsg->msg.gtl.msg_id = msg_id;
        blemsg->msg.gtl.dest_id = dest_id;
        blemsg->msg.gtl.src_id = TASK_ID_GTL;
        blemsg->msg.gtl.param_length = len;

        memset(blemsg->msg.gtl.param, 0, len);

        return blemsg;
}

void ble_gtl_waitqueue_add(uint16_t conn_idx, uint16_t msg_id, uint16_t ext_id,
                                                             ble_gtl_waitqueue_cb_t cb, void *param)
{
        waitqueue_element_t *elem;

#if (BLE_MGR_DIRECT_ACCESS == 1)
        /* Acquire the waitqueue. */
        ble_mgr_waitqueue_acquire();
#endif /* (BLE_MGR_DIRECT_ACCESS == 1) */

        /* There should be still room in the queue before calling this function */
        OS_ASSERT(waitqueue.len < WAITQUEUE_MAXLEN);

        elem = &waitqueue.queue[waitqueue.len++];

        elem->conn_idx = conn_idx;
        elem->msg_id = msg_id;
        elem->ext_id = ext_id;
        elem->cb = cb;
        elem->param = param;

#if (BLE_MGR_DIRECT_ACCESS == 1)
        /* Release the waitqueue. */
        ble_mgr_waitqueue_release();
#endif /* (BLE_MGR_DIRECT_ACCESS == 1) */
}

bool ble_gtl_waitqueue_match(ble_gtl_msg_t *gtl)
{
        uint8_t idx;
        bool ret = false;

#if (BLE_MGR_DIRECT_ACCESS == 1)
        /* Acquire the waitqueue. */
        ble_mgr_waitqueue_acquire();
#endif /* (BLE_MGR_DIRECT_ACCESS == 1) */

        for (idx = 0; idx < waitqueue.len; idx++) {
                bool match = true;
                waitqueue_element_t *elem = &waitqueue.queue[idx];

                /* Connection index is not taken into account */
                if (elem->conn_idx == BLE_CONN_IDX_INVALID) {
                        match = (elem->msg_id == gtl->msg_id);
                } else {
                        match = (elem->conn_idx == TASK_2_CONNIDX(gtl->src_id)) &&
                                                                (elem->msg_id == gtl->msg_id);
                }

                if (!match) {
                        continue;
                }

                switch (elem->msg_id) {
                case GAPM_CMP_EVT:
                {
                        struct gapm_cmp_evt *evt = (void *) gtl->param;
                        match = (evt->operation == elem->ext_id);
                        break;
                }
                case GAPC_CMP_EVT:
                {
                        struct gapc_cmp_evt *evt = (void *) gtl->param;
                        match = (evt->operation == elem->ext_id);
                        break;
                }
                /* Add more events if other commands need more fine-grained matching */
                }

                if (match) {
                        ble_gtl_waitqueue_cb_t cb = elem->cb;
                        void *param = elem->param;

                        /* Remove from queue by moving remaining elements up in queue */
                        waitqueue.len--;
                        memmove(elem, elem + 1, sizeof(waitqueue_element_t) * (waitqueue.len - idx));

                        /* Fire associated callback */
                        cb(gtl, param);

                        ret = true;

                        break;
                }
        }

#if (BLE_MGR_DIRECT_ACCESS == 1)
        /* Release the waitqueue. */
        ble_mgr_waitqueue_release();
#endif /* (BLE_MGR_DIRECT_ACCESS == 1) */

        return ret;
}

void ble_gtl_waitqueue_flush(uint16_t conn_idx)
{
        uint8_t idx;

#if (BLE_MGR_DIRECT_ACCESS == 1)
        /* Acquire the waitqueue. */
        ble_mgr_waitqueue_acquire();
#endif /* (BLE_MGR_DIRECT_ACCESS == 1) */

        for (idx = 0; idx < waitqueue.len; idx++) {
                bool match = false;
                waitqueue_element_t *elem = &waitqueue.queue[idx];

                match = (elem->conn_idx == conn_idx);
                if (!match) {
                        continue;
                }

                switch (elem->msg_id) {
                case GAPC_CMP_EVT:
                {
                        switch (elem->ext_id) {
                        case GAPC_GET_CON_RSSI:
                        case GAPC_ENCRYPT:
#if (dg_configBLE_2MBIT_PHY == 1)
                        case GAPC_LE_SET_PHY:
#endif /* (dg_configBLE_2MBIT_PHY == 1) */

                        case GAPC_SET_SKIP_LATENCY:
                                match = true;
                                break;
                        case GAPC_LE_RD_REM_TX_PWR_LVL:
                                match = true;
                                break;
                        case GAPC_LE_SET_PATH_LOSS_REPORT_PARAMS:
                                match = true;
                                break;
                        case GAPC_LE_SET_PATH_LOSS_REPORT_EN:
                                match = true;
                                break;
                        case GAPC_LE_SET_TX_PWR_REPORT_EN:
                                match = true;
                                break;
                        default:
                                match = false;
                                break;
                        }
                }
                break;
                default:
                        match = false;
                        break;
                }

                if (match) {
                        ble_gtl_waitqueue_cb_t cb = elem->cb;
                        void *param = elem->param;

                        /* Remove from queue by moving remaining elements up in queue */
                        waitqueue.len--;
                        memmove(elem, elem + 1, sizeof(waitqueue_element_t) * (waitqueue.len - idx));

                        /* Fire associated callback with NULL gtl pointer */
                        cb(NULL, param);
                }
        }

#if (BLE_MGR_DIRECT_ACCESS == 1)
        /* Release the waitqueue. */
        ble_mgr_waitqueue_release();
#endif /* (BLE_MGR_DIRECT_ACCESS == 1) */
}

void ble_gtl_waitqueue_flush_all(void)
{
        uint8_t idx;
        uint8_t waitqueue_len = waitqueue.len;

#if (BLE_MGR_DIRECT_ACCESS == 1)
        ble_mgr_waitqueue_acquire();
#endif

        for (idx = 0; idx < waitqueue_len; idx++) {
                waitqueue_element_t *elem = &waitqueue.queue[idx];

                void *param = elem->param;

                /* Remove from queue by decreasing waitqueue.len */
                waitqueue.len--;

                /* Free param buffer */
                OS_FREE(param);
        }

#if (BLE_MGR_DIRECT_ACCESS == 1)
        ble_mgr_waitqueue_release();
#endif
}

static bool ble_gtl_handle_gapm_cmp_evt(ble_gtl_msg_t *gtl)
{
        struct gapm_cmp_evt *gevt = (void *) gtl->param;

        switch (gevt->operation) {
#if (dg_configBLE_PERIPHERAL == 1) || (dg_configBLE_BROADCASTER == 1)
        case GAPM_ADV_NON_CONN:
        case GAPM_ADV_UNDIRECT:
        case GAPM_ADV_DIRECT:
        case GAPM_ADV_DIRECT_LDC:
                ble_mgr_gapm_adv_cmp_evt_handler(gtl);
                break;
        case GAPM_UPDATE_ADVERTISE_DATA:
                break;
        case GAPM_CANCEL_ADVERTISE:
                break;
#endif /* (dg_configBLE_PERIPHERAL == 1) || (dg_configBLE_BROADCASTER == 1) */
#if (dg_configBLE_CENTRAL == 1) || (dg_configBLE_OBSERVER == 1)
        case GAPM_SCAN_ACTIVE:
        case GAPM_SCAN_PASSIVE:
                ble_mgr_gapm_scan_cmp_evt_handler(gtl);
                break;
        case GAPM_CANCEL_SCAN:
                break;
#endif /* (dg_configBLE_CENTRAL == 1) || (dg_configBLE_OBSERVER == 1) */
#if (dg_configBLE_CENTRAL == 1)
        case GAPM_CONNECTION_DIRECT:
                ble_mgr_gapm_connect_cmp_evt_handler(gtl);
                break;
        case GAPM_CANCEL_CONNECTION:
                break;
        case GAPM_SET_CHANNEL_MAP:
                break;
#endif /* (dg_configBLE_CENTRAL == 1) */
#if (dg_configBLE_CENTRAL == 1) || (dg_configBLE_PERIPHERAL == 1)
        case GAPM_SET_SUGGESTED_DFLT_LE_DATA_LEN:
                ble_mgr_gap_cmp__data_length_set_evt_handler(gtl);
#endif /* (dg_configBLE_CENTRAL == 1) || (dg_configBLE_PERIPHERAL == 1) */
                break;
        case GAPM_RESOLV_ADDR:
                ble_mgr_gapm_cmp__address_resolve_evt_handler(gtl);
                break;
        case GAPM_RESET:
        case GAPM_CANCEL:
        case GAPM_SET_DEV_CONFIG:
        case GAPM_GET_DEV_VERSION:
        case GAPM_GET_DEV_BDADDR:
        case GAPM_SET_TX_PW:
        case GAPM_LE_WR_RF_PATH_COMPENS:
#if (dg_configBLE_PERIPHERAL == 1) || (dg_configBLE_BROADCASTER == 1)
        case GAPM_SET_ADV_PERMUTATION:
#endif /* (dg_configBLE_PERIPHERAL == 1) || (dg_configBLE_BROADCASTER == 1) */
                break;
        default:
                return false;
        }

       return true;
}

static bool ble_gtl_handle_gapc_cmp_evt(ble_gtl_msg_t *gtl)
{
        struct gapc_cmp_evt *gevt = (void *) gtl->param;

        switch (gevt->operation) {
#if (dg_configBLE_CENTRAL == 1) || (dg_configBLE_PERIPHERAL == 1)
        case GAPC_DISCONNECT:
                ble_mgr_gapc_cmp__disconnect_evt_handler(gtl);
                break;
        case GAPC_UPDATE_PARAMS:
                ble_mgr_gapc_cmp__update_params_evt_handler(gtl);
                break;
        case GAPC_SET_LE_PKT_SIZE:
                ble_mgr_gap_cmp__data_length_set_evt_handler(gtl);
                break;
        case GAPC_GET_PEER_VERSION:
        case GAPC_GET_PEER_FEATURES:
        case GAPC_GET_CON_RSSI:
        case GAPC_SET_TX_PWR:
        //F_PCLE
        case GAPC_LE_RD_TX_PWR_LVL_ENH:
        case GAPC_LE_RD_REM_TX_PWR_LVL:
        case GAPC_LE_SET_PATH_LOSS_REPORT_PARAMS:
        case GAPC_LE_SET_PATH_LOSS_REPORT_EN:
        case GAPC_LE_SET_TX_PWR_REPORT_EN:
        case GAPC_SET_SKIP_LATENCY:
                break;
#endif /* (dg_configBLE_CENTRAL == 1) || (dg_configBLE_PERIPHERAL == 1) */
#if (dg_configBLE_CENTRAL == 1)
        case GAPC_BOND:
                ble_mgr_gapc_cmp__bond_evt_handler(gtl);
                break;
        case GAPC_ENCRYPT:
                break;
#endif /* (dg_configBLE_CENTRAL == 1) */
#if (dg_configBLE_PERIPHERAL == 1)
        case GAPC_SECURITY_REQ:
                ble_mgr_gapc_cmp__security_req_evt_handler(gtl);
                break;
#endif /* (dg_configBLE_PERIPHERAL == 1) */
#if (dg_configBLE_L2CAP_COC == 1)
        case GAPC_LE_CB_CONNECTION:
                ble_mgr_gapc_cmp__le_cb_connection_evt_handler(gtl);
                break;
#endif /* (dg_configBLE_L2CAP_COC == 1) */

        default:
                return false;
        }

       return true;
}

static bool ble_gtl_handle_gattc_cmp_evt(ble_gtl_msg_t *gtl)
{
        struct gattc_cmp_evt *gevt = (void *) gtl->param;

        switch (gevt->operation) {
#if (dg_configBLE_GATT_SERVER == 1)
        case GATTC_NOTIFY:
        case GATTC_INDICATE:
                ble_mgr_gatts_event_sent_evt_handler(gtl);
                break;
        case GATTC_SVC_CHANGED:
                break;
#endif /* (dg_configBLE_GATT_SERVER == 1) */
#if (dg_configBLE_GATT_CLIENT == 1)
        case GATTC_SDP_DISC_SVC:
        case GATTC_SDP_DISC_SVC_ALL:
                ble_mgr_gattc_cmp__browse_evt_handler(gtl);
                break;
        case GATTC_DISC_BY_UUID_SVC:
        case GATTC_DISC_BY_UUID_CHAR:
        case GATTC_DISC_ALL_SVC:
        case GATTC_DISC_ALL_CHAR:
        case GATTC_DISC_DESC_CHAR:
        case GATTC_DISC_INCLUDED_SVC:
                ble_mgr_gattc_cmp__discovery_evt_handler(gtl);
                break;
        case GATTC_READ:
                ble_mgr_gattc_cmp__read_evt_handler(gtl);
                break;
        case GATTC_WRITE:
        case GATTC_WRITE_NO_RESPONSE:
        case GATTC_EXEC_WRITE:
                ble_mgr_gattc_cmp__write_evt_handler(gtl);
                break;
#endif /* (dg_configBLE_GATT_CLIENT == 1) */
#if (dg_configBLE_GATT_SERVER == 1) || (dg_configBLE_GATT_CLIENT == 1)
        case GATTC_MTU_EXCH:
                break;
#endif /* (dg_configBLE_GATT_SERVER == 1) || (dg_configBLE_GATT_CLIENT == 1) */
        default:
                return false;
        }

       return true;
}

bool ble_gtl_handle_event(ble_gtl_msg_t *gtl)
{
        switch (gtl->msg_id) {
        /* Complete events */
        case GAPM_CMP_EVT:
                return ble_gtl_handle_gapm_cmp_evt(gtl);
        case GAPC_CMP_EVT:
                return ble_gtl_handle_gapc_cmp_evt(gtl);
        case GATTC_CMP_EVT:
                return ble_gtl_handle_gattc_cmp_evt(gtl);

        /* GAPM events */
        case GAPM_DEV_BDADDR_IND:
                ble_mgr_gap_dev_bdaddr_ind_evt_handler(gtl);
                break;
#if (dg_configBLE_CENTRAL == 1) || (dg_configBLE_OBSERVER == 1)
        case GAPM_ADV_REPORT_IND:
                ble_mgr_gap_adv_report_evt_handler(gtl);
                break;
#endif /* (dg_configBLE_CENTRAL == 1) || (dg_configBLE_OBSERVER == 1) */
        case GAPM_ADDR_SOLVED_IND:
                ble_mgr_gap_addr_solved_evt_handler(gtl);
                break;

        /* GAPC events */
#if (dg_configBLE_CENTRAL == 1) || (dg_configBLE_PERIPHERAL == 1)
        case GAPC_CONNECTION_REQ_IND:
                ble_mgr_gap_connected_evt_handler(gtl);
                break;
        case GAPC_DISCONNECT_IND:
                ble_mgr_gap_disconnected_evt_handler(gtl);
                break;
        case GAPC_PEER_VERSION_IND:
                ble_mgr_gap_peer_version_ind_evt_handler(gtl);
                break;
        case GAPC_PEER_FEATURES_IND:
                ble_mgr_gap_peer_features_ind_evt_handler(gtl);
                break;
        case GAPC_CON_RSSI_IND:
                ble_mgr_gap_con_rssi_ind_handler(gtl);
                break;
#endif /* #if (dg_configBLE_CENTRAL == 1) || (dg_configBLE_PERIPHERAL == 1) */
        case GAPC_GET_DEV_INFO_REQ_IND:
                ble_mgr_gap_get_device_info_req_evt_handler(gtl);
                break;
        case GAPC_SET_DEV_INFO_REQ_IND:
                ble_mgr_gap_set_device_info_req_evt_handler(gtl);
                break;
#if (dg_configBLE_CENTRAL == 1) || (dg_configBLE_PERIPHERAL == 1)
        case GAPC_PARAM_UPDATE_REQ_IND:
                ble_mgr_gap_conn_param_update_req_evt_handler(gtl);
                break;
        case GAPC_PARAM_UPDATED_IND:
                ble_mgr_gap_conn_param_updated_evt_handler(gtl);
                break;
        case GAPC_BOND_REQ_IND:
                ble_mgr_gap_bond_req_evt_handler(gtl);
                break;
        case GAPC_BOND_IND:
                ble_mgr_gap_bond_ind_evt_handler(gtl);
                break;
        case GAPC_ENCRYPT_IND:
                ble_mgr_gap_encrypt_ind_evt_handler(gtl);
                break;
        case GAPC_LE_PKT_SIZE_IND:
                ble_mgr_gap_le_pkt_size_ind_evt_handler(gtl);
                break;
#endif /* #if (dg_configBLE_CENTRAL == 1) || (dg_configBLE_PERIPHERAL == 1) */
#if (dg_configBLE_2MBIT_PHY == 1)
        case GAPC_LE_PHY_IND:
                ble_mgr_gap_le_phy_ind_handler(gtl);
                break;
#endif /* (dg_configBLE_2MBIT_PHY == 1) */
#if (dg_configBLE_CENTRAL == 1)
        case GAPC_SECURITY_IND:
                ble_mgr_gap_security_ind_evt_handler(gtl);
                break;
#endif /* (dg_configBLE_CENTRAL == 1) */
        case GAPC_SIGN_COUNTER_IND:
                ble_mgr_gap_sign_counter_ind_evt_handler(gtl);
                break;
#if (dg_configBLE_PERIPHERAL == 1)
        case GAPC_ENCRYPT_REQ_IND:
                ble_mgr_gap_encrypt_req_ind_evt_handler(gtl);
                break;
#endif /* (dg_configBLE_PERIPHERAL == 1) */
#if (dg_configBLE_L2CAP_COC == 1)
        case GAPC_LECB_CONNECT_IND:
                ble_mgr_l2cap_connect_ind_evt_handler(gtl);
                break;
        case GAPC_LECB_DISCONNECT_IND:
                ble_mgr_l2cap_disconnect_ind_evt_handler(gtl);
                break;
        case GAPC_LECB_CONNECT_REQ_IND:
                ble_mgr_l2cap_connect_req_ind_evt_handler(gtl);
                break;
        case GAPC_LECB_ADD_IND:
                ble_mgr_l2cap_add_ind_evt_handler(gtl);
                break;
#endif /* (dg_configBLE_L2CAP_COC == 1) */
        case GAPC_LE_RD_TX_PWR_LVL_ENH_IND:
                ble_mgr_gap_le_rd_tx_pwr_lvl_enh_ind_handler(gtl);
                break;
        case GAPC_LE_TX_PWR_REPORT_IND:
                ble_mgr_gap_le_tx_pwr_lvl_report_ind_handler(gtl);
                break;
        case GAPC_LE_PATH_LOSS_THRES_IND:
                ble_mgr_gap_le_path_loss_thres_ind_handler(gtl);
                break;

        /* GATTC events */
#if (dg_configBLE_GATT_SERVER == 1)
        case GATTC_READ_REQ_IND:
                ble_mgr_gatts_read_value_req_evt_handler(gtl);
                break;
        case GATTC_WRITE_REQ_IND:
                ble_mgr_gatts_write_value_req_evt_handler(gtl);
                break;
        case GATTC_ATT_INFO_REQ_IND:
                ble_mgr_gatts_prepare_write_req_evt_handler(gtl);
                break;
        case GATTC_SVC_CHANGED_CFG_IND:
                ble_mgr_gattc_svc_changed_cfg_ind_evt_handler(gtl);
                break;
#endif /* (dg_configBLE_GATT_SERVER == 1) */
        case GATTC_MTU_CHANGED_IND:
                ble_mgr_gattc_mtu_changed_ind_evt_handler(gtl);
                break;
#if (dg_configBLE_GATT_CLIENT == 1)
        case GATTC_SDP_SVC_IND:
                ble_mgr_gattc_sdp_svc_ind_evt_handler(gtl);
                break;
        case GATTC_DISC_SVC_IND:
                ble_mgr_gattc_disc_svc_ind_evt_handler(gtl);
                break;
        case GATTC_DISC_SVC_INCL_IND:
                ble_mgr_gattc_disc_svc_incl_ind_evt_handler(gtl);
                break;
        case GATTC_DISC_CHAR_IND:
                ble_mgr_gattc_disc_char_ind_evt_handler(gtl);
                break;
        case GATTC_DISC_CHAR_DESC_IND:
                ble_mgr_gattc_disc_char_desc_ind_evt_handler(gtl);
                break;
        case GATTC_READ_IND:
                ble_mgr_gattc_read_ind_evt_handler(gtl);
                break;
        case GATTC_EVENT_IND:
                ble_mgr_gattc_event_ind_evt_handler(gtl);
                break;
#endif /* (dg_configBLE_GATT_CLIENT == 1) */
#if (dg_configBLE_GATT_SERVER == 1) || (dg_configBLE_GATT_CLIENT == 1)
        case GATTC_TRANSACTION_TO_ERROR_IND:
                break;
        case GATTC_EVENT_REQ_IND:
                /* The Service Changed Indication could be received even if the
                 * application implements the GATT Server functionality only */
                ble_mgr_gattc_event_req_ind_evt_handler(gtl);
                break;
#endif
#if (dg_configBLE_L2CAP_COC == 1)
        // L2CC events:
        case L2CC_PDU_SEND_RSP:
                ble_mgr_l2cap_pdu_send_rsp_evt_handler(gtl);
                break;
        case L2CC_LECNX_DATA_RECV_IND:
                ble_mgr_l2cap_lecnx_data_recv_ind_evt_handler(gtl);
                break;
#endif /* (dg_configBLE_L2CAP_COC == 1) */
        default:
                return false;
        }

        return true;
}
