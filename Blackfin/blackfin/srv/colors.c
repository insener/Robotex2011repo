/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  colors.c - image processing routines for the SRV-1 Blackfin robot.
 *    Copyright (C) 2007-2009  Surveyor Corporation
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

#include "colors.h"
#include "print.h"
#include "camera.h"

//////////////////////////////
// Private global constant definitions
//////////////////////////////


//////////////////////////////
// Type definitions
//////////////////////////////


//////////////////////////////
// Private global functions
//////////////////////////////
unsigned int index(unsigned int xx, unsigned int yy);
unsigned int svs_segcode(unsigned char *outbuf, unsigned char *inbuf, int thresh);
void 		 svs_segview(unsigned char *inbuf, unsigned char *outbuf);
void 		 addvect(unsigned char *outbuf, unsigned int columns, unsigned int *vect);
void 		 addline(unsigned char *outbuf, int slope, int intercept);
void		 addbox(unsigned char *outbuf, unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2);
void		 setPixel(unsigned char *inbuf, int x, int y, int *fault);
int			 markGolfBall(unsigned char *inbuf, int x, int y, int radius);

//////////////////////////////
// Private global variables
//////////////////////////////
unsigned int _imgWidth, _imgHeigth;
unsigned int ymax[MAX_COLORS], ymin[MAX_COLORS], umax[MAX_COLORS], umin[MAX_COLORS], vmax[MAX_COLORS], vmin[MAX_COLORS];
unsigned int blobx1[MAX_BLOBS], blobx2[MAX_BLOBS], bloby1[MAX_BLOBS], bloby2[MAX_BLOBS], blobcnt[MAX_BLOBS], blobix[MAX_BLOBS];
unsigned int hist0[256], hist1[256], mean[3];

/*
 * Initializes colors module
 */
void colors_init() {
    unsigned int ii;
    frameSize frame;
    
    for(ii = 0; ii<MAX_COLORS; ii++) {
        ymax[ii] = 0;
        ymin[ii] = 0;
        umax[ii] = 0;
        umin[ii] = 0;
        vmax[ii] = 0;
        vmin[ii] = 0;
    }
    camera_getFrameSize(&frame);
    _imgHeigth = frame.imageHeight;
    _imgWidth = frame.imageWidth;
}

/*
 * Finds the pixel's index according to x and y coordinate in the frame buffer
 */
unsigned int index(unsigned int xx, unsigned int yy)
{
	return ((yy * _imgWidth + xx) * 2) & 0xFFFFFFFC;  // always a multiple of 4
}

/*
 * Gets pixel in UYVy format
 */
unsigned int colors_vpix(unsigned char *frame_buf, unsigned int xx, unsigned int yy) {
        unsigned int ix;
        ix = index(xx,yy); 
        return    ((unsigned int)frame_buf[ix] << 24) +    // returns UYVY packed into 32-bit word
                        ((unsigned int)frame_buf[ix+1] << 16) +
                        ((unsigned int)frame_buf[ix+2] << 8) +
                        (unsigned int)frame_buf[ix+3];
}

/*
 * Uses blob detection algorithm on the frame buffer. Returns number of blobs found that match the search color
 */
