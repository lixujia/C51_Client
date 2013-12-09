/*
 * modbus.c
 *
 *  Created on: 2013Äê12ÔÂ3ÈÕ
 *      Author: lxj
 */

#include <REG51.H>
#include <INTRINS.H>

#include "config.h"
#include "modbus.h"

BYTE modbus_address = ADDRESS;

BYTE (*modbus_read_input_cb)(WORD address, WORD num, WORD arr[]) = NULL;
BYTE (*modbus_read_hold_cb)(WORD address, WORD num, WORD arr[]) = NULL;
BYTE (*modbus_write_hold_cb)(WORD address, WORD num, WORD arr[]) = NULL;
BYTE (*modbus_read_BI_cb)(WORD address, WORD num, WORD arr[]) = NULL;

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

WORD modbus_fault_msg(BYTE arr[]) {
    WORD crc = 0;

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

WORD modbus_process_read(BYTE arr[], BYTE (*cb)(WORD, WORD, WORD*), BYTE (*length_count)(BYTE)) {
    WORD crc = 0;
    BYTE read_num = 0;
    BYTE data_len = 0;

    if (NULL == cb || NULL == length_count) {
        return modbus_fault_msg(arr);
    }

    read_num = arr[5];
    if (0 != cb((WORD) arr[3], (WORD) read_num, (WORD*) arr + 3)) {
        return modbus_fault_msg(arr);
    }

    data_len = length_count(read_num);

    crc = gen_crc(arr, data_len + 3);
    arr[data_len + 3] = crc >> 8;
    arr[data_len + 4] = crc & 0xFF;

    return data_len + 5;
}

sbit led1 = P1 ^ 0;
sbit led2 = P1 ^ 1;

WORD modbus_process_msg(BYTE arr[], BYTE num) {
    WORD crc = 0;
    BYTE (*cb)(WORD, WORD, WORD[]) = NULL;

    if (7 > num || arr[0] != modbus_address) {
        led1 = 0;
        return 0;
    }

    crc = gen_crc(arr, num - 2);

    if (crc != (arr[num - 2] << 8) + (arr[num - 1])) {
        return 0;
    }

    if (0 != arr[2] || 0 != arr[4]) {
        return modbus_fault_msg(arr);
    }

    switch (arr[1]) {
    case 2:
        return modbus_process_read(arr, modbus_read_BI_cb,
                modbus_read_bi_length);
    case 3:
        return modbus_process_read(arr, modbus_read_hold_cb,
                modbus_read_ai_length);
    case 4:
        return modbus_process_read(arr, modbus_read_input_cb,
                modbus_read_ai_length);
    case 6:
        //cb = modbus_write_hold_cb;
        //break;
    default:
        return modbus_fault_msg(arr);
    }
}
