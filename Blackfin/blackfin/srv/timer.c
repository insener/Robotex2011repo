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

//////////////////////////////
// Type definitions
//////////////////////////////


//////////////////////////////
// Private global defines
//////////////////////////////
#define	INTERRUPT_FREQUENCY	1000
#define	PRESCALE_VALUE		1
#define DECREMENT_VALUE		(CORE_CLOCK / PRESCALE_VALUE) / (INTERRUPT_FREQUENCY)

#define DISABLE_CORE_TIMER_INTERRUPT *pIMASK &= ~EVT_IVTMR; SSYNC;
#define ENABLE_CORE_TIMER_INTERRUPT  *pIMASK |= EVT_IVTMR;  SSYNC;

//////////////////////////////
// Private global functions
//////////////////////////////
void initializeCoreTimer(void);

void INTERRUPT_timer(void) __attribute__((interrupt_handler));
void INTERRUPT_coreTimer(void) __attribute__((interrupt_handler));
//////////////////////////////
// Private global variables
//////////////////////////////
unsigned int _coreTimerCount;

void initializeCoreTimer(void)
{
	_coreTimerCount = 0;
	// Put core timer in active state enable autoreload
	*pTCNTL |= TAUTORLD | TMPWR;
	// Init the timer scale register with divisor - 1
	*pTSCALE = PRESCALE_VALUE - 1;
	// Load count register with number to decrement from
	*pTCOUNT = DECREMENT_VALUE;
	*pTPERIOD = DECREMENT_VALUE;
	// Set the interrupt to call timer_ISR()
	*pEVT6 = INTERRUPT_coreTimer;
	// Unmask the core timer interrupt
	*pIMASK |= EVT_IVTMR;
	//Enable timer
	*pTCNTL |= TMREN;
}

/*
 * timer module initialization
 */
void timer_init(void)
{
	// timer 4 for delay
	timer_configureTimer(TIMER4, PULSE_HI | PWM_OUT | PERIOD_CNT, PWMOUT, PERIPHERAL_CLOCK, PERIPHERAL_CLOCK);
	timer_enableTimer(TIMER4);
	// timer 5 for ball sensor and relay activating
	timer_configureTimer(TIMER5, PERIOD_CNT, INTERNAL, PERIPHERAL_CLOCK, PERIPHERAL_CLOCK);  // 1 sec. period
	timer_configureTimerInterrupt(TIMER5);

	initializeCoreTimer();
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
			*pTIMER3_CONFIG = config;
			*pTIMER3_PERIOD = period;
			*pTIMER3_WIDTH  = width;
			switch(type)
			{
				case PWMOUT:
					// set PWM output
					*pPORTF_FER |= TIMER3_PIN;
					break;
				case INTERNAL:
				default:
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
			*pTIMER7_CONFIG = config;
			*pTIMER7_PERIOD = period;
			*pTIMER7_WIDTH  = width;
			switch(type)
			{
				case PWMOUT:
					// set PWM output
					*pPORTF_FER |= TIMER7_PIN;
					break;
				case INTERNAL:
				default:
					break;
			}
			break;
		default:
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
			*pTIMER0_CONFIG |= IRQ_ENA;
			SSYNC;
			*pTIMER_ENABLE |= TIMEN0;
			SSYNC;
			break;
		case TIMER1:
			*pTIMER1_CONFIG |= IRQ_ENA;
			SSYNC;
			*pTIMER_ENABLE |= TIMEN1;
			SSYNC;
			break;
		case TIMER2:
			*pTIMER2_CONFIG |= IRQ_ENA;
			SSYNC;
			*pTIMER_ENABLE |= TIMEN2;
			SSYNC;
			break;
		case TIMER3:
			*pTIMER3_CONFIG |= IRQ_ENA;
			SSYNC;
			*pTIMER_ENABLE |= TIMEN3;
			SSYNC;
			break;
		case TIMER4:
			*pTIMER4_CONFIG |= IRQ_ENA;
			SSYNC;
			*pTIMER_ENABLE |= TIMEN4;
			SSYNC;
			break;
		case TIMER5:
			*pTIMER5_CONFIG |= IRQ_ENA;
			SSYNC;
			*pTIMER_ENABLE |= TIMEN5;
			SSYNC;
			break;
		case TIMER6:
			*pTIMER6_CONFIG |= IRQ_ENA;
			SSYNC;
			*pTIMER_ENABLE |= TIMEN6;
			SSYNC;
			break;
		case TIMER7:
			*pTIMER7_CONFIG |= IRQ_ENA;
			SSYNC;
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
			*pTIMER0_CONFIG &= ~IRQ_ENA;
			SSYNC;
			*pTIMER_DISABLE |= TIMEN0;
			SSYNC;
			break;
		case TIMER1:
			*pTIMER1_CONFIG &= ~IRQ_ENA;
			SSYNC;
			*pTIMER_DISABLE |= TIMEN1;
			SSYNC;
			break;
		case TIMER2:
			*pTIMER2_CONFIG &= ~IRQ_ENA;
			SSYNC;
			*pTIMER_DISABLE |= TIMEN2;
			SSYNC;
			break;
		case TIMER3:
			*pTIMER3_CONFIG &= ~IRQ_ENA;
			SSYNC;
			*pTIMER_DISABLE |= TIMEN3;
			SSYNC;
			break;
		case TIMER4:
			*pTIMER4_CONFIG &= ~IRQ_ENA;
			SSYNC;
			*pTIMER_DISABLE |= TIMEN4;
			SSYNC;
			break;
		case TIMER5:
			*pTIMER5_CONFIG &= ~IRQ_ENA;
			SSYNC;
			*pTIMER_DISABLE |= TIMEN5;
			SSYNC;
			break;
		case TIMER6:
			*pTIMER6_CONFIG &= ~IRQ_ENA;
			SSYNC;
			*pTIMER_DISABLE |= TIMEN6;
			SSYNC;
			break;
		case TIMER7:
			*pTIMER7_CONFIG &= ~IRQ_ENA;
			SSYNC;
			*pTIMER_DISABLE |= TIMEN7;
			SSYNC;
			break;
		default:
			break;
	}
}

/*
 * Returns the core timer value
 */
unsigned int timer_getCoreTimerValue(void)
{
	unsigned int count;
	DISABLE_CORE_TIMER_INTERRUPT;
	count = _coreTimerCount;
	ENABLE_CORE_TIMER_INTERRUPT;
	return count;
}

/*
 * Timer interrupt routine
 */
__attribute__((interrupt_handler))
void INTERRUPT_timer(void)
{
	unsigned int status;

	status = *pTIMER_STATUS;
	// reached to end of period
	if (status & TIMIL5)
	{
		// clear
		*pTIMER_STATUS |= TIMIL5;
		timer_disableTimer(TIMER5);
		srv_relayOff();
		io_enableBallSensorInterrupt();
	}
	// overflow error
	if (status & TOVL_ERR5)
	{
		// clear
		*pTIMER_STATUS |= TOVL_ERR5;
		timer_disableTimer(TIMER5);
		io_enableBallSensorInterrupt();
	}
	debug_setDebugInfo(status);
}

/*
 * Core timer interrupt routine, increments timer's value.
 * Should happen after every 1 ms.
 */
__attribute__((interrupt_handler))
void INTERRUPT_coreTimer(void)
{
	// do not afraid the overflow as program never runs up to 50 days
	_coreTimerCount++;
}