//
unsigned int colors_vblob(unsigned char *frame_buf, unsigned char *blob_buf, unsigned int ii) {
    unsigned int jj, ix, xx, yy, y, u, v, count, bottom, top, tmp;
    unsigned int maxx, maxy;
    unsigned char *bbp, ctmp;
    int itmp, jtmp;
    int y1, y2, u1, u2, v1, v2;
    
    y1 = ymin[ii];
    y2 = ymax[ii];
    u1 = umin[ii];
    u2 = umax[ii];
    v1 = vmin[ii];
    v2 = vmax[ii];
    
    for (ix=0; ix<MAX_BLOBS; ix++) {
        blobcnt[ix] = 0;
        blobx1[ix] = _imgWidth;
        blobx2[ix] = 0;
        bloby1[ix] = _imgHeigth;
        bloby2[ix] = 0;
        blobix[ix] = 0;
    }

    bbp = blob_buf;
    for (ix=0; ix<_imgWidth*_imgHeigth; ix++)
        *bbp++ = 0;

    /* tag all pixels in blob_buf[]    
         matching = 1  
         no color match = 0
       thus all matching pixels will belong to blob #1 */
    bbp = blob_buf;
    for (ix=0; ix<(_imgWidth*_imgHeigth*2); ix+=4) {
        y = (((unsigned int)frame_buf[ix+1] + (unsigned int)frame_buf[ix+3])) >> 1;
        u = (unsigned int)frame_buf[ix];
        v = (unsigned int)frame_buf[ix+2];

        if ((y >= y1) && (y <= y2) && (u >= u1) && (u <= u2) && (v >= v1) && (v <= v2))
            *bbp = 1;
        bbp += 2;
    }

    ix = _imgWidth * (_imgHeigth-1);
    for (xx=0; xx<_imgWidth; xx++) {
        blob_buf[xx] = 0;
        blob_buf[xx+ix] = 0;
    }
    ix = _imgWidth-1;
    for (yy=0; yy<_imgHeigth; yy++) {
        blob_buf[yy*_imgWidth] = 0;
        blob_buf[yy*_imgWidth + ix] = 0;
    }
    
    /* clear out orphan pixels */
    itmp = 0;
    jtmp = 0;
    for (ix=0; ix<(_imgWidth*_imgHeigth); ix+=2) {
        if (blob_buf[ix]) {
            itmp++;
            ctmp = 
                blob_buf[(ix-_imgWidth)-2] +
                blob_buf[ix-_imgWidth] +
                blob_buf[(ix-_imgWidth)+2] +
                blob_buf[ix-2] +
                blob_buf[ix+2] +
                blob_buf[(ix+_imgWidth)-2] +
                blob_buf[ix+_imgWidth] +
                blob_buf[(ix+_imgWidth)+2];
            if (ctmp < 4) {
                jtmp++;
                blob_buf[ix] = 0;
            }
        }
    }
    //printf("cleared %d out of %d matching pixels\r\n", jtmp, itmp);

    /* clear out orphan pixels */
    itmp = 0;
    jtmp = 0;
    for (ix=0; ix<(_imgWidth*_imgHeigth); ix+=2) {
        if (blob_buf[ix]) {
            itmp++;
            ctmp = 
                blob_buf[(ix-_imgWidth)-2] +
                blob_buf[ix-_imgWidth] +
                blob_buf[(ix-_imgWidth)+2] +
                blob_buf[ix-2] +
                blob_buf[ix+2] +
                blob_buf[(ix+_imgWidth)-2] +
                blob_buf[ix+_imgWidth] +
                blob_buf[(ix+_imgWidth)+2];
            if (ctmp < 4) {
                jtmp++;
                blob_buf[ix] = 0;
            }
        }
    }
    //printf("cleared %d out of %d matching pixels\r\n", jtmp, itmp);

    maxx = _imgWidth;
    maxy = _imgHeigth;

    for (jj=0; jj<MAX_BLOBS; jj++) {
        blobcnt[jj] = 0;
        blobx1[jj] = maxx;
        blobx2[jj] = 0;
        bloby1[jj] = maxy;
        bloby2[jj] = 0;
    }
        
    jj = 0;    // jj indicates the current blob being processed
    for (xx=0; xx<maxx; xx+=2) {
        count = 0;
        bottom = maxy;
        top = 0;
        for (yy=0; yy<maxy; yy++) {
            ix = xx + yy*_imgWidth;
            if (blob_buf[ix]) {
                count++;
                if (bottom > yy)
                    bottom = yy;
                if (top < yy)
                    top = yy;
            }
        }
        if (count) {
            if (bloby1[jj] > bottom)
                bloby1[jj] = bottom;
            if (bloby2[jj] < top)
                bloby2[jj] = top;
            if (blobx1[jj] > xx)
                blobx1[jj] = xx;
            if (blobx2[jj] < xx)
                blobx2[jj] = xx;
            blobcnt[jj] += count;
        } else {
            if (blobcnt[jj])    // move to next blob if a gap is found
                jj++;
            if (jj > (MAX_BLOBS-2))
                goto blobbreak;
        }
    }
blobbreak:     // now sort blobs by size, largest to smallest pixel count
    for (xx=0; xx<=jj; xx++) {
        if (blobcnt[xx] == 0)    // no more blobs, so exit
            return xx;
        for (yy=xx; yy<=jj; yy++) {
            if (blobcnt[yy] == 0)
                break;
            if (blobcnt[xx] < blobcnt[yy]) {
                tmp = blobcnt[xx];
                blobcnt[xx] = blobcnt[yy];
                blobcnt[yy] = tmp;
                tmp = blobx1[xx];
                blobx1[xx] = blobx1[yy];
                blobx1[yy] = tmp;
                tmp = blobx2[xx];
                blobx2[xx] = blobx2[yy];
                blobx2[yy] = tmp;
                tmp = bloby1[xx];
                bloby1[xx] = bloby1[yy];
                bloby1[yy] = tmp;
                tmp = bloby2[xx];
                bloby2[xx] = bloby2[yy];
                bloby2[yy] = tmp;
            }
        }
    }
    return xx;
}

