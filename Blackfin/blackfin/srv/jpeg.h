#ifndef JPEG_H_
#define JPEG_H_

//////////////////////////////
// Public global type definitions
//////////////////////////////

//////////////////////////////
// Public global defines
//////////////////////////////

//////////////////////////////
// Public global variables
//////////////////////////////

//////////////////////////////
// Public global functions
//////////////////////////////
unsigned char *jpeg_encodeImage (unsigned char *, unsigned char *, unsigned int, unsigned int, unsigned int);
void		   jpeg_setQualityFactor(unsigned int);
//output_end = encode_image((unsigned char *)0x01800000, output_start, quality, FOUR_TWO_TWO, 320, 256); 
extern unsigned char Lqt[], Cqt[];

#endif

