/*
 * remoteControl.c
 *
 *  Created on: 24.10.2011
 *      Author: pes
 */

#include "remoteControl.h"
#include "srv.h"
#include "stdlib.h"
#include "camera.h"
#include "print.h"
#include "motion.h"
#include "systemTime.h"
#include "io.h"

//////////////////////////////
// Private global constant definitions
//////////////////////////////


//////////////////////////////
// Type definitions
//////////////////////////////


//////////////////////////////
// Private global functions
//////////////////////////////
extern void httpd_request(char firstChar);

//////////////////////////////
// Private global variables
//////////////////////////////


/*
 * Listens remote user commands and acts according to commands. The communication is done over WiFi.
 */
void remoteControl_run(void)
{
    unsigned char ch;
	unsigned char** videoFrames;
	int rtcMs = 0;
    unsigned int ballSensor = 0;
    int time, ballTime = 0;

	// run until play switch is off
	while(1/*!srv_isPlaySwitchOn()*/)
	{
		if ((systemTime_readRTC() - rtcMs) > 1000)
		{
			rtcMs = systemTime_readRTC();

			io_LED1Toggle();
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
		if (getchar(&ch))
		{
			//printf("%c", ch);
			switch (ch)
			{
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
			systemTime_delayUS(12000000/UART0_BAUDRATE);  // flush recv buffer
			while (getchar(&ch))
				systemTime_delayUS(12000000/UART0_BAUDRATE);  // allow enough time for characters to flow in*/
		}
	}
}