// histogram function - 
//  hist0[] holds frequency of u|v combination  
//  hist1[] holds average luminance corresponding each u|v combination

void colors_vhist(unsigned char *frame_buf) {
    unsigned int ix, iy, xx, yy, y1, u1, v1;

    for (ix=0; ix<256; ix++) {          hist0[ix] = 0;  // accumulator 
        hist1[ix] = 0;  
    }
    for (xx=0; xx<_imgWidth; xx+=2) {
        for (yy=0; yy<_imgHeigth; yy++) {
            ix = index(xx,yy);  
            y1 = (((unsigned int)frame_buf[ix+1] + (unsigned int)frame_buf[ix+3])) >> 1;
            u1 = ((unsigned int)frame_buf[ix]);
            v1 = ((unsigned int)frame_buf[ix+2]);
            iy = (u1 & 0xF0) + (v1 >> 4);
            hist0[iy]++;
            hist1[iy] += y1;
        }
    }
    for (ix=0; ix<256; ix++)
        if (hist1[ix])
            hist1[ix] /= hist0[ix];  // normalize by number of hits
}

/*
 * Mean color function - computes mean value for Y, U and V
 * mean[0] = Y mean, mean[1] = U mean, mean[2] = V mean
 */
void colors_vmean(unsigned char *frame_buf) {
    unsigned int ix, xx, yy, y1, u1, v1;
    unsigned int my, mu, mv;
    my = mu = mv = 0;

    for (xx=0; xx<_imgWidth; xx+=2) {
        for (yy=0; yy<_imgHeigth; yy++) {
            ix = index(xx,yy);  // yx, uv, vx range from 0-63 (yuv value divided by 4)
            y1 = (((unsigned int)frame_buf[ix+1] + (unsigned int)frame_buf[ix+3])) >> 1;
            u1 = ((unsigned int)frame_buf[ix]);
            v1 = ((unsigned int)frame_buf[ix+2]);
            my += y1;
            mu += u1;
            mv += v1;
        }
    }
    mean[0] = ((my*2) / _imgWidth) / _imgHeigth;
    mean[1] = ((mu*2) / _imgWidth) / _imgHeigth;
    mean[2] = ((mv*2) / _imgWidth) / _imgHeigth;
}

/*
 * Finds segments in the frame
 */
