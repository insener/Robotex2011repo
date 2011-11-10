#ifndef UART_H_
#define UART_H_

#include "config.h"

extern void 		 uart_uart0Init(int);
extern void 		 uart_uart1Init(int);
extern void 		 uart0SendChar(unsigned char s);
extern void 		 uart0SendString(unsigned char *s);
extern void 		 uart0SendChars(unsigned char *s, unsigned int count);
extern unsigned char uart0GetCh(void);
extern unsigned char uart0GetChar(unsigned char *s);
extern void 		 uart0_CTS(int);
extern void 		 uart_uart1SendChar(unsigned char s);
extern void 		 uart_uart1SendString(unsigned char *s);
extern void 		 uart_uart1SendChars(unsigned char *s, unsigned int count);
extern unsigned char uart_uart1GetCh(void);
extern unsigned char uart_uart1GetChar(unsigned char *s);
extern unsigned char uart_uart1Signal(void);
extern void 		 uart_uart2Init(void);
extern void 		 uart_uart2SetCharsToBuffer(unsigned char *cBuf, int numberOfChars);
extern void 		 uart_uart2SendBuffer(void);
extern void 		 uart_uart2ShiftBitOut(void);

#endif
