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
#include "systemTime.h"
#include "srv.h"

//////////////////////////////
// Private global defines
//////////////////////////////
#define MIN_TRACEABLE_BALL_DIAM  4   //  minimum traceable ball diameter
#define HISTORY_SIZE 			 0x08
#define BALL_EXISTS_LEVEL  		 15
#define SLIDE_WINDOW_COUNT       3
#define WINDOW_NR   			 _bufferIndexer
#define NEXT_WINDOW 			 if(_bufferIndexer == 2) { _bufferIndexer = 0; } else { _bufferIndexer++; }
#define MIDDLE_LEFT             130
#define MIDDLE_RIGHT            190

//////////////////////////////
// Type definitions
//////////////////////////////
enum LogicState
{
    stateStart,
    stateGoalkeeper,
    stateAttack
};

enum GoalSearchingState
{
    stateGoalInit,
    stateGoalOnLeft,
    stateGoalOnRight,
    stateGoalInMiddle,
    stateGoalScore
};

enum GoalAttackingState
{
    stateMoveStop,
    stateMoveLeft,
    stateMoveRight,
    stateMoveForward,
    stateMoveBackward
};


//////////////////////////////
// Private global functions
//////////////////////////////
int  directionCalc(int* currentZone, int location);
void setDebugData(int data);
void moveToPreviousDataInHistory(int *historyIndex);
void traceBall(int ballCount, GolfBall* balls);
void attack(int ballCount, GolfBall* balls, Goal* goalOpponent, Goal* goalOwn);
int  getStateAccordingToOpponentGoalPosition(int x);
int  getStateAccordingToOwnGoalPosition(int x);

//////////////////////////////
// Private global variables
//////////////////////////////
GolfBall   	 _slideBuffer[MAX_GOLF_BALLS * SLIDE_WINDOW_COUNT];
GolfBall*  	 _slideBufferPointers[SLIDE_WINDOW_COUNT];    //array of pointers
unsigned int _bufferBallCount[SLIDE_WINDOW_COUNT];		  //stores the number of balls found in frame
int		   	 _bufferIndexer;
int 		 _debugIndex;
int          _logicState;
int          _startTime;

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
	_logicState = stateAttack;
	// save the start time moment
	_startTime = systemTime_readRTC();
}

/*
 * Performs ball tracking strategy
 */
