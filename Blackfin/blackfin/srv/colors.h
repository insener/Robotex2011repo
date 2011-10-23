#ifndef COLORS_H_
#define COLORS_H_

//////////////////////////////
// Public global defines
//////////////////////////////
#define MAX_BLOBS  63
#define MAX_COLORS 17  // reserve color #16 for internal use
#define MIN_BLOB_SIZE 10
#define MAX_GOLF_BALLS 10

//////////////////////////////
// Public global type definitions
//////////////////////////////
enum AlgorithmState
{
	noBall = 0,
	extendBall,
	closerSearch,
	endOfBall,
	endOfSearch
};

typedef struct
{
	int ballIdent;
	int ballDiam;
	int ballX;
	int ballY;
} GolfBall;

//////////////////////////////
// Public global variables
//////////////////////////////
extern unsigned int ymax[], ymin[], umax[], umin[], vmax[], vmin[];
extern unsigned int blobx1[], blobx2[], bloby1[], bloby2[], blobcnt[], blobix[];
extern unsigned int hist0[], hist1[], hist2[], mean[];

//////////////////////////////
// Public global functions
//////////////////////////////
extern unsigned int colors_vblob(unsigned char *, unsigned char *, unsigned int);
extern unsigned int colors_vpix(unsigned char *, unsigned int, unsigned int);
extern unsigned int colors_vfind(unsigned char *frame_buf, unsigned int clr, unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2);
extern void 		colors_init();
extern void 		colors_vhist(unsigned char *frame_buf);
extern void 		colors_vmean(unsigned char *frame_buf);
extern void 		colors_segment(unsigned char *frame_buf);
extern void 		colors_edgeDetect(unsigned char *outbuf, unsigned char *inbuf, int threshold);
extern unsigned int colors_vscan(unsigned char *outbuf, unsigned char *inbuf, int thresh, unsigned int columns, unsigned int *outvect);
extern unsigned int colors_vhorizon(unsigned char *outbuf, unsigned char *inbuf, int thresh, int columns, unsigned int *outvect, int *slope, int *intercept, int filter);
extern unsigned int colors_searchGolfBalls(unsigned char *inbuf, GolfBall *golfBalls);
#endif
