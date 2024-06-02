#ifndef BSP_DEFAULTS_DA1470x_BRINGUP_H_
#define BSP_DEFAULTS_DA1470x_BRINGUP_H_


/**
 ****************************************************************************************
 *
 * @file bsp_defaults_bringup_da1470x.h
 *
 * @brief Board Support Package. System configuration default values to be used during
 *        DA1470x (silicon) bring up.
 *
 * Copyright (C) 2020-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#define dg_configRTC_CORRECTION                         ( 0 )

#if (MAIN_PROCESSOR_BUILD)

/* Should be enabled for untrimmed samples in custom configuration file */
#ifndef TEST_CS_IN_CONST_TABLE
#define TEST_CS_IN_CONST_TABLE                          ( 0 )
#endif

/* Should be enabled for untrimmed/T0/T0- samples in custom configuration file */
#ifndef TEST_WITH_UNTRIMMED_SILICON
#define TEST_WITH_UNTRIMMED_SILICON                     ( 0 )
#endif

#elif (SNC_PROCESSOR_BUILD)

#undef TEST_CS_IN_CONST_TABLE
#define TEST_CS_IN_CONST_TABLE                          ( 0 )

#undef TEST_WITH_UNTRIMMED_SILICON
#define TEST_WITH_UNTRIMMED_SILICON                     ( 0 )

#endif /* PROCESSOR_BUILD */


#if (TEST_WITH_UNTRIMMED_SILICON == 0)
#define apply_cs_register_values_for_untrimmed_samples()                         \
        do { } while (0)
#else
#define apply_cs_register_values_for_untrimmed_samples()                         \
        CRG_TOP->BANDGAP_REG = 0x00009020;                                       \
        CRG_TOP->CLK_RCHS_REG = 0x001244B2;                                      \
        CRG_TOP->POWER_LVL_REG = 0x00019834;                                     \
        *(volatile uint32_t *) 0x5005042C = 0x371DCD95;
#endif /* TEST_WITH_UNTRIMMED_SILICON */


#endif /* BSP_DEFAULTS_DA1470x_BRINGUP_H_ */
