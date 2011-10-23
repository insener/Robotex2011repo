#ifndef UART_H_
#define UART_H_

#include "config.h"

void 			uart_initUart0(int);
void 			uart_initUart1(int);
void 			uart0SendChar(unsigned char s);
void 			uart0SendString(unsigned char *s);
void 			uart0SendChars(unsigned char *s, unsigned int count);
unsigned char 	uart0GetCh();
unsigned char 	uart0GetChar(unsigned char *s);
unsigned char 	uart0Signal();
void 			uart_uart1SendChar(unsigned char s);
void 			uart_uart1SendString(unsigned char *s);
void 			uart_uart1SendChars(unsigned char *s, unsigned int count);
unsigned char 	uart_uart1GetCh();
unsigned char 	uart_uart1GetChar(unsigned char *s);
unsigned char 	uart_uart1Signal();
void 			printNumber(unsigned char base, unsigned char noDigits, unsigned char sign, unsigned char pad, int number);

void 			uart0_CTS(int);

#endif
