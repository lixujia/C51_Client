/*
 * modbus.c
 *
 *  Created on: 2013年12月3日
 *      Author: lxj
 */

#include <REG51.H>
#include <INTRINS.H>
#include <STRING.H>

#include "config.h"
#include "modbus.h"

BYTE modbus_address = ADDRESS;

WORD gen_crc(BYTE const *buffer, BYTE buffer_length) {
    BYTE c, treat, bcrc;
    WORD wcrc = 0;
    BYTE i, j;

    for (i = 0; i < buffer_length; i++) {
        c = buffer[i];

        for (j = 0; j < 8; j++) {
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

WORD modbus_fault_msg(BYTE* arr) {
    WORD  crc = 0;
	
    arr[1] += 0x80;

    crc = gen_crc(arr, 2);
    arr[2] = crc >> 8;
    arr[3] = crc & 0xFF;

    return 4;
}

BYTE modbus_read_bi_length(BYTE read_num) {
    return read_num >> 3;
}

BYTE modbus_read_ai_length(BYTE read_num) {
    return read_num << 1;
}

sbit led1 = P1 ^ 0;
WORD tmp[16];

WORD modbus_process_read(BYTE* arr) {
    WORD  crc = 0;
    BYTE  read_num = 0;
    BYTE  data_len = 0;

    read_num = arr[5];

    if (0x03 == arr[1] && 0 != modbus_read_hold_cb(arr[3],read_num,(WORD*)(arr + 3))) {
        return modbus_fault_msg(arr);
    }
    else if (0x04 == arr[1] && 0 != modbus_read_input_cb(arr[3],read_num,(WORD*)(arr + 3))) {
        return modbus_fault_msg(arr);
    }

    data_len = modbus_read_ai_length(read_num);
    arr[2] = data_len;

    crc = gen_crc(arr, data_len + 3);
    arr[data_len + 3] = crc >> 8;
    arr[data_len + 4] = crc & 0xFF;

    return data_len + 5;
}

WORD modbus_process_write(BYTE* arr,BYTE (*cb)(WORD, WORD, WORD*)) {
    WORD address = 0;
    WORD number = 0;
    WORD crc = 0;

    if (NULL == cb) {
        return modbus_fault_msg(arr);
    }

    // 预置单个寄存器
    if (6 == arr[1]) {
        address = modbus_ntohs(*((WORD*)(arr + 2)));

        if (0 != cb(address,1,(WORD*)(arr + 4))) {
            return modbus_fault_msg(arr);
        }

        return 8;
    }

    // 预置多个寄存器
    else if (16 == arr[1]) {
        address = modbus_ntohs(*((WORD*)(arr + 2)));
        number  = modbus_ntohs(*((WORD*)(arr + 4)));

        if (0 != cb(address,number,(WORD*)(arr + 7))) {
            return modbus_fault_msg(arr);
        }

        crc = gen_crc(arr,6);
        *((WORD*)(arr + 6)) = modbus_htons(crc);

        return 8;
    }

    return modbus_fault_msg(arr);
}

WORD modbus_process_msg(BYTE* arr, BYTE num) {
    WORD crc = 0;
    //WORD iRet = 0;

    if (7 > num || arr[0] != modbus_address) {
        return 0;
    }

    crc = gen_crc(arr, num - 2);

    if (crc != (arr[num - 2] << 8) + (arr[num - 1])) {
        return 0;
    }

    if (0 != arr[2]) {
        return modbus_fault_msg(arr);
    }

    switch (arr[1]) {
    case 3:
    case 4:
        return modbus_process_read(arr);
    case 6:
    case 16:
        return modbus_process_write(arr,modbus_write_hold_cb);
    default:
        return modbus_fault_msg(arr);
    }
}

WORD modbus_htons(WORD val)
{
    WORD xdata endian = 0x00FF;

    if (0 == *((BYTE*)&endian)) {
        return val;
    }

    return ((val >> 8) & 0x00FF) + ((val << 8) & 0xFF00);
}
