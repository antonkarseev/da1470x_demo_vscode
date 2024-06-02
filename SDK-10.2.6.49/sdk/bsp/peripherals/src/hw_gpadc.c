/**
 ****************************************************************************************
 *
 * @file hw_gpadc.c
 *
 * @brief Implementation of the GPADC Low Level Driver.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_GPADC

#include "hw_gpadc.h"
#if HW_GPADC_DMA_SUPPORT
#include "hw_dma.h"
#define GPADC_DMA_TRIGGER                       HW_DMA_TRIG_GP_ADC_APP_ADC

static DMA_setup gpadc_dma_setup;
#endif

extern void hw_gpadc_check_tcs_custom_values(int16_t se_gain_error, int16_t se_offset_error, int16_t diff_gain_error, int16_t diff_offset_error);
static uint32_t conversions_to_go = 0;
static uint16_t *gpadc_user_buffer = NULL;
static void *gpadc_user_param = NULL;
hw_gpadc_read_cb gpadc_user_callback;

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#define ADC_IRQ                                 ADC_IRQn
#define ADC_Handler                             GPADC_Handler

#define DIFF_TEMP_OFFSET                        0xAC    /* Fixed offset for the DIFF_TEMP sensors */

__RETAINED static int16_t hw_gpadc_differential_gain_error;
__RETAINED static int16_t hw_gpadc_single_ended_gain_error;
__RETAINED static int16_t hw_gpadc_differential_offset_error;
__RETAINED static int16_t hw_gpadc_single_ended_offset_error;

static hw_gpadc_interrupt_cb intr_cb = NULL;

/**
 * \brief Get trimmed values for the two OFFP OFFN registers in either mode
 *
 * \param[in]  input_mode indicates differential or single ended
 * \param[in]  offp trim value for GP_ADC_OFFP
 * \param[in]  offn trim value for GP_ADC_OFFN
 */
__WEAK bool hw_gpadc_get_trimmed_offsets_from_cs(uint8_t mode, uint16_t *offp, uint16_t *offn);


/**
 * \brief Wait for the ADC state-machine to exit
 *
 */
__STATIC_INLINE void hw_gpadc_stop_engine(void)
{
        hw_gpadc_set_continuous(false);
        while (hw_gpadc_in_progress());
}

/**
 * \brief Set the ADC engine to stop, unregister any interrupt handler and exit immediately
 *
 */
__STATIC_INLINE void hw_gpadc_stop_no_wait(void)
{
        hw_gpadc_set_continuous(false);
        hw_gpadc_unregister_interrupt_no_clear();
}

void hw_gpadc_init(const gpadc_config *cfg, bool enable)
{
        /* Assert that no changes occur while conversions are on the way */
        hw_gpadc_stop_engine();

        /*
         * Check if custom trim settings are already applied. If not, apply custom trim settings now.
         */
        hw_gpadc_check_tcs_custom_values(hw_gpadc_single_ended_gain_error, hw_gpadc_single_ended_offset_error, hw_gpadc_differential_gain_error, hw_gpadc_differential_offset_error);
        /*
         * Reset the control registers to defaults,
         * configuring the LDO state as requested.
         */
        if (enable) {
                GPADC->GP_ADC_CTRL_REG = GPADC_GP_ADC_CTRL_REG_GP_ADC_EN_Msk;
        } else {
                GPADC->GP_ADC_CTRL_REG = 0;
        }
        /* default RESULT_MODE = 0x2 */
        GPADC->GP_ADC_CTRL_REG |= 0x00010000;

        /* default SMPL_TIME = 0x1 */
        GPADC->GP_ADC_CTRL2_REG = 0x0200;
        GPADC->GP_ADC_CTRL3_REG = 0x40;      // default value for GP_ADC_EN_DEL
        GPADC->GP_ADC_SEL_REG = 0;
        hw_gpadc_set_offset_positive(HW_GPADC_OFFSET_RESET);
        hw_gpadc_set_offset_negative(HW_GPADC_OFFSET_RESET);

        /* Unregister the user callback */
        hw_gpadc_unregister_interrupt();

#if HW_GPADC_DMA_SUPPORT
        /* Invalidate any DMA configuration by disabling the respective channel */
        if (gpadc_dma_setup.channel_number < HW_DMA_CHANNEL_INVALID) {
                hw_dma_channel_enable(gpadc_dma_setup.channel_number, HW_DMA_STATE_DISABLED);
        }
        gpadc_dma_setup.channel_number = HW_DMA_CHANNEL_INVALID;
#endif

        if (cfg) {
                /* Initialize with configuration */
                hw_gpadc_configure(cfg);
        }
}

