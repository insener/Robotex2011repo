/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  uart.c - uart functions for the SRV-1 Blackfin robot.
 *    Copyright (C) 2005-2009  Surveyor Corporation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details (www.gnu.org/licenses)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "uart.h"
#include "srv.h"
#include "systemTime.h"
#include "timer.h"
#include "io.h"

//////////////////////////////
// Private global constant definitions
//////////////////////////////
#define SUART_SEND0 *pPORTHIO |= 0x4000
#define SUART_SEND1 *pPORTHIO &= 0xBFFF
#define SUART_RECV  (*pPORTHIO & 0x8000)
#define UART2_SHIFT_COUNT 10
#define UART2_BUFFER	  10

//////////////////////////////
// Type definitions
//////////////////////////////


//////////////////////////////
// Private global functions
//////////////////////////////


//////////////////////////////
// Private global variables
//////////////////////////////
unsigned int  _uart2Chars[UART2_BUFFER] = {0};   // it is an 'int' buffer, because stop and start bits will be added
unsigned int  _uart2ByteCount = 0;
unsigned int  _uart2ShiftCount = 0;

void waituntilNS(unsigned int target) {
    if (target > PERIPHERAL_CLOCK) {
        target -= PERIPHERAL_CLOCK;
        while (*pTIMER4_COUNTER > target)  // wait for timer to wrap-around
            continue;
    }
    while (*pTIMER4_COUNTER < target)
        continue;
}

/*
 * Initializes UART0 module
 */
void uart_uart0Init(int baudrate)
{
    int uart_divider;

    uart_divider = (((MASTER_CLOCK * VCO_MULTIPLIER) / SCLK_DIVIDER) / 16) / baudrate;
    *pPORTF_FER |= PF0 | PF1; //0x0003;  // enable UART0 pins
    *pUART0_GCTL = UCEN;
    *pUART0_LCR = DLAB;
    *pUART0_DLL = uart_divider;
    *pUART0_DLH = uart_divider >> 8;
    *pUART0_LCR = WLS(8); // 8 bit, no parity, one stop bit

    // dummy reads to clear possible pending errors / irqs
    char dummy = *pUART0_RBR;
    dummy = *pUART0_LSR;
    dummy = *pUART0_IIR;
    SSYNC;
}

void uart0_CTS(int ix)  // set ~CTS signal.  1 = clear to send   0 = not clear to send
{
    if (ix == 0)
        *pPORTHIO |= 0x0040;  // block incoming data 
    else
        *pPORTHIO &= 0xFFBF;  // allow incoming data 
}

/*
 * Initializes UART1 module
 */
void uart_uart1Init(int baudrate)
{
    int uart_divider;

    uart_divider = (((MASTER_CLOCK * VCO_MULTIPLIER) / SCLK_DIVIDER) / 16) / baudrate;
    *pPORTF_FER |= PF2 | PF3; //0x000C;  // enable UART1 pins
    *pUART1_GCTL = UCEN;
    *pUART1_LCR = DLAB;
    *pUART1_DLL = uart_divider;
    *pUART1_DLH = uart_divider >> 8;
    *pUART1_LCR = WLS(8); // 8 bit, no parity, one stop bit

    char dummy = *pUART1_RBR;
    dummy = *pUART1_LSR;
    dummy = *pUART1_IIR;
    SSYNC;
}

void uart0SendChar(unsigned char c)
{
#ifdef HW_FLOW_CONTROL
    while (*pPORTHIO & 0x0001)  // hardware serial flow control - 
        continue;               //    S32 pin 17 should be grounded to disable
#endif
    while (!(*pUART0_LSR & THRE))
        continue;
    *pUART0_THR = c;
}

void uart0SendString(unsigned char *s)
{
    char a;
    while ((a = *s++)) {
        uart0SendChar(a);
    }
}

void uart0SendChars(unsigned char *buf, unsigned int size)
{
    while (size--) {
        uart0SendChar(*buf++);
    }
}

unsigned char uart0GetCh()
{
    while (!(*pUART0_LSR & DR));
    return *pUART0_RBR;
}

