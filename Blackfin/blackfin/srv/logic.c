/*
 * logic.c
 *
 *  Created on: 28.10.2011
 *      Author: peeter
 */

#include "logic.h"
#include "print.h"
#include "debug.h"
#include "motion.h"

//////////////////////////////
// Type definitions
//////////////////////////////


//////////////////////////////
// Private global defines
//////////////////////////////
#define MIN_TRACEABLE_BALL_DIAM  10   //  minimum traceable ball diameter
#define HISTORY_SIZE 			 0x08
#define BALL_EXISTS_LEVEL  		 15
#define SLIDE_WINDOW_COUNT       3
#define WINDOW_NR   			 _bufferIndexer
#define NEXT_WINDOW 			 if(_bufferIndexer == 2) { _bufferIndexer = 0; } else { _bufferIndexer++; }

//////////////////////////////
// Private global functions
//////////////////////////////
int  directionCalc(int* currentZone, int location);
void setDebugData(int data);

//////////////////////////////
// Private global variables
//////////////////////////////
GolfBall   	 _slideBuffer[MAX_GOLF_BALLS * SLIDE_WINDOW_COUNT];
GolfBall*  	 _slideBufferPointers[SLIDE_WINDOW_COUNT];    //array of pointers
unsigned int _bufferBallCount[SLIDE_WINDOW_COUNT];		  //stores the number of balls found in frame
int		   	 _bufferIndexer;
int 		 _debugIndex;

/*
 * Initialize image buffer with slide window, so its is always compared 3 last images
 */
void logic_init()
{
	int i;

	for (i = 0; i < SLIDE_WINDOW_COUNT; i++)
	{
		_slideBufferPointers[i] = &_slideBuffer[i * MAX_GOLF_BALLS];
		_bufferBallCount[i] = 0;
	}
	_bufferIndexer = 0;
	_debugIndex = 0;
}

/*
 * Performs ball tracking strategy
 */
void logic_traceBall(int ballCount, GolfBall* balls)
{
	static int historyOfBiggestBall[HISTORY_SIZE] = {0};
	static int historyOfBallX[HISTORY_SIZE] = {0};
	static int index = 0;
	static int currentZone = 0;
	int i, decrementHist;
	int ballDiamBiggest = 0;
	int ballExists = 0;
	int direction = 0; // stop
	int temp = 0;

	// find ball with biggest diameter
	for (i = 0; i < ballCount; i++)
	{
		if (balls->ballDiam > ballDiamBiggest)
		{
			ballDiamBiggest = balls->ballDiam;
		}
	}

	// track only balls with at least certain diameter
	if (ballDiamBiggest >= MIN_TRACEABLE_BALL_DIAM)
	{
		// ball exists, overwrite oldest info
		historyOfBiggestBall[index] = 1;
		historyOfBallX[index] = balls->ballX;
	}
	else
	{
		// no ball
		historyOfBiggestBall[index] = 0;
		historyOfBallX[index] = 0;
	}

	// analyze history
	decrementHist = index;
	ballExists = 0;
	for (i = HISTORY_SIZE; i > 0; i--)
	{
		// calculate ball existence probability, this method filters noise.
		// latest measurement has highest weight
		ballExists += historyOfBiggestBall[decrementHist] * i;
		// move to previous history stamp
		if (decrementHist == 0)
		{
			decrementHist = HISTORY_SIZE - 1;
		}
		else
		{
			decrementHist--;
		}
		temp++;
	}

	// give some moving command only when a ball is seen
	if (ballExists >= BALL_EXISTS_LEVEL)
	{
		direction = directionCalc(&currentZone, historyOfBallX[index]);
	}
	else
	{
		direction = 0;
		currentZone = 0;
	}

	//setDebugData(direction);
	if (direction > 0)
	{
		motion_moveSide(100);
	}
	else if (direction < 0)
	{
		motion_moveSide(-100);
	}
	else
	{
		motion_stop();
	}
	// increment general history stamp
	index++;
	if (index >= HISTORY_SIZE)
	{
		index = 0;
	}
}

/*
 * Calculates motion direction, expects image width 320.
 * Location expected range 0...320
 */
int directionCalc(int *currentZone, int location)
{
	int newZone;
	int direction = 0;
	// calculate zone, there are 8 zones (-4, -3, -2, -1, +1, +2, +3, +4), no 0
	if (location >= 320)
	{
		newZone = 4;
	}
	else if (location < 0)
	{
		newZone = -4;
	}
	else
	{
		newZone = location / 40;
		if (newZone <= 3)
		{
			newZone = newZone - 4;
		}
		else
		{
			newZone = newZone - 3;
		}
	}
	// set direction according to zone change
	if ( (newZone == *currentZone) && ((newZone == -1) || (newZone == 1)) )
	{
		direction = 0;
	}
	else if ( (newZone == *currentZone) && (newZone > 1) )
	{
		direction = 1;
	}
	else if ( (newZone == *currentZone) && (newZone < -1) )
	{
		direction = -1;
	}
	else if (newZone > *currentZone)
	{
		direction = 1;
	}
	else if (newZone < *currentZone)
	{
		direction = -1;
	}
	*currentZone = newZone;
	return direction;
}

/*
 * Sets data into local global array, which is printed out later at once.
 * Current buffer size is 12, call get function after every 500 ms
 */
void setDebugData(int data)
{
	debug_setDebugInfo(data, _debugIndex);
	_debugIndex++;
	if (_debugIndex == DEBUG_BUFFER_SIZE)
	{
		_debugIndex = 0;
	}
}
