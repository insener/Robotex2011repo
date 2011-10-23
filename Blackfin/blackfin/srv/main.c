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
#include "rtc.h"
#include "motion.h"
#include "io.h"
#include "timer.h"
#include "debug.h"
#include "soccer.h"

extern void httpd_request(char firstChar);

int main() {
    unsigned char ch;
    frameSize frame;
    int rtcMs = 0;
    unsigned char** videoFrames;
    unsigned int ballSensor = 0;
    int time, ballTime = 0;

    // Sets up the camera to 320x240
    frame.imageWidth = DEFAULT_IMAGE_WIDTH;
    frame.imageHeight = DEFAULT_IMAGE_HEIGHT;

    init_heap();
    io_initPeriphery(); // Initialise LED, GPIO, serial flow & lasers,
    srv_initPWM();		// motor PWM timers
    rtc_init();
    timer_init();
    uart_initUart0(UART0_BAUDRATE);
    uart_initUart1(UART1_BAUDRATE);
    clear_sdram(); // Clears from 0x00100000 to 0x02000000
    camera_setup(frame);
    init_colors(); 			// dependant on camera setup
    srv_initVariables();	// dependant on camera setup

   // serial_out_version();

    while (1)
    {
    	//soccer_run();
    	if ((rtc_read() - rtcMs) > 1000)
    	{
    		rtcMs = rtc_read();
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
    		ballTime = rtc_read();
    		ballSensor = 2;
    	}
    	time = rtc_read();
    	if ( (ballSensor == 2) && ((time - ballTime) > 2000) )
    	{
    		srv_relayOff();
    		io_enableBallSensorInterrupt();
    		ballSensor = 0;
    	}
    	if (getchar(&ch))
        {
    		//printf("%c", ch);
        	switch (ch)
            {
        		case 'n':
        			*pPORTHIO_SET = TEST_OUTPUT;
        			break;
        		case 'm':
					*pPORTHIO_CLEAR = TEST_OUTPUT;
					break;
        		case 'V':   // send version string
                    serial_out_version();
                    break;
                case 'I':
                	// unprocessed frame
                    //srv_grabFrame();
                    //srv_processFrame();
                    //send_frame();
                	srv_grabProcessAndSendFrame();
                    break;
                case '%':
                	camera_grabFrame();
                	srv_sendRawYUV((unsigned char *)FRAME_BUF);
                	break;
                case 'r':
                	videoFrames = camera_grabVideo();
                	if (videoFrames != NULL)
                	{
                		srv_sendRawYUVVideo(videoFrames);
                	}
                	break;
                case 'w':
                	// forward
                	motion_moveStraight(100);
                	printf("#w");
                    break;
                case 'z':
                	// backward
					motion_moveStraight(-100);
					printf("#z");
					break;
                case 's':
                	// stop
                	motion_stop();
                	printf("#s");
                	break;
                case 'a':
                	// left
					motion_moveSide(-100);
					printf("#a");
					break;
                case 'd':
					// right
					motion_moveSide(100);
					printf("#d");
					break;
                case 'G':
                case 'P':
                    httpd_request(ch);
                    break;
                case 'y':
                    camera_invertVideo();
                    break;
                case 'Y':
                    camera_restoreVideo();
                    break;
                case 'o':   // turn on overlay_flag
                    srv_overlayOn();
                    break;
                case 'O':   // turn off overlay_flag
                    srv_overlayOff();
                    break;
                case 'l':
                	srv_relayOn();
                	break;
                case 'L':
                	srv_relayOff();
                	break;
                case 'b':   // 320 x 240
                    camera_reset(320);
                    break;
                case 'c':   // 640 x 480
                    camera_reset(640);
                    break;
                case 'A':   // 1280 x 1024
                    camera_reset(1280);
                    break;
                case 'D':   // check battery
                    check_battery();
                    break;
                case 'q':  // change image quality
                    srv_changeImageQuality();
                    break;
                case '$': // prototype zone
                    switch (getch())
                    {
                        case '!':  // reset processor
                            reset_cpu();
                            break;
                    }
                    break;
                case 't':
                    serial_out_time();
                    break;
                case 'i':   // i2c read / write
                    process_i2c();
                    break;
                default:
                    //printf("#?");  // unknown command
                	break;
            }
            rtc_delayUS(12000000/UART0_BAUDRATE);  // flush recv buffer
            while (getchar(&ch))
                rtc_delayUS(12000000/UART0_BAUDRATE);  // allow enough time for characters to flow in*/
        }
    }
}


