/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  camera.c - OV9655 camera setup for SRV-1 robot
 *    Copyright (C) 2005-2009  Surveyor Corporation
 *    Copyright (C) TUT Robotics Club
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
#include "ov9655.h"
#include "ov7725.h"
#include "camera.h"
#include "string.h"

//////////////////////////////
// Private global constant definitions
//////////////////////////////
#define POL_C            0x0000
#define POL_S            0x0000
#define PPI_SYNCS        0x0020  // 2 or 3 syncs
#define PPI_INPUT_MODE   0x000C  // non ITU-R 656 mode
#define PPI_DATA_LEN     0x0000
#define PPI_PACK         0x0080
#define PPI_ENABLE       0x0001
#define DMA_FLOW_MODE    0x1000
#define DMA_2D           0x0010
#define DMA_WDSIZE_16    0x0004
#define DMA_WDSIZE_8     0x0000
#define DMA_WNR          0x0002  
#define DMA_ENABLE       0x0001

//////////////////////////////
// Type definitions
//////////////////////////////
typedef struct {
    void *pNext;
    unsigned char *pBuf;
    short dConfig;
} dmaDescList;

//////////////////////////////
// Private global functions
//////////////////////////////
int  initializeHW(unsigned char *buf1, unsigned char *buf2, short pixels, short lines);
void moveImage(unsigned char *src1, unsigned char *src2, unsigned char *dst, unsigned int width, unsigned int height);
void moveInvertedImage(unsigned char *src1, unsigned char *src2, unsigned char *dst, unsigned int width, unsigned int height);

//////////////////////////////
// Private global variables
//////////////////////////////
dmaDescList 	_dlist1;
dmaDescList 	_dlist2;
int 		    _imgWidth;     // image width in pixels
int 		    _imgHeight;    // image height in pixels
char 		    _imgHead[IMAGE_HEADER_SIZE];    // image frame header for I command
int			    _invertFlag;
unsigned char*  _videoBuffer[VIDEO_BUF_COUNT];
unsigned char** _videoBufPointer;

/*
 * Initializes MCU's HW for camera output stream
 */
int initializeHW(unsigned char *buf1, unsigned char *buf2, short pixels, short lines)
{
    unsigned short itmp = *pPORTG_FER;
    *pPORTG_FER |= 0x00FF;  // PPI[7:0]
    *pPORTG_FER |= 0x00FF;  // PPI[7:0]
    itmp = *pPORTF_FER;
    *pPORTF_FER |= 0x8300;  // PF.15 = PPI_CLK    PF.9 = FS1     PF.8 = FS2
    *pPORTF_FER |= 0x8300;

    *pPPI_CONTROL =
        POL_S | POL_C | PPI_DATA_LEN | PPI_PACK | PPI_SYNCS | PPI_INPUT_MODE;
    *pPPI_COUNT = (pixels * 2) - 1;  // data format is YUYV, so there are 2 bytes per pixel
    *pPPI_DELAY = 0;
    *pPPI_FRAME = lines;
    SSYNC;

    _dlist1.dConfig = FLOW_LARGE | NDSIZE_5 | DMA_WDSIZE_16 | DMA_2D | DMA_WNR | DMA_ENABLE ;
    _dlist1.pBuf = buf1;
    _dlist1.pNext = &_dlist2;

    _dlist2.dConfig = FLOW_LARGE | NDSIZE_5 | DMA_WDSIZE_16 | DMA_2D | DMA_WNR | DMA_ENABLE ;
    _dlist2.pBuf = buf2;
    _dlist2.pNext = &_dlist1;

    //*pDMA0_CONFIG = DMA_FLOW_MODE | DMA_WDSIZE_16 | DMA_2D | DMA_WNR;
    //*pDMA0_START_ADDR = buf1;
    *pDMA0_X_COUNT = pixels;
    *pDMA0_X_MODIFY = 2;
    *pDMA0_Y_COUNT = lines;
    *pDMA0_Y_MODIFY = 2;  
    SSYNC;

    *pDMA0_CURR_DESC_PTR = &_dlist1;
    *pDMA0_NEXT_DESC_PTR = &_dlist2;
    SSYNC;
    *pDMA0_CONFIG = FLOW_LARGE | NDSIZE_5 | DMA_WDSIZE_16 | DMA_2D | DMA_WNR;

    return 0;
}

