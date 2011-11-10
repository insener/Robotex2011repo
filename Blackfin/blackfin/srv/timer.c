/*
 * timer.c
 *
 *  Created on: 20.10.2011
 *      Author: peeter
 */

#include "timer.h"
#include "srv.h"
#include "io.h"
#include "debug.h"
#include "uart.h"

//////////////////////////////
// Private global constant definitions
//////////////////////////////
#define TIMER_COUNT  8


//////////////////////////////
// Type definitions
//////////////////////////////


//////////////////////////////
// Private global functions
//////////////////////////////
void INTERRUPT_timer(void) __attribute__((interrupt_handler));
void disableTimerInterrupt(enum Timer timer);
void enableTimerInterrupt(enum Timer timer);

//////////////////////////////
// Private global variables
//////////////////////////////
unsigned long _timerPwmWidths[TIMER_COUNT] = {0};


/*
 * timer module initialization
 */
void timer_init(void)
{

}

/*
 * Configure one of 8 timers
 */
void timer_configureTimer(enum Timer timer, unsigned short config, enum TimerMode type, unsigned long period, unsigned long width)
{
	switch (timer)
	{
		case TIMER0:
			*pTIMER0_CONFIG = config;
			*pTIMER0_PERIOD = period;
			*pTIMER0_WIDTH  = width;
			switch(type)
			{
				case PWMOUT:
					// set PWM output
					*pPORTF_FER |= TIMER0_PIN;
					break;
				case INTERNAL:
				default:
					break;
			}
			break;
		case TIMER1:
			*pTIMER1_CONFIG = config;
			*pTIMER1_PERIOD = period;
			*pTIMER1_WIDTH  = width;
			switch(type)
			{
				case PWMOUT:
					// set PWM output
					*pPORTF_FER |= TIMER1_PIN;
					break;
				case INTERNAL:
				default:
					break;
			}
			break;
		case TIMER2:
			*pTIMER2_CONFIG = config;
			*pTIMER2_PERIOD = period;
			*pTIMER2_WIDTH  = width;
			switch(type)
			{
				case PWMOUT:
					// set PWM output
					*pPORTF_FER |= TIMER2_PIN;
					break;
				case INTERNAL:
				default:
					break;
			}
			break;
		case TIMER3:
			// enable timer functionality
			*pPORTF_FER |= TIMER3_PIN;
			switch(type)
			{
				case WDTHCAP:
					*pTIMER3_PERIOD = period;
					*pTIMER3_WIDTH  = width;
					*pTIMER3_CONFIG = config;
					break;
				case PWMOUT:
				case INTERNAL:
				default:
					*pTIMER3_CONFIG = config;
					*pTIMER3_PERIOD = period;
					*pTIMER3_WIDTH  = width;
					break;
			}
			break;
		case TIMER4:
			*pTIMER4_CONFIG = config;
			*pTIMER4_PERIOD = period;
			*pTIMER4_WIDTH  = width;
			switch(type)
			{
				case PWMOUT:
					// set PWM output
					*pPORTF_FER |= TIMER4_PIN;
					break;
				case INTERNAL:
				default:
					break;
			}
			break;
		case TIMER5:
			*pTIMER5_CONFIG = config;
			*pTIMER5_PERIOD = period;
			*pTIMER5_WIDTH  = width;
			switch(type)
			{
				case PWMOUT:
					// set PWM output
					*pPORTF_FER |= TIMER5_PIN;
					break;
				case INTERNAL:
				default:
					break;
			}
			break;
		case TIMER6:
			*pTIMER6_CONFIG = config;
			*pTIMER6_PERIOD = period;
			*pTIMER6_WIDTH  = width;
			switch(type)
			{
				case PWMOUT:
					// set PWM output
					*pPORTF_FER |= TIMER6_PIN;
					break;
				case INTERNAL:
				default:
					break;
			}
			break;
		case TIMER7:
			*pPORTF_FER |= TIMER7_PIN;
			switch(type)
			{
				case WDTHCAP:
					*pTIMER7_PERIOD = period;
					*pTIMER7_WIDTH  = width;
					*pTIMER7_CONFIG = config;
					break;
				case PWMOUT:
				case INTERNAL:
				default:
					*pTIMER7_CONFIG = config;
					*pTIMER7_PERIOD = period;
					*pTIMER7_WIDTH  = width;
					break;
			}
			break;
	}
}

/*
 * Sets timer's width. It is especially necessary with timers in PWM mode
 */
