/*
 * modbus.c
 *
 *  Created on: 2013Äê12ÔÂ3ÈÕ
 *      Author: lxj
 */

#include "modbus.h"

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

void modbus_query_process(char msg[],BYTE len)
{
    WORD wcrc = 0;
		
		wcrc = gen_crc(msg,len);
		return;
}
