#include <REG51.H>
#include <INTRINS.H>
#include <STDLIB.H>
#include <STRING.H>

#include "serial.h"
#include "config.h"

sfr AUXR = 0x8e;
#define T5MS (65536-FOSC/1000 * 5) //1ms timer calculation method in 1T mode

BYTE xdata malloc_mempool [256];

#if 0
extern BYTE serial_buf[SERIAL_BUF_LEN];
extern BYTE  deal_buf[SERIAL_BUF_LEN];
#else
BYTE* serial_buf = NULL;
BYTE*   deal_buf = NULL;
#endif

BYTE serial_len = SERIAL_BUF_LEN;
BYTE  deal_len  = SERIAL_BUF_LEN;



BYTE serial_state = 0; // @see SERIAL_STATE_... 

bit busy = 0;

//#define READ_TIMEOUT_MS 1
//WORD to_ms = 0;

void t0_int()
#ifndef ECLIPSE_EDITOR
interrupt 1
#endif
{
//    if (to_ms < READ_TIMEOUT_MS) {
//        ++to_ms;
//        return;
//    }

    TR0 = 0;

    serial_state = SERIAL_STATE_DATA_WAIT;
}


		
/*
 * UART interrupt service routine
 */
void uart_isr()
#ifndef ECLIPSE_EDITOR
interrupt 4 using 1
#endif
{
    if (RI) {
        //tmp = SBUF;
        RI = 0;

        //TH0 = TL0 = 0xF0;
        TL0 = T5MS;//initial timer0 low byte
        TH0 = T5MS >> 8;//initial timer0 high byte

        if (SERIAL_STATE_RECEIVING != serial_state) {
            TR0 = 1;
            serial_state = SERIAL_STATE_RECEIVING;
        }

        if (SERIAL_BUF_LEN == serial_len) {
            // 队列满，丢弃数据
            ;
        }
        else {
            serial_buf[serial_len++] = SBUF;
        }
    }

    if (TI) {
        TI = 0;
        busy = 0;
        ioctl = 0;
    }
}

void SendData(BYTE dat) {
    while (busy)
        ;
    ACC = dat;
    if (P) {
#if (PARITYBIT == ODD_PARITY)
        TB8 = 0;                      // set parity to 0
#elif (PARITYBIT == EVEN_PARITY)
        TB8 = 1;                      // set parity to 1
#endif
    } else {
#if (PARITYBIT == ODD_PARITY)
        TB8 = 1;                      // set parity to 1
#elif (PARITYBIT == EVEN_PARITY)
        TB8 = 0;                      // set parity to 0
#endif
    }

    SBUF = ACC;
    ioctl = 1;
    busy = 1;
}

void SendBuf(char* s,BYTE len) {
    BYTE i = 0;

    serial_state = SERIAL_STATE_SENDING;

    for (i = 0; i < len; ++i) {
        SendData(s[i]);
    }
}

void serial_init(void) {
#if (PARITYBIT == NONE_PARITY)
    SCON = 0x50;
#elif (PARITYBIT == ODD_PARITY) || (PARITYBIT == EVEN_PARITY) || (PARITYBIT == MARK_PARITY)
    SCON = 0xDA;
#elif (PARITYBIT == SPACE_PARITY)
    SCON = 0xD5;
#endif

    AUXR = 0x80; //timer0 work in 1T mode
    TMOD |= 0x01;
    ET0 = 1; //enable timer0 interrupt

    init_mempool (&malloc_mempool, sizeof(malloc_mempool));
    serial_buf = malloc(SERIAL_BUF_LEN);
    deal_buf   = malloc(SERIAL_BUF_LEN);
    serial_len = SERIAL_BUF_LEN;
    deal_len   = SERIAL_BUF_LEN;
}