void traceBall(int ballCount, GolfBall* balls)
{
	static int historyOfBiggestBall[HISTORY_SIZE] = {0};
	static int historyOfBallX[HISTORY_SIZE] = {0};
	static int historyOfDirection[HISTORY_SIZE] = {0};
	static int index = 0;
	static int currentZone = 0;
	int i, decrementHist;
	int ballDiamBiggest = 0;
	int biggestBallIndex = -1;
	int ballExists = 0;
	int frontDist = 0;
	int backDist = 0;
	int leftDist = 0;
	int rightDist = 0;

	// get sensors' readings
	srv_getDistanceSensorResults(&frontDist, &backDist, &leftDist, &rightDist);

	// find ball with biggest diameter
	for (i = 0; i < ballCount; i++)
	{
		if (balls[i].ballDiam > ballDiamBiggest)
		{
			ballDiamBiggest = balls[i].ballDiam;
			biggestBallIndex = i;
		}
	}

	// track only balls with at least certain diameter
	if (ballDiamBiggest >= MIN_TRACEABLE_BALL_DIAM)
	{
		// ball exists, overwrite oldest info
		historyOfBiggestBall[index] = 1;
	}
	else
	{
		// no ball
		historyOfBiggestBall[index] = 0;
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
		moveToPreviousDataInHistory(&decrementHist);
	}

	// give some moving command only when a ball is seen
	if (ballExists >= BALL_EXISTS_LEVEL)
	{
		if (historyOfBiggestBall[index] == 1)
		{
			historyOfBallX[index] = balls[biggestBallIndex].ballX;
		}
		else
		{
			// ball not seen in last frame, take previous state X coordinate
			decrementHist = index;
			moveToPreviousDataInHistory(&decrementHist);
			historyOfBallX[index] = historyOfBallX[decrementHist];
		}
		historyOfDirection[index] = directionCalc(&currentZone, historyOfBallX[index]);
	}
	// remain moving direction as long as it is in history
	else if (ballExists > 0)
	{
		historyOfBallX[index] = 0;
		decrementHist = index;
		moveToPreviousDataInHistory(&decrementHist);
		historyOfDirection[index] = historyOfDirection[decrementHist];
		currentZone = 0;
	}
	// ball has not seen for a long time, so stop
	else
	{
		historyOfBallX[index] = 0;
		historyOfDirection[index] = 0;
		currentZone = 0;
	}

	//setDebugData(currentZone);

	// actuate motors according to direction and sensors
	if (historyOfDirection[index] > 0)
	{
		if (rightDist == OBSTACLE)
		{
		    motion_stop();
		}
		else if (backDist == OBSTACLE)
		{
		    motion_moveSideForward(MOVE_RIGHT);
		}
		else
		{
		    motion_moveSideBackward(MOVE_RIGHT);
		}
	}
	else if (historyOfDirection[index] < 0)
	{
	    if (leftDist == OBSTACLE)
        {
            motion_stop();
        }
	    else if (backDist == OBSTACLE)
	    {
	        motion_moveSideForward(MOVE_LEFT);
	    }
        else
        {
            motion_moveSideBackward(MOVE_LEFT);
        }
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
 * Decrements index to refer to previous history data
 */
void moveToPreviousDataInHistory(int *historyIndex)
{
	if (*historyIndex == 0)
	{
		*historyIndex = HISTORY_SIZE - 1;
	}
	else
	{
		*historyIndex = *historyIndex - 1;
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
 * Handles the state machine
 */
void logic_handleLogic(int ballCount, GolfBall* balls, Goal* goalOpponent, Goal* goalOwn)
{
    _logicState = stateAttack;
    switch(_logicState)
    {
        case stateStart:
            // at first move to goal keeper position, 4 s from start.
            if ((systemTime_readRTC() - _startTime) < 3000)
            {
                motion_moveSideForward(MOVE_LEFT);

         /*   }
            else
            {
                if (srv_isBlueGoalSelected())
                {
                    // find the opponent goal and set the position according to it
                    if (goal->exists)
                    {
                        if (goal->x < 120)
                        {
                            motion_drift(MOVE_LEFT);
                        }
                        else if (goal->x > 200)
                        {
                            motion_drift(MOVE_RIGHT);
                        }
                        else
                        {
                            motion_stop();
                        }
                    }
                    else
                    {
                        // turn to one direction
                        motion_drift(MOVE_RIGHT);
                    }
                }
                else
                {
                    logic_traceBall(ballCount, balls);
                }*/
            }
            break;
        case stateAttack:
            attack(ballCount, balls, goalOpponent, goalOwn);
            break;
        case stateGoalkeeper:
            traceBall(ballCount, balls);
        default:
            break;
    }

}

/*
 * Attacking strategy. Keep opponent's goal always in view
 */
void attack(int ballCount, GolfBall* balls, Goal* goalOpponent, Goal* goalOwn)
{
    static int goalSearchingState = stateGoalInit;
    static int timeStampScore = -1;
    static int goalAttacState = stateMoveStop;
    int frontDist, backDist, leftDist, rightDist;
    int i;
    int ballDiamBiggest = 0;
    int biggestBallIndex = -1;

    srv_getDistanceSensorResults(&frontDist, &backDist, &leftDist, &rightDist);
    // find ball with biggest diameter
    for (i = 0; i < ballCount; i++)
    {
        if (balls[i].ballDiam > ballDiamBiggest)
        {
            ballDiamBiggest = balls[i].ballDiam;
            biggestBallIndex = i;
        }
    }

    // handle state machine
    switch(goalSearchingState)
    {
        case stateGoalInit:
            // find opponent's goal
            if (goalOpponent->exists)
            {
                goalSearchingState = getStateAccordingToOpponentGoalPosition(goalOpponent->x);
                goalAttacState = stateMoveForward;
            }
            else
            {
                // no opponent's goal on the view, maybe own goal is
                if (goalOwn->exists)
                {
                    //turn according to my own goal
                    goalSearchingState = getStateAccordingToOwnGoalPosition(goalOwn->x);
                    goalAttacState = stateMoveForward;
                }
                else
                {
                    // turn right to find any goal
                    goalSearchingState = stateGoalOnRight;
                }
            }
            break;
        case stateGoalOnLeft:
            // do action
            if (frontDist == OBSTACLE_NON)
            {
                // drift left if allowed
                if (leftDist == OBSTACLE_NON)
                {
                    motion_drift(MOVE_LEFT);
                }
                else
                {
                    motion_moveSideForward(MOVE_RIGHT);
                }
            }
            // get feedback and make new decision
            if (goalOpponent->exists)
            {
                goalSearchingState = getStateAccordingToOpponentGoalPosition(goalOpponent->x);
                goalAttacState = stateMoveForward;
                // maybe i scored
              /*  if (frontDist == OBSTACLE)
                {
                    // GOAL!!!
                    goalSearchingState = stateGoalScore;
                }*/
            }
            // no else, do the same action again until goal is found
            break;
        case stateGoalOnRight:
            // do action
            if (frontDist == OBSTACLE_NON)
            {
                // drift right if allowed
                if (rightDist == OBSTACLE_NON)
                {
                    motion_drift(MOVE_RIGHT);
                }
                else
                {
                    motion_moveSideForward(MOVE_LEFT);
                }
            }
            // get feedback and make new decision
            if (goalOpponent->exists)
            {
                goalSearchingState = getStateAccordingToOpponentGoalPosition(goalOpponent->x);
                goalAttacState = stateMoveForward;

                // maybe i scored
             /*   if (frontDist == OBSTACLE)
                {
                    // GOAL!!!
                    goalSearchingState = stateGoalScore;
                }*/
            }
            else
            {
                // right turn can discover own goal
                if (goalOwn->exists)
                {
                    //turn according to my own goal
                    goalSearchingState = getStateAccordingToOwnGoalPosition(goalOwn->x);
                    goalAttacState = stateMoveForward;
                }
                // else stay in the same state
            }
            break;
        case stateGoalInMiddle:
            // check if we reached the goal
            if (frontDist == OBSTACLE_NON)
            {
                // do action depending on moving state
                switch (goalAttacState)
                {
                    case stateMoveForward:
                        motion_moveStraight(MOVE_FORWARD);
                        break;
                    case stateMoveBackward:
                        motion_moveStraight(MOVE_BACKWARD);
                        break;
                    case stateMoveLeft:
                        if (leftDist == OBSTACLE_NON)
                        {
                            motion_moveSideForward(MOVE_LEFT);
                        }
                        else
                        {
                            motion_moveStraight(MOVE_FORWARD);
                        }
                        break;
                    case stateMoveRight:
                        if (rightDist == OBSTACLE_NON)
                        {
                            motion_moveSideForward(MOVE_RIGHT);
                        }
                        else
                        {
                            motion_moveStraight(MOVE_FORWARD);
                        }
                        break;
                    case stateMoveStop:
                        motion_stop();
                        break;
                    default:
                        break;
                }
                if (goalOpponent->exists)
                {
                    // if ball exists
                    if (biggestBallIndex >= 0)
                    {
                        if (balls[biggestBallIndex].ballX < MIDDLE_LEFT)
                        {
                            goalAttacState = stateMoveLeft;
                        }
                        else if (balls[biggestBallIndex].ballX > MIDDLE_RIGHT)
                        {
                            goalAttacState = stateMoveRight;
                        }
                        else
                        {
                            goalAttacState = stateMoveForward;
                        }
                    }
                    else
                    {
                        // move back
                        goalAttacState = stateMoveBackward;
                    }
                }
                // else - probably a ball is hiding the goal. Continue moving

            }
            else
            {
                goalSearchingState = stateGoalScore;
            }
            break;
        case stateGoalScore:
            // do action
            if (backDist == OBSTACLE_NON)
            {
                motion_moveStraight(MOVE_BACKWARD);
                // move back some seconds
                if (timeStampScore == -1)
                {
                    // take initial time stamp
                    timeStampScore = systemTime_readRTC();
                }
                else
                {
                    // wait until time elapsed
                    if ((systemTime_readRTC() - timeStampScore) > 3000)
                    {
                        timeStampScore = -1;
                        goalSearchingState = stateGoalInit;
                    }
                }
            }
            else
            {
                goalSearchingState = stateGoalInit;
            }
            break;
        default:
            // should not be here
            break;
    }
}

/*
 * Gives the state according to opponent's goal.
 * Expects that goal is visible and coordinate is between valid range.
 */
int getStateAccordingToOpponentGoalPosition(int x)
{
    int state;
    // turn toward opponent goal
    if (x < MIDDLE_LEFT)
    {
        state = stateGoalOnLeft;
    }
    else if (x > MIDDLE_RIGHT)
    {
        state = stateGoalOnRight;
    }
    else
    {
        state = stateGoalInMiddle;
    }
    return state;
}

/*
 * Gives the state according to own goal.
 * Expects that goal is visible and coordinate is between valid range.
 */
int getStateAccordingToOwnGoalPosition(int x)
{
    int state;
    // turn away from own goal
    if (x < MIDDLE_LEFT)
    {
        state = stateGoalOnRight;
    }
    else if (x > MIDDLE_RIGHT)
    {
        state = stateGoalOnLeft;
    }
    else
    {
        state = stateGoalOnRight;
    }
    return state;
}

#ifdef DEBUG
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

#endif
