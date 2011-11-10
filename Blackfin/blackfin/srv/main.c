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
#include "logic.h"

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
    //srv_initRobotFunctions();		// motor PWM and IR receiver timers
    systemTime_init();
    timer_init();
    uart_uart0Init(UART0_BAUDRATE);
    uart_uart1Init(UART1_BAUDRATE);
    //uart_uart2Init();
    clear_sdram(); // Clears from 0x00100000 to 0x02000000
    camera_setup(frame);
    colors_init(); 			// dependant on camera setup
    srv_initVariables();	// dependant on camera setup
    logic_init();
}

/*
 * Main function. Start of the program.
 */
int main()
{
    initializeModules();

    while (1)
    {
    	/*if (srv_isPlaySwitchOn())
    	{
    		soccer_run();
    	}
    	else
    	{*/
    		remoteControl_run();
    	//}
    }
}


