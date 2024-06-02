/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SW_CURSOR Software Cursor
 *
 * \brief Software Cursor
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_sw_cursor.h
 *
 * @brief SW Cursor header file.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_SW_CURSOR_H_
#define SYS_SW_CURSOR_H_


/**
 * \brief Set the GPIO used for the SW cursor to High-Z.
 *
 */
void sys_sw_cursor_setup(void);

/**
 * \brief Triggers the GPIO used for the SW cursor.
 *
 */
void sys_sw_cursor_trigger(void);


#endif /* SYS_SW_CURSOR_H_ */

/**
 * \}
 * \}
 */
