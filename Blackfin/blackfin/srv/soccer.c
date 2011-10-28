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

//////////////////////////////
// Private global constant definitions
//////////////////////////////
#define SYSTEM_MAIN_LOOP_DURATION   30		// one main loop duration


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

	// play soccer forever
	while(1)
	{
		timeBase = systemTime_getSystemTimeBase();

		if (timeBase & SYSTEM_MAIN_LOOP_TIME_BASE)
		{
			*pPORTHIO_TOGGLE = TEST_OUTPUT;
			// take picture
			camera_grabFrame();
			// find golf balls
			ballCount = colors_searchGolfBalls((unsigned char *)FRAME_BUF, &golfBall);
			// trace the ball
			logic_traceBall(ballCount, &golfBall);
		}
		if (timeBase & SYSTEM_3x_MAIN_LOOP_TIME_BASE)
		{

		}
		if (timeBase & SYSTEM_12x_MAIN_LOOP_TIME_BASE)
		{
			io_LED2Toggle();
			printf("%d", timer_getTimerWidth(TIMER3));
			timer_enableTimer(TIMER3);
		}
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
