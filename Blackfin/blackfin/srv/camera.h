#ifndef CAMERA_H_
#define CAMERA_H_

#include "config.h"

//////////////////////////////
// Public global defines
//////////////////////////////
#define IMAGE_HEADER_SIZE  7

//////////////////////////////
// Public global type definitions
//////////////////////////////
typedef struct {
    int imageWidth;
    int imageHeight;
} frameSize;

//////////////////////////////
// Public global variables
//////////////////////////////

//////////////////////////////
// Public global functions
//////////////////////////////
extern void 			camera_setup(frameSize frame);
extern void 			camera_run(void);
extern void 			camera_stop(void);
extern void 			camera_reset(unsigned int width);
extern void 			camera_getFrameSize(frameSize* frame);
extern void 			camera_invertVideo(void);
extern void 			camera_restoreVideo(void);
extern void 			camera_getImageHeader(char* header);
extern void 			camera_grabFrame(void);
extern unsigned char** 	camera_grabVideo(void);

#endif

