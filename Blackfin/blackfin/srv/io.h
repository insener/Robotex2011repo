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
// Public global defines
//////////////////////////////
#define LED1				0x0100 //0b0000000100000000  // PG8
#define LED2				0x0200 //0b0000001000000000	 // PG9

#define MATCHPORT_RTS0		0x0001 //0b0000000000000001  // PH0
#define BALL_SENSOR			0x8000 //0b0000000000000010  // PH1/PH15
#define BATTERY				0x0004 //0b0000000000000100  // PH2
#define UART0_FLOW_CRTL		0x0040 //0b0000000001000000  // PH6
#define TEST_OUTPUT			0x4000  // PH14

#define DISABLE_BALL_SENSOR_INTERRUPT  *pPORTHIO_MASKA_CLEAR = BALL_SENSOR;
#define ENABLE_BALL_SENSOR_INTERRUPT   *pPORTHIO_MASKA_SET  = BALL_SENSOR;

//////////////////////////////
// Public global type definitions
//////////////////////////////

//////////////////////////////
// Public global variables
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

#endif /* IO_H_ */