/*
 * Copies image data from DMA buffer to memory
 */
void moveImage(unsigned char *src1, unsigned char *src2, unsigned char *dst, unsigned int width, unsigned int height)
{
    unsigned char *src;
    unsigned short *isrc, *idst;
    int ix;

    if (*pDMA0_CURR_ADDR < (void *)src2)
    {
        src = src2;
    }
    else
    {
        src = src1;
    }
    isrc = (unsigned short *)src;
    idst = (unsigned short *)dst;
    for (ix = 0; ix < (width * height); ix++)
    {
        *idst++ = *isrc++;
    }
}

/*
 * Copies inverted image data from DMA buffer to memory. Enables to analyze upside down picture.
 */
void moveInvertedImage(unsigned char *src1, unsigned char *src2, unsigned char *dst, unsigned int width, unsigned int height)
{
    unsigned char *src;
    unsigned short *isrc, *idst;
    int ix;

    if (*pDMA0_CURR_ADDR < (void *)src2)
    {
        src = src2;
    }
    else
    {
        src = src1;
    }
    isrc = (unsigned short *)src;
    idst = (unsigned short *)(dst + _imgWidth * _imgHeight * 2 - 2);
    for (ix = 0; ix < (width * height); ix++)
    {
        *idst-- = *isrc++;
    }
}

/*
 * Camera initial setup
 */
void camera_setup (frameSize frame)
{
    int ix, i;

    // Initialize camera-related globals
    _imgWidth = frame.imageWidth;    //default 320
    _imgHeight = frame.imageHeight;  //default 240
    strcpy(_imgHead, "##IMJ5");
    _invertFlag = 0;
    for (i = 0; i < VIDEO_BUF_COUNT; i++)
	{
		_videoBuffer[i] = (unsigned char *)(VIDEO_BUF1 + VIDEO_BUF_ELEMENT_SIZE * i);
	}
    _videoBufPointer = _videoBuffer;

    // write to camera
    for (ix=0; ix<3; ix++) {
        systemTime_delayMS(100);
        i2c_write(0x21, ov7725_qvga, sizeof(ov7725_qvga)>>1, SCCB_ON);
    }
    for (ix=0; ix<3; ix++) {
        systemTime_delayMS(100);
        i2c_write(0x30, ov9655_qvga, sizeof(ov9655_qvga)>>1, SCCB_ON);
    }
    initializeHW((unsigned char *)DMA_BUF1, (unsigned char *)DMA_BUF2, _imgWidth, _imgHeight);
    camera_run();
    // invert video as camera is upside down
    camera_invertVideo();
}

/*
 * Starts the camera
 */
void camera_run(void)
{
    // enable transfers
    *pDMA0_CONFIG |= DMA_ENABLE;
    SSYNC;
    *pPPI_CONTROL |= PPI_ENABLE;
    SSYNC;
}

/*
 * Stops the camera
 */
void camera_stop(void)
{
    // disable transfers
    *pDMA0_CONFIG &= 0xFFFE;
    SSYNC;
    *pPPI_CONTROL &= 0xFFFE;
    SSYNC;
}

/*
 * Reset the camera after a frame size change.
 */
