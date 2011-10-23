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

//////////////////////////////
// Private global defines
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
void doIdleTime(void);
void makeDecision(void);

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
	_pointerToPointers = _slideBufferPointers;
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
	// play soccer forever
	while(1)
	{
		// take picture
		camera_grabFrame();
		// find golf balls
		_bufferBallCount[WINDOW_NR] = colors_searchGolfBalls((unsigned char *)FRAME_BUF, _slideBufferPointers[WINDOW_NR]);
		NEXT_WINDOW;
		makeDesicion();
		// keep in loop until 30 ms elapsed from last time
		doIdleTime();
	}
}

/*
 * Makes decision according to image and sensor readings
 */
void makeDecision(void)
{

}

/*
 * Do idle time until next 30 ms is elapsed
 */
void doIdleTime(void)
{
	static unsigned int coreTimerCurrent = 0;
	static unsigned int coreTimerMainLoop = 0;
	do
	{
		coreTimerCurrent = timer_getCoreTimerValue();
	}
	while ((coreTimerCurrent - coreTimerMainLoop) >= SYSTEM_MAIN_LOOP_DURATION);
	coreTimerMainLoop = coreTimerCurrent;
	*pPORTHIO_TOGGLE = TEST_OUTPUT;
}
