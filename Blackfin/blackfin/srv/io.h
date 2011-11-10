/*
 * io.h
 *
 *  Created on: 18.10.2011
 *      Author: peeter
 */

#ifndef IO_H_
#define IO_H_

#include "config.h"

//////////////////////////////
// Public global constant definitions
//////////////////////////////
#define LED1				PG8
#define LED2				PG9

#define TIMER0_PIN			PF9
#define TIMER1_PIN  		PF8
#define TIMER2_PIN  		PF7  //  external pin  7
#define TIMER3_PIN  		PF6  //  external pin  8
#define TIMER4_PIN  		PF5
#define TIMER5_PIN  		PF4
#define TIMER6_PIN  		PF3  //  external pin  5
#define TIMER7_PIN  		PF2  //  external pin  6


#define MATCHPORT_RTS0		PH0  //  external pin 17
#define BATTERY				PH2  //  external pin 19
#define RELAY_DIRECTION		PH4  //  external pin 21
#define UART0_FLOW_CRTL		PH6  //  external pin 23
#define LASER2				PH9  //  external pin 26
#define UART2_TX			PH10 //  external pin 27
//#define TEST_OUTPUT			UART2_TX
#define BALL_SENSOR			PH11 //  external pin 28
#define IR_DIST_LEFT        PH12 //  external pin 29
#define IR_DIST_RIGHT       PH13 //  external pin 30
#define PLAY_SWITCH_OUT		PH14 //  external pin 31
#define PLAY_SWITCH_IN		PH15 //  external pin 32

#define DISABLE_BALL_SENSOR_INTERRUPT  *pPORTHIO_MASKA_CLEAR = BALL_SENSOR;
#define ENABLE_BALL_SENSOR_INTERRUPT   *pPORTHIO_MASKA_SET  = BALL_SENSOR;

//////////////////////////////
// Public global type definitions
//////////////////////////////


//////////////////////////////
// Public global functions
//////////////////////////////
extern void 		io_initPeriphery(void);
extern unsigned int io_getBallSensorCount(void);
extern void 		io_LED1Set(void);
extern void 		io_LED1Clear(void);
extern void 		io_LED1Toggle(void);
extern void 		io_LED2Set(void);
extern void 		io_LED2Clear(void);
extern void 		io_LED2Toggle(void);
extern void			io_enableBallSensorInterrupt(void);
extern void			io_disableBallSensorInterrupt(void);
extern void 		io_togglePortHPin(int pin);
extern void 		io_setPortHPin(int pin);
extern void 		io_clearPortHPin(int pin);
//////////////////////////////
// Public global variables
//////////////////////////////


#endif /* IO_H_ */
