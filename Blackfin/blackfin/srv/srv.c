/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  srv.c - routines to interface with the SRV-1 Blackfin robot.
 *    modified from main.c - main control loop for SRV-1 robot
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

#include "i2c.h"
#include "systemTime.h"
#include "camera.h"
#include "jpeg.h"
#include "stm_m25p32.h"
#include "font8x8.h"
#include "colors.h"
#include "malloc.h"
#include "print.h"
#include "string.h"
#include "sdcard.h"
#include "srv.h"
#include "timer.h"
#include "io.h"

//////////////////////////////
// Private global defines
//////////////////////////////
#define SWITCH_DEBOUNCE     4

//////////////////////////////
// Type definitions
//////////////////////////////


//////////////////////////////
// Private global functions
//////////////////////////////
void initIO(void);
void initRelay(void);
void initIrBeaconReceivers(void);
void initDistanceSensors(void);
void initSwitches(void);

//////////////////////////////
// Private global variables
//////////////////////////////

/* Size of frame */
unsigned int imgWidth, imgHeight;
/* motor move times */
int move_start_time, move_stop_time, move_time_mS;
int robot_moving;

/* Version */
unsigned char version_string[] = "SRV-1 Blackfin w/ built:" __TIME__ " - " __DATE__;

/* Frame count output string */
unsigned char frame[] = "0000-ms 000-f 000-d 000-l 000-r";
//unsigned char frame[] = "frame     ";

/* Camera globals */
unsigned int framecount, ix, overlay_flag;
unsigned char *output_start, *output_end; /* Framebuffer addresses */
unsigned int image_size; /* JPEG image size */
unsigned int _processTime;
unsigned int _debugInfo;
char _imgHeader[11];


/* General globals */
unsigned char *cp;
unsigned int i, j; // Loop counter.

/* Play switch debouncing*/
int _playSwitchOnCount;
int _playSwitchOffCount;
int _playSwitchState;

/*
 * Initializes robot functions
 */
void srv_initRobotFunctions(void)
{
	//initRelay();
	//initIrBeaconReceivers();
    initDistanceSensors();
    initSwitches();
}

/*
 * Initializes variables
 */
void srv_initVariables(void)
{
	frameSize cameraFrame;

	//variable initialization
	camera_getFrameSize(&cameraFrame);
	imgHeight = cameraFrame.imageHeight;
	imgWidth = cameraFrame.imageWidth;

	overlay_flag = 0;
	framecount = 0;

	_playSwitchOnCount = 0;
	_playSwitchOffCount = 0;
	_playSwitchState = switchNotKnown;

}

/*
 * Initializes distance sensors
 */
void initDistanceSensors(void)
{
    // set HW
    io_initGpioPin(ioInput, portH, IR_DIST_LEFT);
    io_initGpioPin(ioInput, portH, IR_DIST_RIGHT);
}

/*
 * Initializes switches
 */
void initSwitches(void)
{
    io_initGpioPin(ioInput, portH, GOAL_SELECTION);
    io_initGpioPin(ioInput, portH, PLAY_SWITCH);
}

/*
 * Reset CPU
 */
void reset_cpu() {
    asm(
    "p0.l = 0x0100; "
    "p0.h = 0xFFC0; "
    "r0.l = 0x0007; "
    "w[p0] = r0; "
    "ssync; "
    "p0.l = 0x0100; "
    "p0.h = 0xFFC0; "
    "r0.l = 0x0000; "
    "w[p0] = r0; "
    "ssync; "
    "raise 1; ");                        
}

/* clear SDRAM */
void clear_sdram() {
  for (cp=(unsigned char *)0x00100000; cp<(unsigned char *)0x02000000; cp++) {
    *cp = 0;
  }
}

void show_stack_ptr() {
    int x = 0;
    asm("%0 = SP;" : "=r"(x) : "0"(x));
    printf("stack_ptr = 0x%x\r\n", x);
    return;
}

unsigned int stack_remaining() {
    unsigned int x = 0;
    asm("%0 = SP" : "=r"(x) : "0"(x));
    return (x - (unsigned int)STACK_BOTTOM);
}

void show_heap_ptr() {
    printf("heap_ptr  = 0x%x\r\n", (int)heap_ptr);
}

