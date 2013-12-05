/*
 * modbus.c
 *
 *  Created on: 2013Äê12ÔÂ3ÈÕ
 *      Author: lxj
 */

#include "config.h"
#include "modbus.h"

BYTE modbus_address = 255;

BYTE (*modbus_read_input_cb)(WORD address,WORD num,WORD arr[]) = NULL;
BYTE (*modbus_read_hold_cb)(WORD address,WORD num,WORD arr[])  = NULL;
BYTE (*modbus_write_hold_cb)(WORD address,WORD num,WORD arr[]) = NULL;
BYTE (*modbus_read_BI_cb)(WORD address,WORD num,WORD arr[])    = NULL;

WORD gen_crc(BYTE const *buffer,BYTE buffer_length)
{
        BYTE c, treat, bcrc;
        WORD wcrc = 0;
        BYTE i, j;

        for (i = 0; i < buffer_length; i++)
        {
                c = buffer[i];

                for (j = 0; j < 8; j++)
                {
                        treat = c & 0x80;
                        c <<= 1;
                        bcrc = (wcrc >> 8) & 0x80;
                        wcrc <<= 1;
                        if (treat != bcrc)
                                wcrc ^= 0x1021;
                }
        }

        return wcrc;
}

void modbus_fault_msg(BYTE arr[],BYTE* out_len)
{
    WORD crc = 0;

    arr[1] += 0x80;

    crc = gen_crc(arr,2);
    arr[2] = crc >> 8;
    arr[3] = crc & 0xFF;

    *out_len = 4;
}

void modbus_process_msg(BYTE arr[],BYTE num,BYTE* out_len)
{
    WORD crc = 0;
    BYTE read_num = 0;
    BYTE data_len = 0;

    if (7 > num || arr[0] != modbus_address)
        return;

    crc = gen_crc(arr,num - 2);

    if (crc != (arr[num - 2] << 8) + (arr[num - 1]))
        return;

    switch (arr[1]) {
    case 2:
        if (NULL == modbus_read_BI_cb || 0 != arr[2] || 0 != arr[4]) {
            modbus_fault_msg(arr,out_len);
            break;
        }

        read_num = arr[5];
        if( 0 != modbus_read_BI_cb((WORD)arr[3],(WORD)read_num,(WORD*)(arr + 3))) {
            modbus_fault_msg(arr,out_len);
        }
        else {
            data_len = read_num >> 3;

            arr[2] = data_len;

            crc = gen_crc(arr,data_len + 3);
            arr[data_len + 3] = crc >> 8;
            arr[data_len + 4] = crc & 0xFF;

            *out_len = data_len + 5;
        }
        break;
    case 3:
        break;
    case 4:
        break;
    default:
        break;
    }
}
