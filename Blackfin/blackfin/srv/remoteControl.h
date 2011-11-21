/*
 * remoteControl.h
 *
 *  Created on: 24.10.2011
 *      Author: pes
 */

#ifndef REMOTECONTORL_H_
#define REMOTECONTROL_H_

//////////////////////////////
// Public global constant definitions
//////////////////////////////
#define PLAY_SOCCER   1
#define STOP_SOCCER   0

//////////////////////////////
// Public global type definitions
//////////////////////////////


//////////////////////////////
// Public global functions
//////////////////////////////
extern void remoteControl_run(void);
extern int  remoteControl_getPlayCommandState(void);
extern void remoteControl_setPlayCommandState(int command);

//////////////////////////////
// Public global variables
//////////////////////////////

#endif /* REMOTECONTORL_H_ */
