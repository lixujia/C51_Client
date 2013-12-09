#include <REG51.H>
#include <INTRINS.H>

#include "serial.h"
#include "modbus.h"
#include "config.h"


/* Declare SFR associated with the PCA */
sfr CCON = 0xD8;
sbit CCF0 = CCON ^ 0;
sbit CCF1 = CCON ^ 1;
sbit CR = CCON ^ 6;
sbit CF = CCON ^ 7;
sfr CMOD = 0xD9;
sfr CL = 0xE9;
sfr CH = 0xF9;
sfr CCAPM0 = 0xDA;
sfr CCAP0L = 0xEA;
sfr CCAP0H = 0xFA;
sfr CCAPM1 = 0xDB;
sfr CCAP1L = 0xEB;
sfr CCAP1H = 0xFB;
sfr PCAPWM0 = 0xF2;
sfr PCAPWM1 = 0xF3;

char xdata test_buf[] = "hello!";
char xdata test_buf2[] = "world!";

sbit led2 = P1^1;

void main() {
    WORD out_len = 0;
	BYTE input = 0;
	BYTE light = 0;

    /* pwm */
    CCON = 0;
    CL = 0;
    CH = 0;
    CMOD = 0x02;
    CCAP0H = CCAP0L = 0x20;
    CCAPM0 = 0x42;

    ioctl = 0;

    TMOD = 0x20;                     // Set time1 as 8-bit auto reload mode.
    TH1 = TL1 = -(FOSC / 12 / 32 / BAUD); //Set auto reload value.
    TR1 = 1;                        // Time1 start run.
    ES = 1;                        // Enable UART interrupt.
    EA = 1;                        // Open master interrupt switch.

    serial_init();
    modbus_address = ADDRESS;

    CR = 1;                  // PCA timer start run

    SendBuf(test_buf,5);

    while (1) {
        if (SERIAL_STATE_DATA_WAIT == serial_state) {
            led2 = light;
            light = !light;

            P2 = data_len;
            out_len = modbus_process_msg(read_buf,data_len);

            if (0 != out_len) {
                SendBuf(read_buf,out_len);
                //SendBuf(test_buf,6);
                //SendBuf(test_buf2,6);
            }

            data_len = 0;
            serial_state = SERIAL_STATE_IDLE;
        }
    }
}
