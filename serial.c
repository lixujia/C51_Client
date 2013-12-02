#include <REG51.H>
#include <INTRINS.H>

#include "serial.h"
#include "config.h"

sfr AUXR = 0x8e;
#define T1MS (65536-FOSC/1000) //1ms timer calculation method in 1T mode

char read_buf[32];
BYTE head = 0;
BYTE tail = 0;

BYTE serial_state = 0; // @see SERIAL_STATE_... 

bit busy = 0;

#define READ_TIMEOUT_MS 5 
WORD to_ms = 0;

void t0_int() interrupt 1
{
	  if (to_ms < READ_TIMEOUT_MS) {
			  ++to_ms;
				return;
		}
		
	  TR0 = 0;
		
		serial_state = SERIAL_STATE_DATA_WAIT;
}

/*
 * UART interrupt service routine
 */
void uart_isr() interrupt 4 using 1
{
	
	
    if (RI) {
        RI = 0;
				//TH0 = TL0 = 0xF0;
				TL0 = T1MS; //initial timer0 low byte
				TH0 = T1MS >> 8; //initial timer0 high byte
		    to_ms  = 0;
				
				if (SERIAL_STATE_RECEIVING != serial_state) {
				    TR0 = 1;
            serial_state = SERIAL_STATE_RECEIVING;						
				}
				
				if ((63 == tail && 0 == head) || (head == tail + 1)) {
					  // 循环队列满，覆盖最早的数据
					  ++head;
						head &= 63;
				}
				
				read_buf[tail++] = SBUF;
				tail &= 63;
    }

    if (TI) {
        TI = 0;
        busy = 0;
        ioctl = 0;
    }
}

BYTE serial_consume_char(char* p)
{
	  if (tail == head) {
				return 0;
		}
		
		*p = read_buf[head++];
		head &= 63;
		
		return 1;
}

void SendData(BYTE dat)
{
    while (busy);
    ACC = dat;
    if (P) {
#if (PARITYBIT == ODD_PARITY)
        TB8 = 0;                      // set parity to 0
#elif (PARITYBIT == EVEN_PARITY)
        TB8 = 1;                      // set parity to 1
#endif
    }
    else {
#if (PARITYBIT == ODD_PARITY)
        TB8 = 1;                      // set parity to 1
#elif (PARITYBIT == EVEN_PARITY)
        TB8 = 0;                      // set parity to 0
#endif
    }

	  ioctl = 1;
    busy = 1;
    SBUF = ACC;
}

void SendString(char* s)
{
	  serial_state = SERIAL_STATE_SENDING;
		
    while (*s) {
        SendData(*s++);
    }
}

void serial_init(void)
{
	  AUXR = 0x80; //timer0 work in 1T mode
	  TMOD |= 0x01;
		ET0 = 1; //enable timer0 interrupt
}