void colors_segment(unsigned char *frame_buf) {
    unsigned int ix, xx, yy, y, u, v, clr;
    unsigned int ymid[MAX_COLORS], umid[MAX_COLORS], vmid[MAX_COLORS];
    
    for (ix=0; ix<MAX_COLORS; ix++) {
        ymid[ix] = (ymax[ix] + ymin[ix]) >> 1;
        umid[ix] = (umax[ix] + umin[ix]) >> 1;
        vmid[ix] = (vmax[ix] + vmin[ix]) >> 1;
    }
    for (xx=0; xx<_imgWidth; xx+=2) {
        for (yy=0; yy<_imgHeigth; yy++) {
            ix = index(xx,yy);
            y = (((unsigned int)frame_buf[ix+1] + (unsigned int)frame_buf[ix+3])) >> 1;
            //y = (unsigned int)frame_buf[ix+1];
            u = (unsigned int)frame_buf[ix];
            v = (unsigned int)frame_buf[ix+2];
            for (clr=0; clr<MAX_COLORS; clr++) {
                if (ymax[clr] == 0)    // skip this color if not defined
                    continue;
                if ((y >= ymin[clr])
                  && (y <= ymax[clr]) 
                  && (u >= umin[clr]) 
                  && (u <= umax[clr]) 
                  && (v >= vmin[clr]) 
                  && (v <= vmax[clr])) {
                    frame_buf[ix+1] = frame_buf[ix+3] = ymid[clr];
                    frame_buf[ix] = umid[clr];
                    frame_buf[ix+2] = vmid[clr];
                    break;
                }
            }
            if (clr == MAX_COLORS) {  // if no match, black out the pixel
                frame_buf[ix+1] = frame_buf[ix+3] = 0;
                frame_buf[ix] = frame_buf[ix+2] = 128;
            }
        }
    }
}

/*
 * Counts number of pixels matching 'clr' bin in range [x1,y1] to [x2,y2]
 */
unsigned int vfind(unsigned char *frame_buf, unsigned int clr, 
                   unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2) {
    unsigned int ix, xx, yy, y, u, v, count;
    
    count = 0;
    for (xx=x1; xx<x2; xx+=2) {   
        for (yy=y1; yy<y2; yy++) {
            ix = index(xx,yy);
            y = (((unsigned int)frame_buf[ix+1] + (unsigned int)frame_buf[ix+3])) >> 1;
            //y = (unsigned int)frame_buf[ix+1];
            u = (unsigned int)frame_buf[ix];
            v = (unsigned int)frame_buf[ix+2];
            if ((y >= ymin[clr])
              && (y <= ymax[clr]) 
              && (u >= umin[clr]) 
              && (u <= umax[clr]) 
              && (v >= vmin[clr]) 
              && (v <= vmax[clr])) {
                count++;
            }
        }
    }
    return count;
}

/*
 * Uses edge detection algorithm on the frame
 */
void colors_edgeDetect(unsigned char *inbuf, unsigned char *outbuf, int thresh) {
    unsigned int ix, xx, yy, y2, u2, v2, skip;
    unsigned char *ip, *op;
    unsigned int *ip1, *op1;
    unsigned int gx, gy;
    
    skip = _imgWidth*2;
    for (xx=2; xx<_imgWidth-2; xx+=2) {
        for (yy=1; yy<_imgHeigth-1; yy++) {
            gx = gy = 0;
            ix = index(xx, yy);
            ip = inbuf + ix;
            op = outbuf + ix;

            y2 = *(ip+5) + *(ip+7) - *(ip-3) - *(ip-1);
            u2 = *(ip+4) - *(ip-4);
            v2 = *(ip+6) - *(ip-2);
            gy = ((y2*y2)>>2) + u2*u2 + v2*v2;
            
            y2 = *(ip+1+skip) + *(ip+3+skip) - *(ip+1-skip) - *(ip+3-skip);
            u2 = *(ip+skip) - *(ip-skip); 
            v2 = *(ip+2+skip) - *(ip+2-skip); 
            gx = ((y2*y2)>>2) + u2*u2 + v2*v2;
 
            if ((gx > thresh) || (gy > thresh)) {
                *op = 128;
                *(op+2) = 128;
                *(op+1) = 255;
                *(op+3) = 255;
            } else {
                *op = (*(ip) & 0x80) | 0x40;
                *(op+2) = (*(ip+2) & 0x80) | 0x40;
                *(op+1) = (*(ip+1) & 0xC0) | 0x20;
                *(op+3) = (*(ip+3) & 0xC0) | 0x20;
            }
        }
    }

    op1 = (unsigned int *)outbuf;
    ip1 = (unsigned int *)inbuf;
    for (ix=0; ix<(_imgWidth*_imgHeigth>>1); ix++)
        ip1[ix] = op1[ix];
}

