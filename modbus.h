/*
 * modbus.h
 *
 *  Created on: 2013Äê12ÔÂ3ÈÕ
 *      Author: lxj
 */

#ifndef MODBUS_H_
#define MODBUS_H_

#include "config.h"

#define MODBUS_FUNCTION_LIGHT  (1 << 0)
#define MODBUS_FUNCTION_SWITCH (1 << 1)
#define MODBUS_FUNCTION_CLOCK  (1 << 2)

WORD gen_crc(BYTE const *buffer,BYTE buffer_length);

extern BYTE modbus_address;
extern BYTE (*modbus_read_input_cb)(WORD address,WORD num,WORD arr[]);
extern BYTE (*modbus_read_hold_cb)(WORD address,WORD num,WORD arr[]);
extern BYTE (*modbus_write_hold_cb)(WORD address,WORD num,WORD arr[]);
extern BYTE (*modbus_read_BI_cb)(WORD address,WORD num,WORD arr[]);

WORD modbus_process_msg(BYTE arr[],BYTE num);

WORD modbus_htons(WORD val);

#endif /* MODBUS_H_ */
