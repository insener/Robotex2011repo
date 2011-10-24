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

//////////////////////////////
// Private global constant definitions
//////////////////////////////
#define SYSTEM_MAIN_LOOP_DURATION   30		// one main loop duration
#define SLIDE_WINDOW_COUNT           3
#define WINDOW_NR   				_bufferIndexer
#define NEXT_WINDOW 				if(_bufferIndexer == 2) { _bufferIndexer = 0; } else { _bufferIndexer++; }

//////////////////////////////
// Type definitions
//////////////////////////////

//////////////////////////////
// Private global functions
//////////////////////////////
void makeDecision(void);
void doIdleTime(void);

//////////////////////////////
// Private global variables
//////////////////////////////
GolfBall   	 _slideBuffer[MAX_GOLF_BALLS * SLIDE_WINDOW_COUNT];
GolfBall*  	 _slideBufferPointers[SLIDE_WINDOW_COUNT];    //array of pointers
unsigned int _bufferBallCount[SLIDE_WINDOW_COUNT];		  //stores the number of balls found in frame
int		   	 _bufferIndexer;

/*
 * Initialize image buffer with slide window, so its is always compared 3 last images
 */
void soccer_init(void)
{
	int i;

	for (i = 0; i < SLIDE_WINDOW_COUNT; i++)
	{
		_slideBufferPointers[i] = &_slideBuffer[i * MAX_GOLF_BALLS];
		_bufferBallCount[i] = 0;
	}
	_bufferIndexer = 0;
}

/*
 * Play soccer
 */
void soccer_run(void)
{
	unsigned int timeBase;

	// play soccer forever
	while(1)
	{
		timeBase = systemTime_getSystemTimeBase();

		if (timeBase & SYSTEM_MAIN_LOOP_TIME_BASE)
		{
			*pPORTHIO_TOGGLE = TEST_OUTPUT;
			// take picture
			//camera_grabFrame();
			// find golf balls
			//_bufferBallCount[WINDOW_NR] = colors_searchGolfBalls((unsigned char *)FRAME_BUF, _slideBufferPointers[WINDOW_NR]);
			//NEXT_WINDOW;
		}
		if (timeBase & SYSTEM_3x_MAIN_LOOP_TIME_BASE)
		{
			makeDecision();
		}
		if (timeBase & SYSTEM_12x_MAIN_LOOP_TIME_BASE)
		{
			io_LED2Toggle();
		}
		// keep in loop until 30 ms elapsed from last time
		while (!systemTime_checkMainLoopDuration())
		{
			doIdleTime();
		}
	}
}

/*
 * Makes decision according to image and sensor readings
 */
void makeDecision(void)
{

}

void doIdleTime(void)
{

}