/* SRV-1 Firmware Version Request
   Serial protocol char: V */
void serial_out_version () {
    printf("##Version - %s", version_string);
    printf("\r\n");
}

/* Get current time
   Serial protocol char: t */
void serial_out_time () {
    printf("##time - millisecs:  %d\r\n", systemTime_readRTC());
}

/* Dump flash buffer to serial
   Serial protocol char: z-d */
void serial_out_flashbuffer () {
    printf("##zdump: \r\n");
    cp = (unsigned char *)FLASH_BUFFER;
    for (i=0; i<0x10000; i++) {
        if (*cp == 0)
            return;
        if (*cp == 0x0A)
            putchar(0x0D);
        putchar(*cp++);
    }
}


/*
 * Turn relay on. Uses left motor (M1) outputs M1+ and M1-
 * Serial protocol char: l
 */
void srv_relayOn()
{
	//setPWM(100,100);
	io_setPortPin(portH, LASER2);
    //printf("#l");

}

/*
 *  Turns relay off. Uses left motor (M1) outputs M1+ and M1-
 *  Serial protocol char: L
 */
void srv_relayOff()
{
	//setPWM(0,0);
	io_clearPortPin(portH, LASER2);
	//printf("#L");
}

/*
 * Checks if Play switch is switched ON. It uses debouncing, so it does
 * not change the state immediately, but after it is clear that
 * switch has changed the state.
 * This function is meant to be called periodically!!!
 * Returns value in SwitchState enumeration manner
 */
int srv_getPlaySwitchState(void)
{
	switch (_playSwitchState)
	{
	    case switchNotKnown:
	        // read IO pin and update counter
	        if (io_getPortPinValue(portH, PLAY_SWITCH))
            {
                _playSwitchOnCount++;
                _playSwitchOffCount = 0;
            }
            else
            {
                _playSwitchOffCount++;
                _playSwitchOnCount = 0;
            }
	        // check if switch has been in stable position long enough
	        if (_playSwitchOnCount > SWITCH_DEBOUNCE)
            {
                _playSwitchState = switchOn;
            }
	        if (_playSwitchOffCount > SWITCH_DEBOUNCE)
	        {
	            _playSwitchState = switchOff;
	        }
	        break;
	    case switchOff:
	        _playSwitchOffCount = 0;
	        // check if the switch have set to opposite position
	        if (io_getPortPinValue(portH, PLAY_SWITCH))
            {
                _playSwitchOnCount++;
            }
	        // check if switch has been in opposite position long enough
            if (_playSwitchOnCount > SWITCH_DEBOUNCE)
            {
                _playSwitchState = switchOn;
            }
            break;
	    case switchOn:
	        _playSwitchOnCount = 0;
	        // check if the switch have set to opposite position
	        if (!io_getPortPinValue(portH, PLAY_SWITCH))
            {
                _playSwitchOffCount++;
            }
	        // check if switch has been in opposite position long enough
	        if (_playSwitchOffCount > SWITCH_DEBOUNCE)
            {
                _playSwitchState = switchOff;
            }
            break;
	    default:
	        // should not be here
	        _playSwitchState = switchNotKnown;
	        break;
	}
    return _playSwitchState;
}

/*
 * Checks the status of goal selection pin
 */
