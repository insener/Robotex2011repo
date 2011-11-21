/*
 * motion.h
 *
 *  Created on: 05.10.2011
 *      Author: peeter
 */

#ifndef MOTION_H_
#define MOTION_H_

//////////////////////////////
// Public global constant definitions
//////////////////////////////
#define MOVE_LEFT     -100
#define MOVE_RIGHT     100
#define MOVE_FORWARD   100
#define MOVE_BACKWARD -100

//////////////////////////////
// Public global type definitions
//////////////////////////////
enum Direction
{
	stop,
	forward,
	backward,
	leftForward,
	rightForward,
	leftBackward,
	rightBackward,
	leftDrift,
	rightDrift
};

//////////////////////////////
// Public global functions
//////////////////////////////
extern void motion_moveStraight(char speed);
extern void motion_moveSideForward(char speed);
extern void motion_moveSideBackward(char speed);
extern void motion_moveXY(char xSpeed, char ySpeed);
extern void motion_drift(char speed);
extern void motion_stop();

//////////////////////////////
// Public global variables
//////////////////////////////


#endif /* MOTION_H_ */