/*
 * Uses vscan on the frame
 */
unsigned int colors_vscan(unsigned char *outbuf, unsigned char *inbuf, int thresh,
           unsigned int columns, unsigned int *outvect) {
    int x, y;
    unsigned int ix, hits;
    unsigned char *pp;
    
    svs_segcode(outbuf, inbuf, thresh);  // find edge pixels

    hits = 0;
    pp = outbuf;
    for (ix=0; ix<columns; ix++)    // initialize output vector
        outvect[ix] = _imgHeigth;
        
    // now search from top to bottom, noting each hit in the appropriate column
    for (y=0; y<_imgHeigth; y++)
    {
    	// note that edge detect used full UYVY per pixel position
    	for (x=0; x<_imgWidth; x+=2)
        {
    		// look for edge hit
    		if (*pp & 0xC0)
            {
                outvect[((x * columns) / _imgWidth)] = _imgHeigth - y;
                hits++;
            }
            pp++;
        }
    }
    return hits;
}

/*
 * Search for image horizon.  similar to vscan(), but search is top-to-bottom rather than bottom-to-top
 */
unsigned int colors_vhorizon(unsigned char *outbuf, unsigned char *inbuf, int thresh,
           int columns, unsigned int *outvect, int *slope, int *intercept, int filter) {
    int x, y;
    int ix, hits;
    unsigned char *pp;
    static int sfilter[10], ifilter[10];
    
    if (filter > 10) filter = 10;  // max number of filter taps
    if (filter < 1) filter = 1;
    
    svs_segcode(outbuf, inbuf, thresh);  // find edge pixels

    hits = 0;
    for (ix=0; ix<columns; ix++)    // initialize output vector
        outvect[ix] = 0;
        
    for (y=_imgHeigth-1; y>=0; y--) {  // now search from top to bottom, noting each hit in the appropriate column
        pp = outbuf + (y * _imgWidth / 2);
        for (x=0; x<_imgWidth; x+=2) {  // note that edge detect used full UYVY per pixel position
            if (*pp & 0xC0) {   // look for edge hit 
                outvect[((x * columns) / _imgWidth)] = y;
                hits++;
            }
            pp++;
        }
    }
    /*int sx, sy, sxx, sxy, dx, delta;
    sx = sy = sxx = sxy = 0;
    dx = _imgWidth / columns;
    for (ix=0; ix<columns; ix++) {
        x = ix*dx + dx/2;
        y = (int)outvect[ix];
        sx += x;
        sy += y;
        sxx += x*x;
        sxy += x*y;
    }
    delta = columns*sxx - sx*sx;
    *slope = (1000 * (columns*sxy - sx*sx))/ delta;  // slope is scaled by 1000
    *intercept = (sxx*sy - sx*sxy) / delta; */
    int sx, sy, stt, sts, t, dx;
    sx = sy = stt = sts = 0;
    dx = _imgWidth / columns;
    for (ix = 0; ix < columns; ix++) {
        sx += ix*dx + dx/2;
        sy += (int)outvect[ix];
    }
    for (ix = 0; ix < columns; ix++) {
        t = ix*dx + dx/2 - sx/columns;
        stt += t*t;
        sts += t*(int)outvect[ix];
    }
	*slope = (1000*sts)/stt;
	*intercept = (sy*1000 - sx*(*slope))/(columns*1000);
    if (*intercept > (_imgHeigth-1)) *intercept = _imgHeigth-1;
    if (*intercept < 0) *intercept = 0;

    if (filter > 1) {
        for (ix=filter; ix>1; ix--) {  // push old values to make room for latest
            sfilter[ix-1] = sfilter[ix-2];
            ifilter[ix-1] = ifilter[ix-2];
        }
        sfilter[0] = *slope;
        ifilter[0] = *intercept;
        sx = sy = 0;
        for (ix=0; ix<filter; ix++) {  // now average the data
            sx += sfilter[ix];
            sy += ifilter[ix];
        }
        *slope = sx / filter;
        *intercept = sy / filter;
    }
    //printf("vhorizon:  slope = %d   intercept = %d\r\n", *slope, *intercept);
    return hits;
}

