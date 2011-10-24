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


//////////////////////////////
// Public global type definitions
//////////////////////////////


//////////////////////////////
// Public global functions
//////////////////////////////
extern void motion_moveStraight(char speed);
extern void motion_moveSide(char speed);
extern void motion_moveXY(char xSpeed, char ySpeed);
extern void motion_stop();

//////////////////////////////
// Public global variables
//////////////////////////////


#endif /* MOTION_H_ */