void hw_gpadc_offset_calibrate(void)
{
        uint16_t adc_off_p, adc_off_n, verify, deviation;
        int factor;

        bool ldo_state = hw_gpadc_is_enabled();
        bool sign_state = hw_gpadc_get_sign_change();
        HW_GPADC_OVERSAMPLING ovs_state = hw_gpadc_get_oversampling();
        uint8_t sample_time_state = hw_gpadc_get_sample_time();
        HW_GPADC_INPUT_MODE mode = hw_gpadc_get_input_mode();
        bool continuous_state = hw_gpadc_get_continuous();
        HW_GPADC_RESULT_MODE result_mode = hw_gpadc_get_result_mode();

        if (!ldo_state) {
                hw_gpadc_enable();
        }

        if (sign_state) {
                hw_gpadc_set_sign_change(false);
        }

        if (continuous_state) {
                hw_gpadc_set_continuous(false);
#if HW_GPADC_DMA_SUPPORT
                /*
                 * Absolutely improper time for calibration
                 */
                if (gpadc_dma_setup.channel_number != HW_DMA_CHANNEL_INVALID) {
                        ASSERT_ERROR(hw_dma_is_channel_active(gpadc_dma_setup.channel_number) == false);
                }
#endif
        }

        hw_gpadc_set_oversampling(HW_GPADC_OVERSAMPLING_16_SAMPLES);
        hw_gpadc_set_sample_time(3);
        hw_gpadc_set_result_mode(HW_GPADC_RESULT_NORMAL);
        hw_gpadc_set_mute(true);
        hw_gpadc_set_offset_positive(HW_GPADC_OFFSET_RESET);
        hw_gpadc_set_offset_negative(HW_GPADC_OFFSET_RESET);

        /* formula differs for SE and DIFF modes by this factor */
        factor = (mode == HW_GPADC_INPUT_MODE_SINGLE_ENDED) ? 2 : 1;

        /* Up to five calibration tries */
        for (int i = 0; i < 5; i++) {
                hw_gpadc_adc_measure();
                adc_off_p = (hw_gpadc_get_raw_value() >> HW_GPADC_UNUSED_BITS) - HW_GPADC_OFFSET_RESET;

                hw_gpadc_set_sign_change(true);
                hw_gpadc_adc_measure();
                adc_off_n = (hw_gpadc_get_raw_value() >> HW_GPADC_UNUSED_BITS) - HW_GPADC_OFFSET_RESET;

                hw_gpadc_set_offset_positive(HW_GPADC_OFFSET_RESET - factor * adc_off_p);
                hw_gpadc_set_offset_negative(HW_GPADC_OFFSET_RESET - factor * adc_off_n);

                hw_gpadc_set_sign_change(false);
                hw_gpadc_adc_measure();

                /* Verification: Is result on mute close to 0x200 ? */
                verify = hw_gpadc_get_raw_value() >> HW_GPADC_UNUSED_BITS;
                deviation = (verify < HW_GPADC_OFFSET_RESET)? (HW_GPADC_OFFSET_RESET - verify) : (verify - HW_GPADC_OFFSET_RESET);

                /* Calibration converges */
                if (deviation < 0x8) {
                        break;
                }

                /* Reset OFFSET registers if calibration does not converge */
                if (i == 4) {
                        ASSERT_WARNING(0);
                        hw_gpadc_set_offset_positive(HW_GPADC_OFFSET_RESET);
                        hw_gpadc_set_offset_negative(HW_GPADC_OFFSET_RESET);
                }
        }

        hw_gpadc_set_sign_change(sign_state);
        hw_gpadc_set_oversampling(ovs_state);
        hw_gpadc_set_sample_time(sample_time_state);
        hw_gpadc_set_result_mode(result_mode);

        if (continuous_state) {
                hw_gpadc_set_continuous(true);
        }

        if (!ldo_state) {
                hw_gpadc_disable();
        }

        hw_gpadc_set_mute(false);
}