int srv_isBlueGoalSelected(void)
{
    if (io_getPortPinValue(portH, GOAL_SELECTION))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void check_battery() { // 'D' command
    if (*pPORTHIO & 0x0004)
        printf("##D - low battery voltage detected\r\n");
    else
        printf("##D - battery voltage okay\r\n");
}

void srv_grabProcessAndSendFrame()
{
	int start, stop;
	GolfBall balls[MAX_GOLF_BALLS];
	Goal goal;

	camera_grabFrame();
	//srv_processFrame();
	_debugInfo = colors_searchGolfBalls((unsigned char *)FRAME_BUF, balls, 1);
	colors_searchGoal((unsigned char *)FRAME_BUF, &goal, colorBlue, 1);

	start = systemTime_readRTC();
	srv_sendFrame();
	stop = systemTime_readRTC();
	if (stop < start)
	{
		_processTime = 0;
	}
	else
	{
		_processTime = (unsigned int)(stop - start);
	}
}

/*
 * JPEG compress and send frame captured by grab_frame()
 * Serial protocol char: I
 */
void srv_sendFrame () {
    unsigned char i2c_data[2];
    unsigned char ch;
    unsigned int ix;
    
    if (overlay_flag)
    {
        //frame[9] = (framecount % 10) + 0x30;
        //frame[8] = ((framecount/10)% 10) + 0x30;
        //frame[7] = ((framecount/100)% 10) + 0x30;

        i2c_data[0] = 0x41;  // read compass twice to clear last reading
        i2c_read(0x22, (unsigned char *)i2c_data, 2, SCCB_ON);
        i2c_data[0] = 0x41;
        i2c_read(0x22, (unsigned char *)i2c_data, 2, SCCB_ON);
        ix = ((i2c_data[0] << 8) + i2c_data[1]) / 10;

        // send frame grabbing and processing time in ms
        frame[3] = (_processTime % 10) + 0x30;
        frame[2] = ((_processTime / 10) % 10) + 0x30;
        frame[1] = ((_processTime / 100) % 10) + 0x30;
        frame[0] = ((_processTime / 1000) % 10) + 0x30;

        // send debug info
        frame[12] = (_debugInfo % 10) + 0x30;
		frame[11] = ((_debugInfo / 10) % 10) + 0x30;
		frame[10] = ((_debugInfo / 100) % 10) + 0x30;
		frame[9] = ((_debugInfo / 1000) % 10) + 0x30;
		frame[8] = ((_debugInfo / 10000) % 10) + 0x30;

       /* sonar();
        ix = sonar_data[1] / 100;
        frame[10] = (ix % 10) + 0x30;
        frame[9] = ((ix/10)% 10) + 0x30;
        frame[8] = ((ix/100)% 10) + 0x30;
        ix = sonar_data[2] / 100;
        frame[16] = (ix % 10) + 0x30;
        frame[15] = ((ix/10)% 10) + 0x30;
        frame[14] = ((ix/100)% 10) + 0x30;
        ix = sonar_data[3] / 100;
        frame[22] = (ix % 10) + 0x30;
        frame[21] = ((ix/10)% 10) + 0x30;
        frame[20] = ((ix/100)% 10) + 0x30;
        ix = sonar_data[4] / 100;
        frame[28] = (ix % 10) + 0x30;
        frame[27] = ((ix/10)% 10) + 0x30;
        frame[26] = ((ix/100)% 10) + 0x30;
        */
        set_caption(frame, imgWidth);
    }
    output_start = (unsigned char *)JPEG_BUF;
    output_end = jpeg_encodeImage((unsigned char *)FRAME_BUF, output_start, FOUR_TWO_TWO, imgWidth, imgHeight);
    image_size = (unsigned int)(output_end - output_start);

  //  printf("%d, %d", imgWidth, imgHeight);
    framecount++;

    // copies 6 characters of image header first
    camera_getImageHeader(_imgHeader);

    // adds sizes afterwards
    _imgHeader[6] = (unsigned char)(image_size & 0x000000FF);
    _imgHeader[7] = (unsigned char)((image_size & 0x0000FF00) >> 8);
    _imgHeader[8] = (unsigned char)((image_size & 0x00FF0000) >> 16);
    _imgHeader[9] = 0x00;
    for (i=0; i<10; i++) {
        //while (*pPORTHIO & 0x0001)  // hardware flow control
            //continue;
        putchar(_imgHeader[i]);
    }
    cp = (unsigned char *)JPEG_BUF;
    for (i=0; i<image_size; i++) 
        putchar(*cp++);

    while (getchar(&ch)) {
        // flush input 
        continue;
    }
}

/*
 * Sends raw YUV picture data
 */
void srv_sendRawYUV(unsigned char *srcBuffer)
{
	// Current frame buffer
	unsigned char *frameBuffer = srcBuffer;

	// send header first
	//printf("##rawYUVstart\r\n");
	//systemTime_delayUS(20);
	// Format is U Y1 V Y2, where Y1 and Y2 is brightness for pixels #1 and #2.
	// U and V are common for both pixels.
	for (ix = 0; ix < (imgWidth * imgHeight) * 2; ix++)
		putchar(*frameBuffer++);
	// send tale afterwards
	//systemTime_delayUS(200);
	//printf("##rawYUVstop\r\n");
	return;
}

void srv_sendRawYUVVideo(unsigned char **srcBuffer)
{
	for (i = 0; i < VIDEO_BUF_COUNT; i++)
	{
		srv_sendRawYUV(srcBuffer[i]);
		systemTime_delayMs(500);
	}
}

/* Turn image overlay on.
   Serial protocol char: o */
void srv_overlayOn ()
{
    overlay_flag = 1;
    printf("#o");
}


/* Turn image overlay off.
   Serial protocol char: O */
void srv_overlayOff ()
{
    overlay_flag = 0;
    printf("#O");
}

/* Change image quality.
   Serial protocol char: q */
void srv_changeImageQuality ()
{
    unsigned char ch;
    unsigned int quality;

    ch = getch();
    quality = (unsigned int)(ch & 0x0f);
    jpeg_setQualityFactor(quality);
    printf("##quality - %c\r\n", ch);
}

// write caption string of up to 40 characters to frame buffer 
void set_caption(unsigned char *str, unsigned int width) {
    unsigned char *fbuf, *fcur, *str1, cc;
    int len, ix, iy, iz, w2;
    
    w2 = width * 2;
    str1 = str;
    
    for (len=0; len<40 && *str1++; len++);          // find string length
    fbuf = FRAME_BUF + (unsigned char *)((width * 17) - (len * 8));  // point to 1st char
    
    for (ix=0; ix<len; ix++) {
        fcur = fbuf;
        for (iy=0; iy< 8; iy++) {
            cc = font8x8[str[ix]*8 + iy];
            for (iz=0; iz<8; iz++) {
                if (cc & fontmask[iz]) {
                    fcur[0] = 0x80;
                    fcur[1] = 0xff;
                }
                fcur += 2;
            }
            fcur += (width * 2) - 16;          // move to next line
        }    
        fbuf += 16;  // move to next char position
    }
}

/* Process i2c command:  
        irxy  - i2c read device x, register y, return '##ir value'
        iRxy  - i2c read device x, register y, return 2-byte '##iR value'
        iMxyz - i2c read device x, register y, count z, return z-byte '##iM values'
        iwxyz - i2c write device x, register y, value z, return '##iw'
        iWabcd - i2c write device a, data b, c, d, return '##ix'
        idabcde - i2c dual write device a, register1 b, data1 c, register2 d, data2 e, return '##ix'
   Serial protocol char: i */
void process_i2c(void)
{
    unsigned char i2c_device, i2c_data[16], cx, count, c1, c2;
    
    switch ((unsigned char)getch()) {
        case 'r':
            i2c_device = (unsigned char)getch();
            i2c_data[0] = (unsigned char)getch();
            i2c_read(i2c_device, (unsigned char *)i2c_data, 1, SCCB_ON);
            printf("##ir%2x %d\r\n", i2c_device, i2c_data[0]);
            break;
        case 'R':
            i2c_device = (unsigned char)getch();
            i2c_data[0] = (unsigned char)getch();
            i2c_read(i2c_device, (unsigned char *)i2c_data, 2, SCCB_ON);
            printf("##iR%2x %d\r\n",i2c_device, (i2c_data[0] << 8) + i2c_data[1]);
            break;
        case 'M':
            i2c_device = (unsigned char)getch();
            i2c_data[0] = (unsigned char)getch();
            count = (unsigned char)getch() & 0x0F;
            i2c_read(i2c_device, (unsigned char *)i2c_data, (unsigned int)count, SCCB_ON);
            printf("##iM%2x  ", i2c_device);
            for (cx=0; cx<count; cx++) printf("%d ", i2c_data[cx]);
            printf("\r\n");
            break;
        case 'w':
            i2c_device = (unsigned char)getch();
            i2c_data[0] = (unsigned char)getch();
            i2c_data[1] = (unsigned char)getch();
            i2c_write(i2c_device, (unsigned char *)i2c_data, 1, SCCB_ON);
            printf("##iw%2x\r\n", i2c_device);
            break;
        case 'W':  // multi-write
            i2c_device = (unsigned char)getch();
            i2c_data[0] = (unsigned char)getch();
            i2c_data[1] = (unsigned char)getch();
            i2c_data[2] = (unsigned char)getch();
            i2c_writeX(i2c_device, (unsigned char *)i2c_data, 3, SCCB_ON);
            printf("##iW%2x", i2c_device);
            break;
        case 'd':  // dual channel single byte I2C write
            i2c_device = (unsigned char)getch();
            i2c_data[0] = (unsigned char)getch();
            i2c_data[1] = (unsigned char)getch();
            c1 = (unsigned char)getch();
            c2 = (unsigned char)getch();
            i2c_write(i2c_device, (unsigned char *)i2c_data, 1, SCCB_ON);
            systemTime_delayUS(1000);
            i2c_data[0] = c1;
            i2c_data[1] = c2;
            i2c_write(i2c_device, (unsigned char *)i2c_data, 1, SCCB_ON);
            printf("##id%2x", i2c_device);
            break;
        default:
            return;
    }
}

/*
 * Configure timers 2 for PWM (H-bridge interface) to drive relay, PIN initialization
 * is done in IO module io_initPeriphery
 */
void initRelay(void)
{
    unsigned short config;
    unsigned long period, width;

    config = PULSE_HI | PWM_OUT | PERIOD_CNT;
    period = PERIPHERAL_CLOCK / 1000;
    width = ((PERIPHERAL_CLOCK / 1000) * 1) / 100;

    timer_configureTimer(TIMER2, config, PWMOUT, period, width);
    //timer_enableTimer(TIMER2);

    //*pPORTHIO_DIR |= 0x0030;  // set PORTH4 and PORTH5 to output for direction control
    //*pPORTHIO &= 0xFFCF;      // set output low
    //*pPORTHIO |= 0x0030;  
}


void setPWM (int mleft, int mright)
{
    unsigned long width;

	if (mleft < 0) {
        *pPORTHIO = (*pPORTHIO & 0xFFEF);  // clear left direction bit
        mleft = -mleft;
    } else {
        *pPORTHIO = (*pPORTHIO & 0xFFEF) | 0x0010;  // turn on left direction bit
    }
    if (mleft > 100)
        mleft = 100;
    if (mleft < 1)
        mleft = 1;

    if (mright < 0) {
        *pPORTHIO = (*pPORTHIO & 0xFFDF);  // clear right direction bit
        mright = -mright;
    } else {
        *pPORTHIO = (*pPORTHIO & 0xFFDF) | 0x0020;  // turn on right direction bit
    }
    if (mright > 100)
        mright = 100;
    if (mright < 1)
        mright = 1;

    width = ((PERIPHERAL_CLOCK / 1000) * mleft) / 100;
    timer_setTimerWidth(TIMER2, width);
    //*pTIMER2_WIDTH = ((PERIPHERAL_CLOCK / 1000) * mleft) / 100;
    //*pTIMER3_WIDTH = ((PERIPHERAL_CLOCK / 1000) * mright) / 100;
}

/*
 * Initializes IR beacon receivers. Left side is connected to TMR7, right side to TMR3
 */
void initIrBeaconReceivers(void)
{
	// Set up peripheral timers 3 and 7 for IR beacon receivers, note that starts measuring from falling edge
	// PERIOD_CNT = 0, PULSE_HI = 0, disable and enable the timer in interrupt
	timer_configureTimer(TIMER3, WDTH_CAP | EMU_RUN, WDTHCAP, PERIPHERAL_CLOCK, 1);
	timer_configureTimerInterrupt(TIMER3);
	timer_configureTimer(TIMER7, WDTH_CAP | EMU_RUN, WDTHCAP, PERIPHERAL_CLOCK, 1);
	timer_configureTimerInterrupt(TIMER7);
}

/*
 * Gets the result of IR distance sensors.
 * 1 - obstacle not detected
 * 0 - obstacle detected
 */
void srv_getDistanceSensorResults(int *left, int *right)
{
    *left = io_getPortPinValue(portH, IR_DIST_LEFT);
    *right = io_getPortPinValue(portH, IR_DIST_RIGHT);
}


