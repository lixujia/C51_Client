/*
 * modbus.h
 *
 *  Created on: 2013��12��3��
 *      Author: lxj
 */

#ifndef MODBUS_H_
#define MODBUS_H_

#include "config.h"

WORD gen_crc(BYTE const *buffer,BYTE buffer_length);

void modbus_query_process(char msg[],BYTE len);

#endif /* MODBUS_H_ */
