#include <REG51.H>
#include <INTRINS.H>

typedef unsigned char BYTE;
typedef unsigned int  WORD;

//#define FOSC 18432000L          // System Frequency
//#define   FOSC 11059200L          // System Frequency
//#define   FOSC 11071692L
#define   FOSC 22137200L
#define BAUD 9600                 // UART baudrate

/* Define UART Parity Mode */
#define NONE_PARITY 0
#define ODD_PARITY  1
#define EVEN_PARITY 2
#define MARK_PARITY 3
#define SPACE_PARITY 4

#define PARITYBIT EVEN_PARITY

sbit bit9 = P2^2;
bit busy;

sbit led1 = P1^0;
sbit led2 = P1^1;
sbit led3 = P1^2;
sbit led4 = P1^3;

sbit ioctl = P3^2;

/* Declare SFR associated with the PCA */
sfr CCON = 0xD8;
sbit CCF0 = CCON^0;
sbit CCF1 = CCON^1;
sbit CR   = CCON^6;
sbit CF   = CCON^7;
sfr CMOD  = 0xD9;
sfr CL    = 0xE9;
sfr CH    = 0xF9;
sfr CCAPM0 = 0xDA;
sfr CCAP0L = 0xEA;
sfr CCAP0H = 0xFA;
sfr CCAPM1 = 0xDB;
sfr CCAP1L = 0xEB;
sfr CCAP1H = 0xFB;
sfr PCAPWM0 = 0xF2;
sfr PCAPWM1 = 0xF3;


/*
 * UART interrupt service routine
 */
void uart_isr() interrupt 4 using 1
{
    char a;

    if (RI) {
        RI = 0;
        a = SBUF;
        P1 = 0xFF;

        CR = 1;

        if ('0' >= a) {
            CCAP0H = CCAP0L = 0;
        }
        else if ('9' < a) {
            CCAP0H = CCAP0L = 0xFF;
            CR = 0;
        }
        else {
            CCAP0H = CCAP0L = 0xFF * (a - '0') / 10;
        }
    }

    if (TI) {
        TI = 0;
        busy = 0;
        ioctl = 0;
    }
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
    while (*s) {
        SendData(*s++);
    }
}

void main()
{
#if (PARITYBIT == NONE_PARITY)
    SCON = 0x50;
#elif (PARITYBIT == ODD_PARITY) || (PARITYBIT == EVEN_PARITY) || (PARITYBIT == MARK_PARITY)
    SCON = 0xDA;
#elif (PARITYBIT == SPACE_PARITY)
    SCON = 0xD5;
#endif

    /* pwm */
    CCON = 0;
    CL = 0;
    CH = 0;
    CMOD = 0x02;
    CCAP0H = CCAP0L = 0x20;
    CCAPM0 = 0x42;

    ioctl = 0;

    TMOD = 0x20;             // Set time1 as 8-bit auto reload mode.
    TH1  = TL1 = -(FOSC/12/32/BAUD); //Set auto reload value.
    TR1  = 1;                // Time1 start run.
    ES   = 1;                // Enable UART interrupt.
    EA   = 1;                // Open master interrupt switch.

    CR = 1;                  // PCA timer start run

    SendString("STC12C5A60S2\r\nUart Test!\r\n");
    while (1);
}
