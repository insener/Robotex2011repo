/*
 * rtc.h
 *
 *  Created on: 27.05.2011
 *      Author: peeter
 */

#ifndef RTC_H_
#define RTC_H_

#include <cdefBF537.h>
#include "config.h"

//////////////////////////////
// Public global functions
//////////////////////////////
extern void rtc_init(void);
extern void rtc_initTimer4(void);
extern int  rtc_read(void);
extern void rtc_clear(void);
extern void rtc_delayMS(int delay);
extern void rtc_delayUS(int delay);
extern void rtc_delayNS(int delay);

#endif /* RTC_H_ */