unsigned char uart0GetChar(unsigned char *a)
{
    if (!(*pUART0_LSR & DR))
        return 0;
    *a = *pUART0_RBR;
    return 1;
}

void uart_uart1SendChar(unsigned char c)
{
    while (!(*pUART1_LSR & THRE));
    *pUART1_THR = c;
}

void uart_uart1SendString(unsigned char *s)
{
    char a;
    while ((a = *s++)) {
        uart_uart1SendChar(a);
    }
}

void uart_uart1SendChars(unsigned char *buf, unsigned int size)
{
    while (size--) {
        uart_uart1SendChar(*buf++);
    }
}

unsigned char uart_uart1GetCh()
{
    while (!(*pUART1_LSR & DR));
        return *pUART1_RBR;
}

unsigned char uart_uart1GetChar(unsigned char *a)
{
    if (!(*pUART1_LSR & DR))
        return 0;
    *a = *pUART1_RBR;
    return 1;
}

unsigned char uart_uart1Signal()
{
    if (!(*pUART1_LSR & DR))
        return 0;
    return 1;
}

/*
 * Initializes UART2 module, which is done purely in SW using one of the regular GPIOs.
 * Fixed baud rate: 38400, 1 start bit, 8 data bits, 1 stop bit, no parity
 */
void uart_uart2Init(void)
{
	unsigned long period = (PERIPHERAL_CLOCK / 100);  // try 3250
	unsigned long width = (PERIPHERAL_CLOCK / 100);

	// timer 5 for generating software UART clock, disable output
	timer_configureTimer(TIMER5, PWM_OUT | PERIOD_CNT | OUT_DIS, PWMOUT, period, width);  // 26 us period
	timer_configureTimerInterrupt(TIMER5);
}

/*
 * Copies characters into UART2 buffer to shift them out.
 * Start and stop bits are added.
 */
void uart_uart2SetCharsToBuffer(unsigned char *cBuf, int numberOfChars)
{
	unsigned int i, character;

	timer_disableTimer(TIMER5);
	if (numberOfChars <= UART2_BUFFER)
	{
		for (i = 0; i < numberOfChars; i++)
		{
			character = *(cBuf + i);
			// add start and stop bits
			character = (character << 1) & ~0x01;
			character |= 0x200;
			// add to buffer
			_uart2Chars[i] = character;
		}
		_uart2ByteCount = numberOfChars;
	}
}

/*
 * Sends given characters, enable UART clock generating timer
 */
void uart_uart2SendBuffer(void)
{
	timer_enableTimer(TIMER5);
}

/*
 * Shifts out the bit of the character into GPIO pin.
 * NB!!! Called only in the interrupt routine
 */
void uart_uart2ShiftBitOut(void)
{
	// check if any bytes to send
	if (_uart2ByteCount != 0)
	{
		if (_uart2ShiftCount < (UART2_SHIFT_COUNT - 1))
		{
			// send a bit
			if (_uart2Chars[_uart2ByteCount - 1] & 0x01)
			{
				io_setPortHPin(UART2_TX);
			}
			else
			{
				io_clearPortHPin(UART2_TX);
			}
			// shift next bit for sending
			_uart2Chars[_uart2ByteCount - 1] = _uart2Chars[_uart2ByteCount - 1] >> 1;
			_uart2ShiftCount++;
		}
		else if (_uart2ShiftCount == UART2_SHIFT_COUNT - 1)
		{
			// stop bit is high
			io_setPortHPin(UART2_TX);
			// if last byte, then disable timer
			if (_uart2ByteCount == 1)
			{
				timer_disableTimer(TIMER5);
				_uart2ByteCount = 0;
			}
			else
			{
				// take next byte
				_uart2ByteCount--;
			}
			// clear the shift count anyway
			_uart2ShiftCount = 0;
		}
		else
		{
			// should not happen, but just in case
			timer_disableTimer(TIMER5);
			_uart2ByteCount = 0;
			_uart2ShiftCount = 0;
		}
	}
}



