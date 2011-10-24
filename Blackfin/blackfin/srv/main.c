/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  main.c - main control loop for SRV-1 robot
 *    Copyright (C) 2005-2009  Surveyor Corporation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details (www.gnu.org/licenses)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "srv.h"
#include "print.h"
#include "string.h"
#include "config.h"
#include "camera.h"
#include "systemTime.h"
#include "motion.h"
#include "io.h"
#include "timer.h"
#include "debug.h"
#include "soccer.h"
#include "colors.h"
#include "remoteControl.h"

//////////////////////////////
// Private global constant definitions
//////////////////////////////


//////////////////////////////
// Type definitions
//////////////////////////////


//////////////////////////////
// Private global functions
//////////////////////////////
void initializeModules(void);

//////////////////////////////
// Private global variables
//////////////////////////////


void initializeModules(void)
{
	frameSize frame;

    // Sets up the camera to 320x240
    frame.imageWidth = DEFAULT_IMAGE_WIDTH;
    frame.imageHeight = DEFAULT_IMAGE_HEIGHT;

    init_heap();
    io_initPeriphery(); // Initialise LED, GPIO, serial flow & lasers,
    srv_initPWM();		// motor PWM timers
    systemTime_init();
    timer_init();
    uart_initUart0(UART0_BAUDRATE);
    uart_initUart1(UART1_BAUDRATE);
    clear_sdram(); // Clears from 0x00100000 to 0x02000000
    camera_setup(frame);
    colors_init(); 			// dependant on camera setup
    srv_initVariables();	// dependant on camera setup
}

/*
 * Main function. Start of the program.
 */
int main()
{
    int rtcMs = 0;
    unsigned int ballSensor = 0;
    int time, ballTime = 0;

    initializeModules();

    while (1)
    {
   // 	if (io_isPlaySwitchOn())
   // 	{
    		soccer_run();
   /* 	}
    	else
    	{
    		remoteControl_run();
    	}
    	if ((systemTime_readRTC() - rtcMs) > 1000)
    	{
    		rtcMs = systemTime_readRTC();
    		//serial_out_time();
    		io_LED2Toggle();

    		//printf("%d", ballSensor);
    	}
    	if (ballSensor == 0)
    	{
    		ballSensor = io_getBallSensorCount();
    	}
    	if (ballSensor == 1)
    	{
    		ballTime = systemTime_readRTC();
    		ballSensor = 2;
    	}
    	time = systemTime_readRTC();
    	if ( (ballSensor == 2) && ((time - ballTime) > 2000) )
    	{
    		srv_relayOff();
    		io_enableBallSensorInterrupt();
    		ballSensor = 0;
    	}
*/
    }
}


