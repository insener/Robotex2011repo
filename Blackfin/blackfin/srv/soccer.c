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
#include "colors.h"
#include "systemTime.h"
#include "print.h"
#include "logic.h"
#include "systemTime.h"
#include "debug.h"
#include "srv.h"
#include "uart.h"

//////////////////////////////
// Private global constant definitions
//////////////////////////////
#define SYSTEM_MAIN_LOOP_DURATION   30		// one main loop duration
#define MEASURE_START 				_executionTime = systemTime_readRTC()
#define MEASURE_STOP				_executionTime = systemTime_readRTC() - _executionTime
#define MEASURE_RESULT				_executionTime

//////////////////////////////
// Type definitions
//////////////////////////////

//////////////////////////////
// Private global functions
//////////////////////////////
void doIdleTime(void);


//////////////////////////////
// Private global variables
//////////////////////////////
int _executionTime = 0;

/*
 * Soccer module initialization
 */
void soccer_init(void)
{

}

/*
 * Play soccer
 */
void soccer_run(void)
{
	unsigned int timeBase, ballCount;
	GolfBall     golfBall;
	int 		 maxTime = 0;
	int* 		 debugInfo;
	unsigned char buf[3] = {0xAA, 0x00, 0xF0};

	// play soccer until play switch turned on
	while(1/*srv_isPlaySwitchOn()*/)
	{
		MEASURE_START;

		timeBase = systemTime_getSystemTimeBase();
		if (timeBase & SYSTEM_MAIN_LOOP_TIME_BASE)
		{
			// take picture
			camera_grabFrame();
			// find golf balls
			ballCount = colors_searchGolfBalls((unsigned char *)FRAME_BUF, &golfBall);
			// trace the ball
			//logic_traceBall(ballCount, &golfBall);
		}
		if (timeBase & SYSTEM_3x_MAIN_LOOP_TIME_BASE)
		{
			//uart_uart2SetCharsToBuffer(buf, 3);
			//uart_uart2SendBuffer();
		}
		if (timeBase & SYSTEM_12x_MAIN_LOOP_TIME_BASE)
		{
			io_LED2Toggle();
			// enable IR receivers
			timer_enableTimer(TIMER3);
			timer_enableTimer(TIMER7);
			printf("left: %d, right: %d", timer_getTimerWidth(TIMER7), timer_getTimerWidth(TIMER3) );
			// debug data
			debugInfo = debug_getDebugInfo();
		/*	printf("%d %d %d %d %d %d %d %d %d %d %d %d", *debugInfo, *(debugInfo + 1), *(debugInfo + 2), *(debugInfo + 3),
								*(debugInfo + 4), *(debugInfo + 5), *(debugInfo + 6), *(debugInfo + 7),
									*(debugInfo + 8), *(debugInfo + 9), *(debugInfo + 10), *(debugInfo + 11) );

		*/
		}
		MEASURE_STOP;
		if ( MEASURE_RESULT > maxTime ) maxTime = MEASURE_RESULT;
		// keep in loop until 30 ms elapsed from last time
		while (!systemTime_checkMainLoopDuration())
		{
			doIdleTime();
		}
	}
}

void doIdleTime(void)
{

}
