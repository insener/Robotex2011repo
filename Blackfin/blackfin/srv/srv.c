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

#include "uart.h"
#include "i2c.h"
#include "rtc.h"
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

//////////////////////////////
// Private global functions
//////////////////////////////
void initIO();

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
unsigned int segmentation_flag, edge_detect_flag, frame_diff_flag, horizon_detect_flag;
unsigned int obstacle_detect_flag;
unsigned int blob_display_flag;
unsigned int blob_display_num;
unsigned int edge_thresh;
unsigned char *output_start, *output_end; /* Framebuffer addresses */
unsigned int image_size; /* JPEG image size */
unsigned int _processTime;
unsigned int _debugInfo;
char _imgHeader[11];

/* Motor globals */
int lspeed, rspeed, lcount, rcount, lspeed2, rspeed2, base_speed, base_speed2, err1;
int pwm1_mode, pwm2_mode, pwm1_init, pwm2_init, xwd_init;
int encoder_flag;

/* IMU globals */
int x_acc, x_acc0, x_center;
int y_acc, y_acc0, y_center;

/* Failsafe globals */
//int failsafe_mode = 0;
//int lfailsafe, rfailsafe;
//int failsafe_clock;

/* Sonar globals */
int sonar_data[5], sonar_flag = 0;

/* random number generator globals */
unsigned int rand_seed = 0x55555555;

/* General globals */
unsigned char *cp;
unsigned int i, j; // Loop counter.
unsigned int master;  // SVS master or slave ?
unsigned int uart1_flag = 0;


void srv_initPWM()
{
	initPWM();
}

void srv_initVariables()
{
	frameSize cameraFrame;

	//variable initialization
	camera_getFrameSize(&cameraFrame);
	imgHeight = cameraFrame.imageHeight;
	imgWidth = cameraFrame.imageWidth;

	overlay_flag = 0;
	framecount = 0;
}

/* reset CPU */
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
    printf("##time - millisecs:  %d\r\n", rtc_read());
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

/* Turn relay on. Uses left motor (M1) outputs M1+ and M1-
   Serial protocol char: l */
void srv_relayOn()
{
	setPWM(100,100);
    //printf("#l");
}

/* Turn relay off. Uses left motor (M1) outputs M1+ and M1-
   Serial protocol char: L */
void srv_relayOff()
{
	setPWM(0,0);
	//printf("#L");
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

	camera_grabFrame();
	//srv_processFrame();
	_debugInfo = algorithm_searchGolfBalls((unsigned char *)FRAME_BUF);

	start = rtc_read();
	srv_sendFrame();
	stop = rtc_read();
	if (stop < start)
	{
		_processTime = 0;
	}
	else
	{
		_processTime = (unsigned int)(stop - start);
	}
}

/* JPEG compress and send frame captured by grab_frame()
   Serial protocol char: I */
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

// send raw YUV picture data
void srv_sendRawYUV(unsigned char *srcBuffer)
{
	// Current frame buffer
	unsigned char *frameBuffer = srcBuffer;

	// send header first
	printf("##rawYUVstart\r\n");
	rtc_delayUS(20);
	// Format is U Y1 V Y2, where Y1 and Y2 is brightness for pixels #1 and #2.
	// U and V are common for both pixels.
	for (ix = 0; ix < (imgWidth * imgHeight) * 2; ix++)
		putchar(*frameBuffer++);
	// send tale afterwards
	rtc_delayUS(200);
	printf("##rawYUVstop\r\n");
	return;
}

void srv_sendRawYUVVideo(unsigned char **srcBuffer)
{
	for (i = 0; i < VIDEO_BUF_COUNT; i++)
	{
		srv_sendRawYUV(srcBuffer[i]);
		rtc_delayMS(500);
	}
}

/* Turn image overlay on.
   Serial protocol char: o */
void srv_overlayOn () {
    overlay_flag = 1;
    printf("#o");
}


/* Turn image overlay off.
   Serial protocol char: O */
void srv_overlayOff () {
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
void process_i2c() {
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
            rtc_delayUS(1000);
            i2c_data[0] = c1;
            i2c_data[1] = c2;
            i2c_write(i2c_device, (unsigned char *)i2c_data, 1, SCCB_ON);
            printf("##id%2x", i2c_device);
            break;
        default:
            return;
    }
}

void initPWM() {
    unsigned short config;
    unsigned long period, width;

    config = PULSE_HI | PWM_OUT | PERIOD_CNT;
    period = PERIPHERAL_CLOCK / 1000;
    width = ((PERIPHERAL_CLOCK / 1000) * 1) / 100;
	// configure timers 2 and 3 for PWM (H-bridge interface)
    //*pPORT_MUX = 0;  // don't do this - it clobbers timers 6/7
    *pPORTF_FER |= 0x00C0;  // configure PF6 and PF7 as TMR3 and TMR2
   /* *pTIMER2_CONFIG = PULSE_HI | PWM_OUT | PERIOD_CNT;
    *pTIMER2_PERIOD = PERIPHERAL_CLOCK / 1000;                // 1000Hz
    *pTIMER2_WIDTH = ((PERIPHERAL_CLOCK / 1000) * 1) / 100; 
    *pTIMER_ENABLE = TIMEN2; //| TIMEN3;*/
    timer_configureTimer(TIMER2, config, PWMOUT, period, width);
    timer_enableTimer(TIMER2);

    *pPORTHIO_DIR |= 0x0030;  // set PORTH4 and PORTH5 to output for direction control
    *pPORTHIO &= 0xFFCF;      // set output low 
    //*pPORTHIO |= 0x0030;  
}

void setPWM (int mleft, int mright) {
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




