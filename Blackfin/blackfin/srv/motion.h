/*
 * motion.h
 *
 *  Created on: 05.10.2011
 *      Author: peeter
 */

#ifndef MOTION_H_
#define MOTION_H_

extern void motion_moveStraight(char speed);
extern void motion_moveSide(char speed);
extern void motion_moveXY(char xSpeed, char ySpeed);
extern void motion_stop();

#endif /* MOTION_H_ */