#if HW_GPADC_DMA_SUPPORT
static void hw_gpadc_dma_configure(gpadc_dma_cfg *cfg)
{
        /*
         * According to the DMA_REQ_MUX description,
         * the GPADC trigger operates only with even channels.
         */
        ASSERT_ERROR(cfg->channel < HW_DMA_CHANNEL_INVALID && (cfg->channel & 0x1) == 0);

        /*
         * Apply DMA volatile user configuration
         */
        gpadc_dma_setup.channel_number = cfg->channel;
        gpadc_dma_setup.dma_prio = cfg->prio;
        gpadc_dma_setup.irq_nr_of_trans = cfg->irq_nr_of_trans;
        gpadc_dma_setup.circular = cfg->circular ? HW_DMA_MODE_CIRCULAR : HW_DMA_MODE_NORMAL;

        /*
         * Apply DMA fixed configuration suitable for GPADC
         */
        gpadc_dma_setup.bus_width = HW_DMA_BW_HALFWORD;
        gpadc_dma_setup.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
        gpadc_dma_setup.dreq_mode = HW_DMA_DREQ_TRIGGERED;
        gpadc_dma_setup.burst_mode = HW_DMA_BURST_MODE_DISABLED;
        gpadc_dma_setup.a_inc = HW_DMA_AINC_FALSE;
        gpadc_dma_setup.b_inc = HW_DMA_BINC_TRUE;
        gpadc_dma_setup.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE;
        gpadc_dma_setup.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
        gpadc_dma_setup.dma_req_mux = GPADC_DMA_TRIGGER;
        gpadc_dma_setup.src_address = (uint32_t) &GPADC->GP_ADC_RESULT_REG;
        gpadc_dma_setup.callback = NULL;
        gpadc_dma_setup.user_data = NULL;

        hw_gpadc_set_dma_functionality(true);
}
#endif