void timer_setTimerWidth(enum Timer timer, unsigned long width)
{
	switch (timer)
	{
		case TIMER0:
			*pTIMER0_WIDTH  = width;
			break;
		case TIMER1:
			*pTIMER1_WIDTH  = width;
			break;
		case TIMER2:
			*pTIMER2_WIDTH  = width;
			break;
		case TIMER3:
			*pTIMER3_WIDTH  = width;
			break;
		case TIMER4:
			*pTIMER4_WIDTH  = width;
			break;
		case TIMER5:
			*pTIMER5_WIDTH  = width;
			break;
		case TIMER6:
			*pTIMER6_WIDTH  = width;
			break;
		case TIMER7:
			*pTIMER7_WIDTH  = width;
			break;
		default:
			break;
	}
}

/*
 * Configure timer interrupt
 */
void timer_configureTimerInterrupt(enum Timer timer)
{
	switch (timer)
	{
		case TIMER0:
			*pTIMER0_CONFIG |= IRQ_ENA;
			SSYNC;
			*pSIC_IMASK |= IRQ_TIMER0;
			SSYNC;
			break;
		case TIMER1:
			*pTIMER1_CONFIG |= IRQ_ENA;
			SSYNC;
			*pSIC_IMASK |= IRQ_TIMER1;
			SSYNC;
			break;
		case TIMER2:
			*pTIMER2_CONFIG |= IRQ_ENA;
			SSYNC;
			*pSIC_IMASK |= IRQ_TIMER2;
			SSYNC;
			break;
		case TIMER3:
			*pTIMER3_CONFIG |= IRQ_ENA;
			SSYNC;
			*pSIC_IMASK |= IRQ_TIMER3;
			SSYNC;
			break;
		case TIMER4:
			*pTIMER4_CONFIG |= IRQ_ENA;
			SSYNC;
			*pSIC_IMASK |= IRQ_TIMER4;
			SSYNC;
			break;
		case TIMER5:
			*pTIMER5_CONFIG |= IRQ_ENA;
			SSYNC;
			*pSIC_IMASK |= IRQ_TIMER5;
			SSYNC;
			break;
		case TIMER6:
			*pTIMER6_CONFIG |= IRQ_ENA;
			SSYNC;
			*pSIC_IMASK |= IRQ_TIMER6;
			SSYNC;
			break;
		case TIMER7:
			*pTIMER7_CONFIG |= IRQ_ENA;
			SSYNC;
			*pSIC_IMASK |= IRQ_TIMER7;
			SSYNC;
			break;
		default:
			break;
	}
	*pEVT12 = INTERRUPT_timer;
	SSYNC;
	*pIMASK |= EVT_IVG12;
	SSYNC;
}

/*
 * Enables given timer
 */
void timer_enableTimer(enum Timer timer)
{
	switch (timer)
	{
		case TIMER0:
			enableTimerInterrupt(TIMER0);
			*pTIMER_ENABLE |= TIMEN0;
			SSYNC;
			break;
		case TIMER1:
			enableTimerInterrupt(TIMER1);
			*pTIMER_ENABLE |= TIMEN1;
			SSYNC;
			break;
		case TIMER2:
			enableTimerInterrupt(TIMER2);
			*pTIMER_ENABLE |= TIMEN2;
			SSYNC;
			break;
		case TIMER3:
			enableTimerInterrupt(TIMER3);
			*pTIMER_ENABLE |= TIMEN3;
			SSYNC;
			break;
		case TIMER4:
			enableTimerInterrupt(TIMER4);
			*pTIMER_ENABLE |= TIMEN4;
			SSYNC;
			break;
		case TIMER5:
			enableTimerInterrupt(TIMER5);
			*pTIMER_ENABLE |= TIMEN5;
			SSYNC;
			break;
		case TIMER6:
			enableTimerInterrupt(TIMER6);
			*pTIMER_ENABLE |= TIMEN6;
			SSYNC;
			break;
		case TIMER7:
			enableTimerInterrupt(TIMER7);
			*pTIMER_ENABLE |= TIMEN7;
			SSYNC;
			break;
		default:
			break;
	}
}

/*
 * Disables given timer and its interrupt
 */
void timer_disableTimer(enum Timer timer)
{
	switch (timer)
	{
		case TIMER0:
			disableTimerInterrupt(TIMER0);
			*pTIMER_DISABLE |= TIMEN0;
			SSYNC;
			break;
		case TIMER1:
			disableTimerInterrupt(TIMER1);
			*pTIMER_DISABLE |= TIMEN1;
			SSYNC;
			break;
		case TIMER2:
			disableTimerInterrupt(TIMER2);
			*pTIMER_DISABLE |= TIMEN2;
			SSYNC;
			break;
		case TIMER3:
			disableTimerInterrupt(TIMER3);
			*pTIMER_DISABLE |= TIMEN3;
			SSYNC;
			break;
		case TIMER4:
			disableTimerInterrupt(TIMER4);
			*pTIMER_DISABLE |= TIMEN4;
			SSYNC;
			break;
		case TIMER5:
			disableTimerInterrupt(TIMER5);
			*pTIMER_DISABLE |= TIMEN5;
			SSYNC;
			break;
		case TIMER6:
			disableTimerInterrupt(TIMER6);
			*pTIMER_DISABLE |= TIMEN6;
			SSYNC;
			break;
		case TIMER7:
			disableTimerInterrupt(TIMER7);
			*pTIMER_DISABLE |= TIMEN7;
			SSYNC;
			break;
		default:
			break;
	}
}

