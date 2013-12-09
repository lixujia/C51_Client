/*
 * modbus.h
 *
 *  Created on: 2013Äê12ÔÂ3ÈÕ
 *      Author: lxj
 */

#ifndef MODBUS_H_
#define MODBUS_H_

#include "config.h"

WORD gen_crc(BYTE const *buffer,BYTE buffer_length);

extern BYTE modbus_address;
extern BYTE (*modbus_read_input_cb)(WORD address,WORD num,WORD arr[]);
extern BYTE (*modbus_read_hold_cb)(WORD address,WORD num,WORD arr[]);
extern BYTE (*modbus_write_hold_cb)(WORD address,WORD num,WORD arr[]);
extern BYTE (*modbus_read_BI_cb)(WORD address,WORD num,WORD arr[]);

WORD modbus_process_msg(BYTE arr[],BYTE num);

#endif /* MODBUS_H_ */
