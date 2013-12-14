#ifndef CONFIG_H
#define CONFIG_H

typedef unsigned char BYTE;
typedef unsigned int  WORD;


//#define FOSC 18432000L          // System Frequency
//#define   FOSC 11059200L          // System Frequency
//#define   FOSC 11071692L
#define   FOSC 22118400L
#define BAUD 9600                 // UART baudrate

#define ADDRESS 1

#define PARITYBIT NONE_PARITY

#endif
