#ifndef SERIAL_H
#define SERIAL_H

#include "config.h"

/* Define UART Parity Mode */
#define NONE_PARITY 0
#define ODD_PARITY  1
#define EVEN_PARITY 2
#define MARK_PARITY 3
#define SPACE_PARITY 4

sbit ioctl = P3^2;

#define SERIAL_BUF_LEN 128
extern char xdata read_buf[SERIAL_BUF_LEN];
extern BYTE data_len;

extern bit busy;

#define SERIAL_STATE_IDLE      0
#define SERIAL_STATE_SENDING   1
#define SERIAL_STATE_RECEIVING 2
#define SERIAL_STATE_DATA_WAIT 3
extern BYTE serial_state;

void serial_init(void);
void SendBuf(char s[],BYTE len);

BYTE serial_consume_char(char* p);
#endif