/*
 *
 */
unsigned int svs_segcode(unsigned char *outbuf, unsigned char *inbuf, int thresh) {
    unsigned int ix, xx, yy, y2, u2, v2, skip;
    unsigned char *ip, *op, cc;
    unsigned int gx, gy, edgepix;
    
    if (_imgWidth > 640)   // buffer size limits this function to 640x480 resolution
        return 0;
    
    skip = _imgWidth*2;
    op = outbuf;
    edgepix = 0;
    for (yy=0; yy<_imgHeigth; yy++) {
        for (xx=0; xx<_imgWidth; xx+=2) {
            if ((xx < 2) || (xx >= _imgWidth-2) || (yy < 1) || (yy >= _imgHeigth-1)) {
                *op++ = 0;
                continue;
            }
            gx = gy = 0;
            ix = index(xx, yy);
            ip = inbuf + ix;

            y2 = *(ip+5) + *(ip+7) - *(ip-3) - *(ip-1);
            u2 = *(ip+4) - *(ip-4);
            v2 = *(ip+6) - *(ip-2);
            gy = ((y2*y2)>>2) + u2*u2 + v2*v2;
            
            y2 = *(ip+1+skip) + *(ip+3+skip) - *(ip+1-skip) - *(ip+3-skip);
            u2 = *(ip+skip) - *(ip-skip); 
            v2 = *(ip+2+skip) - *(ip+2-skip); 
            gx = ((y2*y2)>>2) + u2*u2 + v2*v2;
 
            cc = ((*ip >> 2) & 0x38)       // threshold U to 0x38 position
               + ((*(ip+2) >> 5) & 0x07);  // threshold V to 0x07 position
            if (gx > thresh)  
                cc |= 0x80;               // add 0x80 flag if this is a horizontal edge pixel
            if (gy > thresh)  
                cc |= 0x40;               // add 0x40 flag if this is a vertical edge pixel
            if (cc & 0xC0)
                edgepix++;
            *op++ = cc;
        }
    }
    return edgepix;
}

/*
 *
 */
void svs_segview(unsigned char *inbuf, unsigned char *outbuf) {
    unsigned int ix;
    unsigned char *ip, *op;

    if (_imgWidth > 640)   // buffer size limits this function to 640x480 resolution
        return;
    
    ip = inbuf;
    op = outbuf;
    for (ix=0; ix<_imgWidth*_imgHeigth; ix+=2) {
        if (*ip & 0xC0) {      // is this an edge pixel ?
            *(op+1) = *(op+3) = 0xFF;
            *op = *(op+2) = 0x80;
        } else {
            *(op+1) = *(op+3) = 0xA0;
            *op = ((*ip & 0x38) << 2) + 0x10;
            *(op+2) = ((*ip & 0x07) << 5) + 0x10;
        }
        op += 4;
        ip++;
    }
}

/*
 * Display vector as red pixels (YUV = 72 84 255)
 */
void addvect(unsigned char *outbuf, unsigned int columns, unsigned int *vect)
{
    unsigned int xx, yy, ix;

    if (_imgWidth > 640)   // buffer size limits this function to 640x480 resolution
        return;
    for (xx=0; xx<_imgWidth; xx++) {
        yy = (_imgHeigth) - (vect[(xx * columns) / _imgWidth]);
        ix = index(xx, yy);
        outbuf[ix+1] =  outbuf[ix+3] = 72;
        outbuf[ix] = 84;
        outbuf[ix+2] = 255;
    }
}

/*
 * Display line as red pixels (red YUV = 72 84 255) (yellow YUV = 194 18 145)
 * note that slope is scaled up by 1000
 */
void addline(unsigned char *outbuf, int slope, int intercept)
{
    int xx, yy, ix;

    if (_imgWidth > 640)   // buffer size limits this function to 640x480 resolution
        return;
    for (xx=0; xx<_imgWidth; xx+=2) {
        yy = ((slope * xx) / 1000) + intercept; 
        if (yy > (_imgHeigth-1)) yy = _imgHeigth - 1;
        if (yy < 0) yy = 0; 
        ix = index(xx, yy);
        outbuf[ix+1] =  outbuf[ix+3] = 72;
        outbuf[ix] = 84;
        outbuf[ix+2] = 255;
    }
}

