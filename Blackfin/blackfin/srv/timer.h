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
// Public global defines
//////////////////////////////
#define TIMER0_PIN	PF9
#define TIMER1_PIN  PF8
#define TIMER2_PIN  PF7
#define TIMER3_PIN  PF6
#define TIMER4_PIN  PF5
#define TIMER5_PIN  PF4
#define TIMER6_PIN  PF3
#define TIMER7_PIN  PF2
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
// Public global variables
//////////////////////////////

//////////////////////////////
// Public global functions
//////////////////////////////

/* core timer (system timer)*/
extern unsigned int timer_getCoreTimerValue(void);

/* peripheral timers*/
extern void timer_configureTimer(enum Timer timer, unsigned short config, enum TimerMode type, unsigned long period, unsigned long width);
extern void timer_setTimerWidth(enum Timer timer, unsigned long width);
extern void timer_configureTimerInterrupt(enum Timer timer);
extern void timer_enableTimer(enum Timer timer);
extern void timer_disableTimer(enum Timer timer);
extern void timer_init(void);

#endif /* TIMER_H_ */
