/*
 * timer.h
 *
 *  Created on: 20.10.2011
 *      Author: peeter
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "config.h"

//////////////////////////////
// Public global constant definitions
//////////////////////////////

//////////////////////////////
// Public global type definitions
//////////////////////////////
enum Timer
{
	TIMER0,
	TIMER1,
	TIMER2,
	TIMER3,
	TIMER4,
	TIMER5,
	TIMER6,
	TIMER7
};

enum TimerMode
{
	INTERNAL = 0,
	PWMOUT,
	WDTHCAP,
	EXTCLK3
};

//////////////////////////////
// Public global functions
//////////////////////////////

/* peripheral timers*/
extern void timer_configureTimer(enum Timer timer, unsigned short config, enum TimerMode type, unsigned long period, unsigned long width);
extern void timer_setTimerWidth(enum Timer timer, unsigned long width);
extern void timer_configureTimerInterrupt(enum Timer timer);
extern void timer_enableTimer(enum Timer timer);
extern void timer_disableTimer(enum Timer timer);
extern void timer_init(void);

//////////////////////////////
// Public global variables
//////////////////////////////

#endif /* TIMER_H_ */