/*
 * Display a box as red pixels (YUV = 72 84 255) or yellow pixels (YUV = 194 18 145)
 */
void addbox(unsigned char *outbuf, unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2)
{
    unsigned int xx, yy, ix;

    for (xx=x1; xx<=x2; xx+=2) {
        ix = index(xx, y1);
        outbuf[ix+1] =  outbuf[ix+3] = 194;
        outbuf[ix] = 18;
        outbuf[ix+2] = 145;
        ix = index(xx, y2);
        outbuf[ix+1] =  outbuf[ix+3] = 194;
        outbuf[ix] = 18;
        outbuf[ix+2] = 145;
    }
    for (yy=y1; yy<=y2; yy++) {
        ix = index(x1, yy);
        outbuf[ix+1] =  outbuf[ix+3] = 194;
        outbuf[ix] = 18;
        outbuf[ix+2] = 145;
        ix = index(x2, yy);
        outbuf[ix+1] =  outbuf[ix+3] = 194;
        outbuf[ix] = 18;
        outbuf[ix+2] = 145;
    }
}

/*
 * Searches orange golf balls, if found then marks them. Up to 10 balls detected, so
 * the golf ball buffer (_golfBalls) must be capable of holding up to 10 balls.
 * Returns the number of balls found.
 */
unsigned int colors_searchGolfBalls(unsigned char *inbuf, GolfBall* golfBalls)
{
	const unsigned char CORRECTIVE_HEIGHT_CONSTANT = 17;
	/*image processing constants*/
	const unsigned int ORANGE_V_LOW_LIMIT = 170;
	const unsigned int ORANGE_V_HIGH_LIMIT = 250;

	unsigned int start;
	unsigned int end;
	unsigned int y;
	int rowIndex = 0;
	//int ballDiam[MAX_GOLF_BALLS] = {0,0,0,0,0,0,0,0,0,0};
	//int ballX[MAX_GOLF_BALLS]= {0,0,0,0,0,0,0,0,0,0};
	//int ballY[MAX_GOLF_BALLS]= {0,0,0,0,0,0,0,0,0,0};
	int ballIndex;
	int newBall;
	int state;
	int rowStep = _imgWidth * 2;
	unsigned int i;
	unsigned int debugInfo = 0;

	// do line scan only for the middle row
	y = _imgHeigth / 2 - CORRECTIVE_HEIGHT_CONSTANT + rowIndex; // 17 is a corrective height index constant
	start = index(0, y);
	end = start + _imgWidth * 2 - 4;
	// nill the buffer
	for (i = 0; i < MAX_GOLF_BALLS; i++)
	{
		golfBalls[i].ballIdent = 0;
		golfBalls[i].ballDiam = 0;
		golfBalls[i].ballX = 0;
		golfBalls[i].ballY = 0;
	}
	ballIndex = 0;
	//ballDiam[ballIndex] = 0;
	//ballX[ballIndex] = 0;
	//ballY[ballIndex] = 0;
	newBall = 0;
	state = noBall;

	debugInfo = start;

	// take the row
	for (i = start; i < end; i = i + 4)
	{
		switch (state)
		{
			case noBall:
				// filter orange using only V value
				if ( (*(inbuf + i + 2) > ORANGE_V_LOW_LIMIT) && (*(inbuf + i + 2) < ORANGE_V_HIGH_LIMIT) )
				{
					//*(outbuf + i) = *(inbuf + i);
					golfBalls[ballIndex].ballDiam++;
					state = extendBall;
				}
				break;
			case extendBall:
				// find how far the orange goes
				if ( (*(inbuf + i + 2) > ORANGE_V_LOW_LIMIT) && (*(inbuf + i + 2) < ORANGE_V_HIGH_LIMIT) )
				{
					//*(outbuf + i) = *(inbuf + i);
					golfBalls[ballIndex].ballDiam++;
				}
				else  // end of orange -> take a closer search
				{
					state = closerSearch;
				}
				break;
			case closerSearch:
				// observe upper and lower row
				if ( ((*(inbuf + i + 2 + rowStep) > ORANGE_V_LOW_LIMIT) && (*(inbuf + i + 2 + rowStep) < ORANGE_V_HIGH_LIMIT)) ||
						(( *(inbuf + i + 2 - rowStep) > ORANGE_V_LOW_LIMIT) && (*(inbuf + i + 2 - rowStep) < ORANGE_V_HIGH_LIMIT )) )
				{
					// continue with the same ball, add previous and this pixel
					golfBalls[ballIndex].ballDiam += 2;
					state = extendBall;
				}
				else
				{
					//end of ball -> calculate ball info
					//ballDiam[ballIndex] = ballDiam[ballIndex]; // divide by 2 as 1 pixel involves 2 bytes
					golfBalls[ballIndex].ballX = (((i - 4)/ 2 - start / 2) - golfBalls[ballIndex].ballDiam);
					golfBalls[ballIndex].ballY = _imgHeigth / 2 - CORRECTIVE_HEIGHT_CONSTANT + rowIndex;
					golfBalls[ballIndex].ballIdent = ballIndex;
					ballIndex++;
					if (ballIndex >= MAX_GOLF_BALLS)
					{
						//end of search
						state = endOfSearch;
					}
					else
					{
						// start looking for next ball
						state = noBall;
					}
				}
				break;
			case endOfSearch:
				i = end;
				break;
			default:
				break;
		}
	}
	// check if the ball was cut by scene
	if (state == extendBall)
	{
		golfBalls[ballIndex].ballX = (_imgWidth - golfBalls[ballIndex].ballDiam);
		golfBalls[ballIndex].ballY = _imgHeigth / 2 - CORRECTIVE_HEIGHT_CONSTANT + rowIndex;
		golfBalls[ballIndex].ballIdent = ballIndex;
		ballIndex++;
	}

	for (i = 0; i < ballIndex; i++)
	{
		markGolfBall(inbuf, golfBalls[i].ballX, golfBalls[i].ballY, golfBalls[i].ballDiam);
	}

	return ballIndex;
}

