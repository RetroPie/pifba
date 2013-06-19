/*
 * EZX Emulator for MOTO EZX Modile Phone
 * Copyright (C) 2006 OopsWare. CHINA.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: gamewidget.cpp,v 0.10 2006/06/07 $
 */

#include <stdio.h>
#include <time.h>

#include "moose.h"
#include "gamewidget.h"
#include "configfile.h"
#include "font.h"

bool GameLooping	= false;
bool GamePauseing	= false;
int  GameScreenMode	= 0;
int  GameFrameSkip	= -1;
bool GameMute		= false;
int  GameGain		= 0;
int  GameKeymap[10]	= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//unsigned short VideoBufferH [320][240];

struct HARD_KEY_CODE HKCS[EZX_KEY_COUNT];

void initHKCS()
{
	for(int i=0; i<EZX_KEY_COUNT; i++)
		HKCS[i].down = false;
}

#if 0

extern "C" void fastRotateBlit(ushort *fb, ushort *bits, int x, int y, int w, int h);

#endif

#if 1

// rotate and blit screen from left top to right button (64bit-H) 
// full speed 108 FPS

static __inline void fastRotateBlit( ushort *fb, ushort *bits, int x, int y, int w, int h )
{
	// FIXME: this only works correctly for 240x320 displays
	ushort * pfb = fb;
	long long * ppfb;
	long long aa; 
	
	ushort * pbits = bits; 
	register ushort *ppbits1, *ppbits2, *ppbits3, *ppbits4;
	
	pfb += ( (320 - 1 - x) * 240 + y );
	pbits += ( y * 320 + x );
	
	register int i, j = h >> 2;
	while ( j-- ) {
		
		if ( GamePauseing )
			break;
		
		ppfb = (long long *)pfb;
		ppbits1 = pbits;
		ppbits2 = pbits + 320;
		ppbits3 = pbits + 640;
		ppbits4 = pbits + 960;
		
		pfb += 4;
		pbits += 1280;
		
		i = w;
		while ( i-- ) {
			aa  = *ppbits4;
			aa <<= 16;
			aa |= *ppbits3;
			aa <<= 16;
			aa |= *ppbits2;
			aa <<= 16;
			*ppfb = aa | *ppbits1;
			
			ppbits1 ++;
			ppbits2 ++;
			ppbits3 ++;
			ppbits4 ++;
			ppfb -= 60;
		}
	} 
}

#endif


#define POS_MENU_X	66
#define POS_MENU_Y	52
#define POS_MENU_H	15

#define MOUSE_KEY_POPUP_MENU		1
#define MOUSE_KEY_MENU_OK			2
#define MOUSE_KEY_MENU_RETURN		3

#define MOUSE_KEY_MENU_ITEM1		11
#define MOUSE_KEY_MENU_ITEM2		12
#define MOUSE_KEY_MENU_ITEM3		13
#define MOUSE_KEY_MENU_ITEM4		14
#define MOUSE_KEY_MENU_ITEM5		15
#define MOUSE_KEY_MENU_ITEM6		16
#define MOUSE_KEY_MENU_ITEM7		17
#define MOUSE_KEY_MENU_ITEM8		18
#define MOUSE_KEY_MENU_ITEM9		19
#define MOUSE_KEY_MENU_ITEM0		20

static inline void fastRotateBlit_menu( ushort *fb, ushort *bits, int x, int y, int w, int h )
{
	// FIXME: this only works correctly for 240x320 displays
	ushort * pfb = fb;
	register ushort * ppfb;
	ushort * pbits = bits; 
	register ushort * ppbits;
	
	pfb += ( (320 - x - w) * 240 + y );
	pbits += ( y * 320 + x + w - 1 );
	
	int i, j = w;
	while ( j-- ) {
		ppfb = pfb;
		ppbits = pbits;
		pfb += 240;
		pbits --;
		i = h;
		while ( i-- ) {
			*ppfb = *ppbits;
			ppbits += 320;
			ppfb ++;
		}
	} 
}

#include <time.h>
#include <sys/time.h>
#include <errno.h>

static struct timeval start;

void EZX_StartTicks(void)
{
	gettimeofday(&start, NULL);
}

unsigned int EZX_GetTicks (void)
{
	unsigned int ticks;
	struct timeval now;
	gettimeofday(&now, NULL);
	ticks=(now.tv_sec-start.tv_sec)*1000000+now.tv_usec-start.tv_usec;
	return ticks;
}

void EZX_Delay (unsigned int ms)
{
	int was_error;
	struct timespec elapsed, tv;
	elapsed.tv_sec = ms/1000;
	elapsed.tv_nsec = (ms%1000)*1000000;
	do {
		errno = 0;
		tv.tv_sec = elapsed.tv_sec;
		tv.tv_nsec = elapsed.tv_nsec;
		was_error = nanosleep(&tv, &elapsed);
	} while ( was_error && (errno == EINTR) );
}

static void copy_row2(unsigned short *src, int src_w, unsigned short *dst, int dst_w)
{
	int i;
	int pos, inc;
	unsigned short pixel = 0;

	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for ( i=dst_w; i>0; --i ) {
		while ( pos >= 0x10000L ) {
			pixel = *src++;
			pos -= 0x10000L;
		}
		*dst++ = pixel;
		pos += inc;
	}
}

int EZX_SoftStretch(void *src, int sx, int sy, int sw, int sh, int sp, void *dst, int dw, int dh)
{
	int pos, inc;
	int dst_width;
	int dst_maxrow;
	int src_row, dst_row;
	unsigned char *srcp = NULL;
	unsigned char *dstp;

	/* Set up the data... */
	pos = 0x10000;
	inc = (sh << 16) / dh;
	src_row = sy * sp; //srcrect->y;
	dst_row = 0; //dstrect->y;
	dst_width = dw * 2;

	/* Perform the stretch blit */
	for ( dst_maxrow = dst_row+dh; dst_row<dst_maxrow; ++dst_row ) {
		dstp = (unsigned char *)dst + (dst_row * 640);	// + (dtrect->x*bpp);
		while ( pos >= 0x10000L ) {
			srcp = (unsigned char *)src + src_row + sx * 2;
			src_row += sp;
			pos -= 0x10000L;
		}

		copy_row2((unsigned short *)srcp, sw, (unsigned short *)dstp, dw);
		pos += inc;
	}

	/* We need to unlock the surfaces if they're locked */
	return(0);
}

