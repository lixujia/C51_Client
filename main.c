#include <REG51.H>
#include <INTRINS.H>
#include <STRING.H>

#include "serial.h"
#include "modbus.h"
#include "config.h"


/* Declare SFR associated with the PCA */
sfr CCON = 0xD8;
//sbit CCF0 = CCON ^ 0;
//sbit CCF1 = CCON ^ 1;
sbit CR = CCON ^ 6;
//sbit CF = CCON ^ 7;
sfr CMOD = 0xD9;
sfr CL = 0xE9;
sfr CH = 0xF9;
sfr CCAPM0 = 0xDA;
sfr CCAP0L = 0xEA;
sfr CCAP0H = 0xFA;
//sfr CCAPM1 = 0xDB;
//sfr CCAP1L = 0xEB;
//sfr CCAP1H = 0xFB;
//sfr PCAPWM0 = 0xF2;
//sfr PCAPWM1 = 0xF3;

WORD  out_len = 0;
WORD  counter = 0x0000;

WORD  current = 0xFFFF;
WORD  target  = 0xFFFF;
WORD  step    = 0xFFFF;

BYTE modbus_read_input_cb(WORD address, WORD num, WORD* arr)
{
    if (1 != address || 1 != num) {
        return 0xFF;
    }

    arr[0] = modbus_htons(MODBUS_FUNCTION_LIGHT);

    return 0;
}

BYTE modbus_read_hold_cb(WORD address,WORD num,WORD* arr)
{
    BYTE i = 0;

    if (address + num > 4 || address < 1) {
        return 0xFF;
    }

    for (i = 0; i < num; ++i) {
        switch (address + i) {
        case 1:
            arr[i] = modbus_htons(current);
            break;
        case 2:
            arr[i] = modbus_htons(target);
            break;
        case 3:
            arr[i] = modbus_htons(step);
            break;
        default:
            return 0xFF;
        }
    }

    return 0;
}

BYTE modbus_write_hold_cb(WORD address,WORD num,WORD* arr)
{
    BYTE i = 0;

    if (address + num > 4 || address < 1) {
        return 0xFF;
    }

    for (i = 0; i < num; ++i) {
        switch (address + i) {
        case 1:
            current = modbus_ntohs(arr[i]);
            break;
        case 2:
            target = modbus_ntohs(arr[i]);
            break;
        case 3:
            step = modbus_ntohs(arr[i]);
            break;
        default:
            return 0xFF;
        }
    }

    return 0;
}

void main() {
    /* pwm */
    CCON = 0;
    CL = 0;
    CH = 0;
    CMOD = 0x02;

    CCAP0H = CCAP0L = 0xFF;
	
    CCAPM0 = 0x42;

    ioctl = 0;

    TMOD = 0x20;                     // Set time1 as 8-bit auto reload mode.
    TH1 = TL1 = -(FOSC / 12 / 32 / BAUD); //Set auto reload value.
    TR1 = 1;                        // Time1 start run.
    ES = 1;                        // Enable UART interrupt.
    EA = 1;                        // Open master interrupt switch.

    serial_init();

    CR = 1;                  // PCA timer start run

    while (1) {
        //memcpy(serial_buf,"\x01\x03\x00\x01\x00\x03\xAC\x21",8);
        //serial_len = 8;
        //serial_state = SERIAL_STATE_DATA_WAIT;

        if (SERIAL_STATE_DATA_WAIT == serial_state) {
            deal_len = serial_len;
            memcpy(deal_buf,serial_buf,deal_len);
            out_len = modbus_process_msg(deal_buf,deal_len);

            if (0 != out_len) {
                SendBuf(deal_buf,out_len);
            }

            serial_len = 0;
            serial_state = SERIAL_STATE_IDLE;
        }
        else if (current != target){
            if (current < target) {
                if (0xFFFF - counter < step) {
                    ++current;

                    CCAP0H = current >> 8;
                    CCAP0L = current & 0xFF;
                }

                counter += step;
            }
            else if (current > target) {
                if (counter < step) {
                    --current;

                    CCAP0H = current >> 8;
                    CCAP0L = current & 0xFF;
                }

                counter -= step;
            }
        }

        if (0xFFFF == current) {
            CR = 0;
        }
        else {
            CR = 1;
        }
    }
}