/*
 * Draws a circle to mark a ball
 */
int markGolfBall(unsigned char *inbuf, int x, int y, int radius)
{
	int cx = 0;
	int cy = radius;
	int df = 1 - radius;
	int d_e = 3;
	int d_se = -2 * radius + 5;
	int faultValue = 0;
	int *fault = &faultValue;

	do
	{
		setPixel(inbuf, x+cx, y+cy, fault);

		if (cx)
		{
			setPixel(inbuf, x-cx, y+cy, fault);
		}

		if (cy)
		{
			setPixel(inbuf, x+cx, y-cy, fault);
		}

		if ((cx) && (cy))
		{
			setPixel(inbuf, x-cx, y-cy, fault);
		}

		if (cx != cy)
		{
			setPixel(inbuf, x+cy, y+cx, fault);

			if (cx)
			setPixel(inbuf, x+cy, y-cx, fault);

			if (cy)
				setPixel(inbuf, x-cy, y+cx, fault);

			if (cx && cy)
				setPixel(inbuf, x-cy, y-cx, fault);

			if (df < 0)
			{
				df += d_e;
				d_e += 2;
				d_se += 2;
			}
			else {
				df += d_se;
				d_e += 2;
				d_se += 4;
				cy--;
			}
		}
		cx++;
		//check if any faulty index
		if (*fault == 34)
		{
			return 0;
		}
	} while (cx <= cy);
	// successfully drawn circle
	return 1;
}

/*
 * Sets certain pixel to blue
 */
void setPixel(unsigned char *inbuf, int x, int y, int *fault)
{
	int i;

	i = index(x, y);
	// check that index is inside the buffer
	if (i > _imgWidth * _imgHeigth * 2)
	{
		// set fault value
		*fault = 34;
	}
	// sets blue value (u y v y)
	*(inbuf + i) = 195;
	*(inbuf + i + 1) = 81;
	*(inbuf + i + 2) = 101;
	*(inbuf + i + 3) = 81;
}

