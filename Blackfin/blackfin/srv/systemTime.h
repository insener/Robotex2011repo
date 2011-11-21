/*
 * systemTime.h
 *
 *  Created on: 27.05.2011
 *      Author: peeter
 */

#ifndef SYSTEMTIME_H_
#define SYSTEMTIME_H_

#include "config.h"

//////////////////////////////
// Public global constant definitions
//////////////////////////////
#define MAIN_LOOP_DURATION   				40
#define LONGEST_TIME_BASE_MULTIPLIER		12
#define SYSTEM_MAIN_LOOP_TIME_BASE			0x01   // 40 ms
#define SYSTEM_3x_MAIN_LOOP_TIME_BASE     	0x02   //  3 x 40 = 120 ms
#define SYSTEM_12x_MAIN_LOOP_TIME_BASE     	0x04   // 12 x 40 = 500 ms

//////////////////////////////
// Public global type definitions
//////////////////////////////


//////////////////////////////
// Public global functions
//////////////////////////////
extern void 		systemTime_init(void);
extern int  		systemTime_readRTC(void);
extern void 		systemTime_clearRTC(void);
extern void 		systemTime_delayMs(int delay);
extern void 		systemTime_delayUS(int delay);
extern void 		systemTime_delayNS(int delay);
extern unsigned int systemTime_getSystemTimeBase(void);
extern int			systemTime_checkMainLoopDuration(void);

//////////////////////////////
// Public global variables
//////////////////////////////

#endif /* SYSTEMTIME_H_ */