void hw_gpadc_configure(const gpadc_config *cfg)
{
        bool irq_enabled = false;

        ASSERT_ERROR(cfg);
        ASSERT_WARNING(hw_gpadc_in_progress() == false);

        /*
         * Pause serving the interrupts while configuring the ADC
         */
        if (NVIC_GetEnableIRQ(ADC_IRQ)) {
                NVIC_DisableIRQ(ADC_IRQ);
                NVIC_ClearPendingIRQ(ADC_IRQ);
                irq_enabled = true;
        }

        hw_gpadc_set_input_mode(cfg->input_mode);
        if (cfg->positive == HW_GPADC_INP_DIE_TEMP) {
                /*
                 * Ensure a 25 usec delay interval before
                 * enabling the GPADC LDO for DIE_TEMP to settle
                 */
                hw_gpadc_set_die_temp(true);
                hw_gpadc_set_ldo_delay(HW_GPADC_TEMPSENS_INIT_DELAY);
                /*
                 * Force the recommended settings for accurate temperature conversion.
                 * This matches the GPADC configuration when calculating the calibration point
                 * stored in the OTP - Configuration Script entry SYS_TCS_GROUP_TEMP_SENS_25C
                 */
                hw_gpadc_set_ldo_constant_current(true);
                ASSERT_WARNING(cfg->sample_time == HW_GPADC_DIE_TEMP_SMPL_TIME);
                ASSERT_WARNING(cfg->chopping == true);
                ASSERT_WARNING(cfg->input_attenuator == HW_GPADC_INPUT_VOLTAGE_UP_TO_0V9);
                ASSERT_WARNING(cfg->oversampling >= HW_GPADC_OVERSAMPLING_16_SAMPLES);
        }

        if (cfg->positive == HW_GPADC_INP_DIFF_TEMP &&
            cfg->temp_sensor > HW_GPADC_NO_TEMP_SENSOR) {
                /*
                 * Adjust the ADC to handle the high diode voltage
                 */
                hw_gpadc_set_offset_positive(DIFF_TEMP_OFFSET);
                hw_gpadc_set_offset_negative(DIFF_TEMP_OFFSET);
                goto calibration_done;
        }
        /*
         * Try retrieving OFFP/OFFN from the Configuration Script.
         * Runtime Offset Calibration for the previously chosen input mode in case of failure.
         */
        if (hw_gpadc_get_trimmed_offsets_from_cs) {
                uint16_t offp, offn;
                if (hw_gpadc_get_trimmed_offsets_from_cs(cfg->input_mode, &offp, &offn)) {
                        hw_gpadc_set_offset_positive(offp);
                        hw_gpadc_set_offset_negative(offn);
                } else {
                        hw_gpadc_offset_calibrate();
                }
        } else {
                hw_gpadc_offset_calibrate();
        }
calibration_done:

        hw_gpadc_set_positive(cfg->positive);
        hw_gpadc_set_negative(cfg->negative);
        hw_gpadc_set_sample_time(cfg->sample_time);
        hw_gpadc_set_continuous(cfg->continuous);
        hw_gpadc_set_interval(cfg->interval);
        hw_gpadc_set_input_attenuator_state(cfg->input_attenuator);
        if (cfg->positive == HW_GPADC_INP_VSYS || cfg->positive == HW_GPADC_INP_VBUS || cfg->positive == HW_GPADC_INP_VBAT) {
                /*
                 * Using an attenuator scaler to measure these input channels is not recommended.
                 * Each of them is driven to the ADC using their own intrinsic scaling.
                 * Check the documentation of HW_GPADC_INPUT_MUX2 for detailed scaler values.
                 */
                ASSERT_WARNING(cfg->input_attenuator == HW_GPADC_INPUT_VOLTAGE_UP_TO_0V9);
        }
        hw_gpadc_set_chopping(cfg->chopping);
        hw_gpadc_set_oversampling(cfg->oversampling);
        hw_gpadc_set_result_mode(cfg->result_mode);
        if (hw_gpadc_get_positive() == HW_GPADC_INP_DIFF_TEMP) {
                const HW_GPADC_TEMP_SENSORS sensor = cfg->temp_sensor;

                ASSERT_ERROR(sensor <= HW_GPADC_TEMP_SENSOR_NEAR_BANDGAP);
                ASSERT_WARNING(sensor != HW_GPADC_NO_TEMP_SENSOR);
                ASSERT_WARNING(sensor != HW_GPADC_CHARGER_TEMPSENS_GND);
                /*
                 * Switches on/off the GP_ADC_DIFF_TEMP_EN bit, according to cfg->temp_sensor value.
                 * This field drives the TEMPSENS input circuit (diodes or charger tempsens).
                 */
                hw_gpadc_set_diff_temp_sensors(sensor > HW_GPADC_CHARGER_TEMPSENS_VTEMP);
                hw_gpadc_select_diff_temp_sensor(sensor);
                /*
                 * Enforcing all highly recommended settings for correct DIFF_TEMP usage
                 */
                ASSERT_WARNING(hw_gpadc_get_chopping() == true);
                ASSERT_WARNING(hw_gpadc_get_sample_time() >= 4);
                if (sensor == HW_GPADC_TEMP_SENSOR_NEAR_RADIO ||
                    sensor == HW_GPADC_TEMP_SENSOR_NEAR_CHARGER ||
                    sensor == HW_GPADC_TEMP_SENSOR_NEAR_BANDGAP) {
                        ASSERT_WARNING(hw_gpadc_get_input_attenuator_state() == HW_GPADC_INPUT_VOLTAGE_UP_TO_0V9);
                        ASSERT_WARNING(hw_gpadc_get_oversampling() >= HW_GPADC_OVERSAMPLING_16_SAMPLES);
                }
                hw_gpadc_set_ldo_constant_current(true);
                hw_gpadc_set_ldo_delay(HW_GPADC_TEMPSENS_INIT_DELAY);
        }
#if HW_GPADC_DMA_SUPPORT
        if (cfg->dma_setup) {
                hw_gpadc_dma_configure(cfg->dma_setup);
        }
#endif
        if (irq_enabled) {
                NVIC_ClearPendingIRQ(ADC_IRQ);
                NVIC_EnableIRQ(ADC_IRQ);
        }
}