void camera_reset (unsigned int width) {
    if (width == 160) {
        _imgWidth = width;
        _imgHeight = 120;
        strcpy(_imgHead, "##IMJ3");
        camera_stop();
        i2c_write(0x21, ov7725_qqvga, sizeof(ov7725_qqvga)>>1, SCCB_ON);
        i2c_write(0x30, ov9655_qqvga, sizeof(ov9655_qqvga)>>1, SCCB_ON);
       // printf("#a");
    } else if (width == 320) {
        _imgWidth = width;
        _imgHeight = 240;
        strcpy(_imgHead, "##IMJ5");
        camera_stop();
        i2c_write(0x21, ov7725_qvga, sizeof(ov7725_qvga)>>1, SCCB_ON);
        i2c_write(0x30, ov9655_qvga, sizeof(ov9655_qvga)>>1, SCCB_ON);
       // printf("#b");
    } else if (width == 640) {
        _imgWidth = width;
        _imgHeight = 480;
        strcpy(_imgHead, "##IMJ7");
        camera_stop();
        i2c_write(0x21, ov7725_vga, sizeof(ov7725_vga)>>1, SCCB_ON);
        i2c_write(0x30, ov9655_vga, sizeof(ov9655_vga)>>1, SCCB_ON);
       // printf("#c");
    } else if (width == 1280) {
        _imgWidth = width;
        _imgHeight = 1024;
        strcpy(_imgHead, "##IMJ9");
        camera_stop();
        i2c_write(0x30, ov9655_sxga, sizeof(ov9655_sxga)>>1, SCCB_ON);
       // printf("#d");
    }
    initializeHW((unsigned char *)DMA_BUF1, (unsigned char *)DMA_BUF2, _imgWidth, _imgHeight);
    camera_run();
}

/*
 * Flips video for upside-down camera
 */
void camera_invertVideo(void) {
    _invertFlag = 1;
	i2c_write(0x21, ov7725_invert, sizeof(ov7725_invert)>>1, SCCB_ON);  // flip UV on OV7725
    i2c_write(0x30, ov9655_invert, sizeof(ov9655_invert)>>1, SCCB_ON);  // flip UV on OV9655
}

/*
 * Restores normal video orientation
 */
void camera_restoreVideo(void) {
    _invertFlag = 0;
	i2c_write(0x21, ov7725_restore, sizeof(ov7725_restore)>>1, SCCB_ON); // restore UV on OV7725
    i2c_write(0x30, ov9655_restore, sizeof(ov9655_restore)>>1, SCCB_ON); // restore UV on OV9655
}

/*
 * Returns current frame size
 */
void camera_getFrameSize(frameSize* frame)
{
	frame->imageHeight = _imgHeight;
	frame->imageWidth = _imgWidth;
}

/*
 * Copies image header depending on image size. Header size is 6 characters.
 */
void camera_getImageHeader(char* header)
{
	int i;
	for (i = 0; i < IMAGE_HEADER_SIZE - 1; i++)
	{
		*(header + i) = _imgHead[i];
	}
}

/*
 * Grabs frame from DMA buffer and copies it into frame buffer for analyzing.
 */
void camera_grabFrame(void)
{
    if (_invertFlag)
    {
        moveInvertedImage((unsigned char *)DMA_BUF1, (unsigned char *)DMA_BUF2,  // grab and flip new frame
        						(unsigned char *)FRAME_BUF, _imgWidth, _imgHeight);
    }
    else
    {
        moveImage((unsigned char *)DMA_BUF1, (unsigned char *)DMA_BUF2,  // grab new frame
        						(unsigned char *)FRAME_BUF, _imgWidth, _imgHeight);
	}
}

/*
 * Grabs defined number of frames with 100 ms delay between frames.
 * Allows video only if resolution is 160 x 120 or 320 x 240
 * Returns the pointer to the video buffer
 */
unsigned char** camera_grabVideo(void)
{
	int i;

	if (_imgWidth > 320 || _imgHeight > 240)
	{
		return NULL;
	}
	for (i = 0; i < VIDEO_BUF_COUNT; i++)
	{
		if (_invertFlag)
		{
			// grab and flip new frame
			moveInvertedImage((unsigned char *)DMA_BUF1, (unsigned char *)DMA_BUF2,
													_videoBuffer[i], _imgWidth, _imgHeight);
		}
		else
		{
			// grab new frame
			moveImage((unsigned char *)DMA_BUF1, (unsigned char *)DMA_BUF2,
													_videoBuffer[i], _imgWidth, _imgHeight);
		}
		systemTime_delayMS(40);
	}
	return _videoBufPointer;
}
