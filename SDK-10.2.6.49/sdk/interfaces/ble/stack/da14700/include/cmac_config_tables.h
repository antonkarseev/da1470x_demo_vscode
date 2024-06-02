/**
 ****************************************************************************************
 * @file cmac_config_tables.h
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef _CMAC_CONFIG_TABLES_H
#define _CMAC_CONFIG_TABLES_H

#include <stdint.h>
#include <stdbool.h>



typedef struct cmac_config_ {
        bool     wait_on_main;                  // When this flag is set to true, CMAC will wait on main() entry until the flag becomes false.
        uint8_t  ble_bd_address[6];             // The BLE Device Address
        uint8_t  rf_calibration_delay;          // The maximum delay allowed for RF calibration (in multiples of 100 msec)
        uint8_t  lp_clock_freq;                 // LP clock type:
                                                //      0 = 32768Hz XTAL
                                                //      1 = 32000Hz XTAL
                                                //      2 = RCX
                                                // Default: 32768Hz
        uint16_t lp_clock_drift;                // Device SCA setting, Default: 500

        uint16_t ble_rx_buffer_size;            // BLE Rx data buffer size, Default: 262 bytes
        uint16_t ble_tx_buffer_size;            // BLE Tx data buffer size, Defalut: 262 bytes
        bool     ble_length_exchange_needed;    // Flag to control Length Exchange, Default: true
        uint16_t ble_chnl_assess_timer;         // Channel Assessment Timer duration (5s -
                                                // Multiple of 10ms), Default: 500, Private
        uint8_t  ble_chnl_reassess_timer;       // Channel Reassessment Timer duration (Multiple of
                                                // Channel Assessment Timer duration), Default: 8, Private
        int8_t   ble_chnl_assess_min_rssi;      // BLE Chnl Assess alg, Min RSSI, Default: -60 (dBm)
        uint16_t ble_chnl_assess_nb_pkt;        // # of packets to receive for statistics, Default: 20, Private
        uint16_t ble_chnl_assess_nb_bad_pkt;    // # of bad packets needed to remove a channel, Default: 10, Private
        uint8_t  system_tcs_length;             // Number of valid entries in the table
        uint8_t  synth_tcs_length;              // Number of valid entries in the table
        uint8_t  rfcu_tcs_length;               // Number of valid entries in the table
        uint8_t  initial_tx_power_lvl;          // The initial TX power level index used in Advertising and Data channels
        uint8_t  ble_dup_filter_max;            // Maximum number of devices for the duplicate filtering list
        bool     ble_dup_filter_found;          // Unknown devices are treated as "found" (be in the
                                                // duplicate filter buffer) when the buffer is full,
                                                // if true, Default: true, Private
        bool     use_high_performance_1m;       // Enable 1M High Performance mode
        bool     use_high_performance_2m;       // Enable 2M High Performance mode

        int8_t   golden_range_low;              // RSSI "Golden Range" lower value (dBm)
        int8_t   golden_range_up;               // RSSI "Golden Range" upper value (dBm)
        int8_t   golden_range_pref;             // Preferred RSSI value inside "Golden Range" (dBm)
        uint8_t  pcle_min_tx_pwr_idx;           // Min Tx Power index used in PCLE feature
        uint8_t  pcle_max_tx_pwr_idx;           // Max Tx Power index used in PCLE feature

} cmac_configuration_table_t;

typedef enum {
    CMAC_STATE_DISABLED,        // Not yet started
    CMAC_STATE_DEEPSLEEPING,    // Deep sleeping or entering deep sleep
    CMAC_STATE_AWAKE,           // Awake
} CMAC_STATE;

typedef struct cmac_dynamic_config_ {
        bool     sleep_enable;                  // Flag to control sleep, Default: false
        bool     ble_host_irq_after_event;      // Flag to control IRQ after BLE event, Default: false

        uint32_t rcx_period;                    // RCX period in usec as a 12.20 fixed point number.
        uint32_t rcx_clock_hz_acc;              // RCX frequency in Hz as a 29.3 fixed point number.
        uint16_t wakeup_time;                   // The total wake up time in LP clock cycles including
                                                // the HW FSM wake-up time plus
                                                // the XTAL32M settling time.
        bool        first_rfcu_enable;
        uint8_t     pwr_level;
        uint8_t     femonly_fine_atten;
        uint8_t     femonly_fine_atten_disabled;
        uint8_t     coarse_atten;
        uint8_t     rfio_tx_dcf_val;
        uint8_t     rfio_rx_dcf_val;
        uint8_t     rfio_tx_dcf_pref_val;
        uint32_t    tx_0dbm_2ndharm_trim;
        uint32_t    tx_6dbm_2ndharm_trim;

        uint32_t    power_ctrl_reg_onwakeup_value;  // The value that should be applied in POWER_CTRL_REG on wakeup
        uint32_t    power_ctrl_reg_onsleep_value;   // The value that should be applied in POWER_CTRL_REG on sleep
        uint32_t    power_level_reg_onwakeup_value; // The value that should be applied in POWER_LEVEL_REG on wakeup
        uint32_t    power_level_reg_onsleep_value;  // The value that should be applied in POWER_LEVEL_REG on sleep
        union {
                uint32_t  gpadc_tempsens_val;             // The last temperature sensor value read from GPADC
                uint32_t *gpadc_tempsens_ptr;             // Pointer to GPADC temperature value
        };

        uint8_t     maccpu_state;                   // The current state of MAC CPU (type of CMAC_STATE)

        /*
         * ble_advertising_permutation: The permutation index to take effect next time advertising
         * begins.  Its value will be propagated to adv_perm_sel at the beginning of the next
         * advertising cycle, so as to not violate the standard by broadcasting more than one PDU in
         * each channel.
         */
        uint8_t     ble_advertising_permutation;
} cmac_dynamic_configuration_table_t;