/*
 * Disables given timer interrupt
 */
void disableTimerInterrupt(enum Timer timer)
{
	switch (timer)
	{
		case TIMER0:
			*pTIMER0_CONFIG &= ~IRQ_ENA;
			SSYNC;
			break;
		case TIMER1:
			*pTIMER1_CONFIG &= ~IRQ_ENA;
			SSYNC;
			break;
		case TIMER2:
			*pTIMER2_CONFIG &= ~IRQ_ENA;
			SSYNC;
			break;
		case TIMER3:
			*pTIMER3_CONFIG &= ~IRQ_ENA;
			SSYNC;
			break;
		case TIMER4:
			*pTIMER4_CONFIG &= ~IRQ_ENA;
			SSYNC;
			break;
		case TIMER5:
			*pTIMER5_CONFIG &= ~IRQ_ENA;
			SSYNC;
			break;
		case TIMER6:
			*pTIMER6_CONFIG &= ~IRQ_ENA;
			SSYNC;
			break;
		case TIMER7:
			*pTIMER7_CONFIG &= ~IRQ_ENA;
			SSYNC;
			break;
		default:
			break;
	}
}

/*
 * Enables given timer interrupt
 */
void enableTimerInterrupt(enum Timer timer)
{
	switch (timer)
	{
		case TIMER0:
			*pTIMER0_CONFIG |= IRQ_ENA;
			SSYNC;
			break;
		case TIMER1:
			*pTIMER1_CONFIG |= IRQ_ENA;
			SSYNC;
			break;
		case TIMER2:
			*pTIMER2_CONFIG |= IRQ_ENA;
			SSYNC;
			break;
		case TIMER3:
			*pTIMER3_CONFIG |= IRQ_ENA;
			SSYNC;
			break;
		case TIMER4:
			*pTIMER4_CONFIG |= IRQ_ENA;
			SSYNC;
			break;
		case TIMER5:
			*pTIMER5_CONFIG |= IRQ_ENA;
			SSYNC;
			break;
		case TIMER6:
			*pTIMER6_CONFIG |= IRQ_ENA;
			SSYNC;
			break;
		case TIMER7:
			*pTIMER7_CONFIG |= IRQ_ENA;
			SSYNC;
			break;
		default:
			break;
	}
}

/*
 * Timer interrupt routine
 */
__attribute__((interrupt_handler))
void INTERRUPT_timer(void)
{
	unsigned int status;

	status = *pTIMER_STATUS;
	// timer 3 -> end of pulse measurement
	if (status & TIMIL3)
	{
		// clear the flag
		*pTIMER_STATUS |= TIMIL3;
		timer_disableTimer(TIMER3);
		_timerPwmWidths[TIMER3] = *pTIMER3_WIDTH;
	}
	// timer 3 overflow error
	if (status & TOVL_ERR3)
	{
		// clear the flag
		*pTIMER_STATUS |= TOVL_ERR3;
		timer_disableTimer(TIMER3);
	}
	// reached to end of period -> shift out next character
	if (status & TIMIL5)
	{
		// clear the flag
		*pTIMER_STATUS |= TIMIL5;

		uart_uart2ShiftBitOut();
	}
	// timer 5 overflow error
	if (status & TOVL_ERR5)
	{
		// clear the flag
		*pTIMER_STATUS |= TOVL_ERR5;
		timer_disableTimer(TIMER5);
	}
	if (status & TIMIL7)
	{
		// clear the flag
		*pTIMER_STATUS |= TIMIL7;
		timer_disableTimer(TIMER7);
		_timerPwmWidths[TIMER7] = *pTIMER7_WIDTH;
	}
	// timer 7 overflow error
	if (status & TOVL_ERR7)
	{
		// clear the flag
		*pTIMER_STATUS |= TOVL_ERR7;
		timer_disableTimer(TIMER7);
	}
}

/*
 * Gets the timer's width. It is used in width capture mode
 */
unsigned long timer_getTimerWidth(enum Timer timer)
{
	unsigned long width;
	disableTimerInterrupt(timer);
	width = _timerPwmWidths[timer];
	enableTimerInterrupt(timer);
	return width;
}




