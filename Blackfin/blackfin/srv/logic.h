/*
 * logic.h
 *
 *  Created on: 28.10.2011
 *      Author: peeter
 */

#ifndef LOGIC_H_
#define LOGIC_H_

#include "colors.h"
//////////////////////////////
// Public global constant definitions
//////////////////////////////

//////////////////////////////
// Public global type definitions
//////////////////////////////


//////////////////////////////
// Public global functions
//////////////////////////////
extern void logic_init(void);
extern void logic_handleLogic(int ballCount, GolfBall* balls, Goal* goalOpponent, Goal* goalOwn);

//////////////////////////////
// Public global variables
//////////////////////////////

#endif /* LOGIC_H_ */
