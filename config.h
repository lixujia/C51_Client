#ifndef CONFIG_H
#define CONFIG_H

typedef unsigned char BYTE;
typedef unsigned int  WORD;

#define NULL ((void*)0)

//#define FOSC 18432000L          // System Frequency
//#define   FOSC 11059200L          // System Frequency
//#define   FOSC 11071692L
#define   FOSC 22137200L
#define BAUD 9600                 // UART baudrate



#define PARITYBIT EVEN_PARITY

#endif
