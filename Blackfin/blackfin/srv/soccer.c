/*
 * soccer.c
 *
 *  Created on: 16.10.2011
 *      Author: peeter
 */
#include "soccer.h"
#include "camera.h"
#include "timer.h"
#include "io.h"

//////////////////////////////
// Type definitions
//////////////////////////////
#define SYSTEM_10_MS_INTERVAL   0x01
#define SYSTEM_25_MS_INTERVAL   0x02
#define SYSTEM_30_MS_INTERVAL   0x04
#define SYSTEM_100_MS_INTERVAL  0x08
#define SYSTEM_500_MS_INTERVAL  0x10

//////////////////////////////
// Private global defines
//////////////////////////////


//////////////////////////////
// Private global functions
//////////////////////////////


//////////////////////////////
// Private global variables
//////////////////////////////
unsigned int coreTimerCurrent = 0;
unsigned int coreTimerLast = 0;
unsigned int systemDevider = 0;

void soccer_run(void)
{
	// play soccer forever
	while(1)
	{
		coreTimerCurrent = timer_getCoreTimerValue();
		if ((coreTimerCurrent - coreTimerLast) >= 30)
		{
			coreTimerLast = coreTimerCurrent;
			systemDevider |= SYSTEM_30_MS_INTERVAL;
		}
		if (systemDevider & SYSTEM_30_MS_INTERVAL)
		{
			systemDevider &= ~SYSTEM_30_MS_INTERVAL; // clear the bit
			//camera_grabFrame();
			*pPORTHIO_TOGGLE = TEST_OUTPUT;
		}
	}

}