typedef struct cmac_info_ {
        uint32_t ble_conn_evt_counter[BLE_CONNECTION_MAX_USER];
        uint32_t ble_conn_evt_counter_non_apfm[BLE_CONNECTION_MAX_USER];

        uint32_t ble_adv_evt_counter;
        uint32_t ble_adv_evt_counter_non_apfm;
} cmac_info_table_t;

typedef struct cmac_tcs_table_ {
    uint32_t* tcs_attributes_ptr;
    uint32_t tcs_attributes_size;
    uint32_t* tcs_data_ptr;
    uint32_t tcs_data_size;
} cmac_tcs_table_t;

struct _tcs_entry {
        uint32_t *register_p;
        uint32_t value;
};

typedef struct cmac_exception_ctx_ {
        uint32_t magic;                 // 0x00
        uint32_t magic_0;               // 0x04
        uint32_t magic_1;               // 0x08

        // The SP when NMI_HandlerC() was called.
        //    (stacked_r8, .. , hf_stacked_psr)
        uint32_t stack_ptr;             // 0x0C

        // {r4 - r7} at time of crash
        uint32_t stacked_r8;            // 0x10
        uint32_t stacked_r9;            // 0x14
        uint32_t stacked_r10;           // 0x18
        uint32_t stacked_r11;           // 0x1C

        // {r8 - r11} at time of crash
        uint32_t stacked_r4;            // 0x20
        uint32_t stacked_r5;            // 0x24
        uint32_t stacked_r6;            // 0x28
        uint32_t stacked_r7;            // 0x2C

        // NMI exception frame
        uint32_t nmi_stacked_r0;        // 0x30
        uint32_t nmi_stacked_r1;        // 0x34
        uint32_t nmi_stacked_r2;        // 0x38
        uint32_t nmi_stacked_r3;        // 0x3C
        uint32_t nmi_stacked_r12;       // 0x40
        uint32_t nmi_stacked_lr;        // 0x44
        uint32_t nmi_stacked_pc;        // 0x48
        uint32_t nmi_stacked_psr;       // 0x4C

        // HardFault exception frame (Not always present)
        uint32_t hf_stacked_r0;         // 0x50
        uint32_t hf_stacked_r1;         // 0x54
        uint32_t hf_stacked_r2;         // 0x58
        uint32_t hf_stacked_r3;         // 0x5C
        uint32_t hf_stacked_r12;        // 0x60
        uint32_t hf_stacked_lr;         // 0x64
        uint32_t hf_stacked_pc;         // 0x68
        uint32_t hf_stacked_psr;        // 0x6C

        uint32_t _CFSR;                 // 0x70
        uint32_t _MMSR;                 // 0x74
        uint32_t _BFSR;                 // 0x78
        uint32_t _UFSR;                 // 0x7C
        uint32_t _HFSR;                 // 0x80
        uint32_t _DFSR;                 // 0x84
        uint32_t _AFSR;                 // 0x88
        uint32_t _BFAR;                 // 0x8C
        uint32_t _MMAR;                 // 0x90
        uint32_t error_val;             // 0x94
        uint32_t exc_val;               // 0x98
        uint32_t _BS_SMPL_ST;           // 0x9C
        uint32_t _BS_SMPL_D;            // 0xA0
} cmac_exception_ctx_t;

extern volatile cmac_configuration_table_t              *cmac_config_table_ptr;
extern volatile cmac_dynamic_configuration_table_t      *cmac_dynamic_config_table_ptr;
extern volatile cmac_info_table_t                       *cmac_info_table_ptr;
extern volatile cmac_tcs_table_t                        *cmac_tcs_table_ptr;
extern volatile struct _tcs_entry                       *cmac_sys_tcs_table_ptr;
extern volatile struct _tcs_entry                       *cmac_synth_tcs_table_ptr;
extern volatile struct _tcs_entry                       *cmac_rfcu_tcs_table_ptr;
extern volatile cmac_exception_ctx_t                    *cmac_exception_ctx_ptr;

#endif // _CMAC_CONFIG_TABLES_H