void hw_gpadc_register_interrupt(hw_gpadc_interrupt_cb cb)
{
        intr_cb = cb;

        hw_gpadc_clear_interrupt();
        hw_gpadc_set_mint(true);

        NVIC_ClearPendingIRQ(ADC_IRQ);
        NVIC_EnableIRQ(ADC_IRQ);
}

void hw_gpadc_unregister_interrupt(void)
{
        hw_gpadc_set_mint(false);
        hw_gpadc_clear_interrupt();

        NVIC_DisableIRQ(ADC_IRQ);
        NVIC_ClearPendingIRQ(ADC_IRQ);

        intr_cb = NULL;
}

void hw_gpadc_unregister_interrupt_no_clear(void)
{
        hw_gpadc_set_mint(false);
        NVIC_DisableIRQ(ADC_IRQ);
        NVIC_ClearPendingIRQ(ADC_IRQ);
        intr_cb = NULL;
}

void ADC_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        hw_gpadc_clear_interrupt();

        if (intr_cb) {
                intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

static void read_irq_callback_wrapper(void)
{
        conversions_to_go--;

        /* Last interrupt handling */
        if (conversions_to_go == 0) {
                hw_gpadc_register_interrupt(hw_gpadc_stop_no_wait);
        }

        if (gpadc_user_buffer) {
                *gpadc_user_buffer = hw_gpadc_get_raw_value();
                gpadc_user_buffer++;
                if (conversions_to_go == 0) {
                        gpadc_user_callback(gpadc_user_param, conversions_to_go);
                }
        } else {
                /* No buffer forces callback on every interrupt */
                gpadc_user_callback(gpadc_user_param, conversions_to_go);
        }
}

static bool read_irq_mode()
{
        if (conversions_to_go > 1) {
                ASSERT_WARNING(hw_gpadc_get_continuous());
        }

        hw_gpadc_register_interrupt(read_irq_callback_wrapper);

        hw_gpadc_start();
        return true;
}

#if HW_GPADC_DMA_SUPPORT
static void read_dma_callback_wrapper(void *user_data, dma_size_t len)
{
        /* Variable len holds the total DMA transferred items so far */
        conversions_to_go = gpadc_dma_setup.length - len;

        if ((len >= gpadc_dma_setup.length) && (gpadc_dma_setup.circular == HW_DMA_MODE_NORMAL)) {
                hw_gpadc_stop_no_wait();
        }

        if (gpadc_user_callback) {
                /* Notifies the user about remaining conversions */
                gpadc_user_callback(gpadc_user_param, conversions_to_go);
        }

        if (gpadc_dma_setup.irq_nr_of_trans) {
                uint16_t next_step = MIN(gpadc_dma_setup.length - 1, len + gpadc_dma_setup.irq_nr_of_trans - 1);
                hw_dma_channel_update_int_ix(gpadc_dma_setup.channel_number, next_step);
        }
}

static bool read_dma_mode()
{
        if (conversions_to_go > 1) {
                /* In interrupt and DMA modes the ADC engine needs to operate in continuous mode */
                ASSERT_WARNING(hw_gpadc_get_continuous());
        }
        if ((dma_size_t)gpadc_dma_setup.irq_nr_of_trans > conversions_to_go) {
                /* Invalid DMA configuration */
                return false;
        }
        if (gpadc_dma_setup.irq_nr_of_trans > 0 && gpadc_dma_setup.circular == HW_DMA_MODE_CIRCULAR) {
                /* This option is not supported */
                return false;
        }

        /*
         * Setup DMA - Enable channel
         */
        gpadc_dma_setup.length = (dma_size_t) conversions_to_go;
        gpadc_dma_setup.dest_address = (uint32_t) gpadc_user_buffer;
        gpadc_dma_setup.callback = read_dma_callback_wrapper;
        hw_dma_channel_initialization(&gpadc_dma_setup);

        hw_dma_channel_enable(gpadc_dma_setup.channel_number, HW_DMA_STATE_ENABLED);

        hw_gpadc_start();
        return true;
}
#endif

static bool read_polling_mode()
{
        if (conversions_to_go == 1) {
                hw_gpadc_adc_measure();
                /* outbuf can be omitted - the result register holds the result */
                if (gpadc_user_buffer) {
                        *gpadc_user_buffer = hw_gpadc_get_raw_value();
                }
                return true;
        }

        if (!gpadc_user_buffer) {
                /* A buffer is mandatory to store multiple results */
                return false;
        }

        for (uint32_t i = 0; i < conversions_to_go; i++) {
                hw_gpadc_adc_measure();
                gpadc_user_buffer[i] = hw_gpadc_get_raw_value();
        }
        return true;
}

void hw_gpadc_abort_read(void)
{
        hw_gpadc_stop_engine();

#if HW_GPADC_DMA_SUPPORT
        if (gpadc_dma_setup.channel_number < HW_DMA_CHANNEL_INVALID) {
                if (hw_dma_is_channel_active(gpadc_dma_setup.channel_number)) {
                        /*
                         * DMA callback in stop
                         */
                        hw_dma_channel_stop(gpadc_dma_setup.channel_number);
                }
                return;
        }
#endif
        if (gpadc_user_callback) {
                hw_gpadc_unregister_interrupt();
                gpadc_user_callback(gpadc_user_param, conversions_to_go);
        }
}

bool hw_gpadc_read(uint32_t nof_conv, uint16_t *out_buf, hw_gpadc_read_cb cb, void *user_data)
{
        if (nof_conv == 0) {
                return false;
        }

        if (hw_gpadc_in_progress()) {
                return false;
        }

        /*
         * Update local data
         */
        gpadc_user_buffer = out_buf;
        gpadc_user_callback = cb;
        gpadc_user_param = user_data;
        conversions_to_go = nof_conv;

#if HW_GPADC_DMA_SUPPORT
        if (gpadc_dma_setup.channel_number < HW_DMA_CHANNEL_INVALID) {
                /* A buffer is mandatory to set the DMA destination address */
                ASSERT_ERROR(gpadc_user_buffer != NULL);

                return read_dma_mode();
        }
#endif

        if (gpadc_user_callback) {
                return read_irq_mode();
        } else {
                return read_polling_mode();
        }
}

bool hw_gpadc_pre_check_for_gain_error(void)
{
        if (dg_configUSE_ADC_GAIN_ERROR_CORRECTION == 1) {
                return (hw_gpadc_single_ended_gain_error && hw_gpadc_differential_gain_error);
        }

        return false;
}

int16_t hw_gpadc_get_single_ended_gain_error(void)
{
        return hw_gpadc_single_ended_gain_error;
}

void hw_gpadc_store_se_gain_error(int16_t single)
{
        hw_gpadc_single_ended_gain_error = single;
}

void hw_gpadc_store_diff_gain_error(int16_t diff)
{
        hw_gpadc_differential_gain_error = diff;
}

void hw_gpadc_store_se_offset_error(int16_t single)
{
        hw_gpadc_single_ended_offset_error = single;
}

void hw_gpadc_store_diff_offset_error(int16_t diff)
{
        hw_gpadc_differential_offset_error = diff;
}

/*
 * This function performs a fine offset correction (on top of the coarse offset calibration)
 * and a gain correction, using the trimmed values from the Configuration Script.
 */
uint16_t hw_gpadc_apply_correction(const gpadc_config *cfg, uint16_t raw)
{
        int64_t res64;
        int32_t res;
        uint8_t mode = cfg? cfg->input_mode : hw_gpadc_get_input_mode();

        HW_GPADC_INPUT_POSITIVE channel = cfg ? cfg->positive : hw_gpadc_get_positive();
        if (channel == HW_GPADC_INP_DIFF_TEMP) {
                if (hw_gpadc_get_diff_temp_sensors()) {
                        return raw;
                }
        }

        res = raw;

        /* Offset Correction */
        if (mode == HW_GPADC_INPUT_MODE_SINGLE_ENDED) {
                res -=  hw_gpadc_single_ended_offset_error;
        } else {
                res -=  hw_gpadc_differential_offset_error;
        }
        /* Boundary check for lower limit */
        if (res <= 0) {
                return 0;
        }

        if (!hw_gpadc_pre_check_for_gain_error()) {
                return res;
        }

        res64 = res;

        /* Gain Correction */
        if (mode == HW_GPADC_INPUT_MODE_SINGLE_ENDED) {
                res64 = (UINT16_MAX * res64) / (UINT16_MAX + hw_gpadc_single_ended_gain_error);
                /* Boundary check for upper limit */
                if (res64 >= UINT16_MAX) {
                        return UINT16_MAX;
                }
                return res64;
        } else {
                res64 = (int16_t)(res64 ^ 0x8000);
                res64 = (UINT16_MAX * res64) / (UINT16_MAX + hw_gpadc_differential_gain_error);
                /* Boundary check for lower limit */
                if (res64 < INT16_MIN) {
                        return 0;
                }
                /* Boundary check for upper limit */
                if (res64 > INT16_MAX) {
                        return UINT16_MAX;
                }
                return res64 ^ 0x8000;
        }
}


uint16_t hw_gpadc_get_value(void)
{
        uint16_t adc_raw_res = hw_gpadc_get_raw_value();

        if (hw_gpadc_get_result_mode() == HW_GPADC_RESULT_TRUNCATED) {
                /*
                 * Make result MSB-aligned again with respect to oversampling
                 */
                adc_raw_res = adc_raw_res << (2 + MIN(HW_GPADC_UNUSED_BITS, hw_gpadc_get_oversampling()));
        }

        return hw_gpadc_apply_correction(NULL,
                adc_raw_res) >> (HW_GPADC_UNUSED_BITS - MIN(HW_GPADC_UNUSED_BITS, hw_gpadc_get_oversampling()));
}

int16_t hw_gpadc_convert_to_millivolt(const gpadc_config *cfg, uint16_t raw)
{
        uint32_t max_adc = UINT16_MAX;
        uint16_t corrected = (int32_t) hw_gpadc_apply_correction(cfg, raw);
        int32_t val;

        HW_GPADC_INPUT_MODE mode = cfg ? cfg->input_mode       : hw_gpadc_get_input_mode();
        HW_GPADC_MAX_INPUT_VOLTAGE attn_factor = cfg ? cfg->input_attenuator : hw_gpadc_get_input_attenuator_state();
        HW_GPADC_INPUT_POSITIVE channel        = cfg ? cfg->positive         : hw_gpadc_get_positive();

        if (mode == HW_GPADC_INPUT_MODE_DIFFERENTIAL) {
                val = 2 * corrected - max_adc;
        } else {
                val = hw_gpadc_internal_scaler_compensate(channel, corrected);
        }

        /* Scale according to attenuator state. */
        attn_factor++;

        val *= attn_factor * HW_GPADC_VREF_MILLIVOLT;
        return val / (int32_t) max_adc;
}

#endif /* dg_configUSE_HW_GPADC */
