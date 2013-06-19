// Run module
#include "burner.h"
#include "gamewidget.h"
#include "snd.h"
#include "font.h"
#include "gp2xsdk.h"
#include "gp2xmemfuncs.h"
#include "config.h"

extern int fps;
extern unsigned int FBA_KEYPAD[4];
extern CFG_OPTIONS config_options;
extern void do_keypad();

static int VideoBufferWidth = 0;
static int VideoBufferHeight = 0;
static int PhysicalBufferWidth = 0;


//static unsigned short BurnVideoBuffer[384 * 224];	// think max enough
static unsigned short * BurnVideoBuffer = NULL;	// think max enough
static bool BurnVideoBufferAlloced = false;

bool bShowFPS = true;

int InpMake(unsigned int[]);
void uploadfb(void);
void VideoBufferUpdate(void);
void VideoTrans();

int RunReset()
{
	if (VideoBufferWidth == 384 && (config_options.option_rescale == 2 || VideoBufferHeight == 240))
		gp2x_setvideo_mode(384,240);
	else
	if (VideoBufferWidth == 448)
		gp2x_setvideo_mode(448,240);
	nFramesEmulated = 0;
	nCurrentFrame = 0;
	nFramesRendered = 0;	
	
	return 0;
}

int RunOneFrame(bool bDraw, int fps) 
{
	do_keypad();
	InpMake(FBA_KEYPAD);

	nFramesEmulated++;
	nCurrentFrame++;
	
	pBurnDraw = NULL;
	if ( bDraw )
	{
		nFramesRendered++;
		VideoBufferUpdate();
		pBurnDraw = (unsigned char *)&BurnVideoBuffer[0];
	}
	
	BurnDrvFrame();
	
	pBurnDraw = NULL;
	if ( bDraw )
	{
		VideoTrans();
		if (bShowFPS)
		{
			char buf[10];
			int x;
			sprintf(buf, "FPS: %2d", fps);
			//draw_text(buf, x, 0, 0xBDF7, 0x2020);
			DrawRect((uint16 *) (unsigned short *) &VideoBuffer[0],0, 0, 60, 9, 0,PhysicalBufferWidth);
			DrawString (buf, (unsigned short *) &VideoBuffer[0], 0, 0,PhysicalBufferWidth);
		}
		gp2x_video_flip();
	}
/*	if (config_options.option_sound_enable)
		SndPlay();
*/	return 0;
}

// --------------------------------

static unsigned int HighCol16(int r, int g, int b, int  /* i */)
{
	unsigned int t;

	t  = (r << 8) & 0xF800;
	t |= (g << 3) & 0x07E0;
	t |= (b >> 3) & 0x001F;

	return t;
}

static void BurnerVideoTransDemo(){}

static void (*BurnerVideoTrans) () = BurnerVideoTransDemo;

//typedef unsigned long long UINT64;

static void BurnerVideoTrans384x224Clip() 
{
	// CPS1 & CPS2 384x224
	unsigned short * p = &VideoBuffer[2560];
	unsigned short * q = &BurnVideoBuffer[32];
	
	for (int i=0; i<224; i++,p+=320,q+=384)
	{
		gp2x_memcpy(p,q,640);
	}
}

static void BurnerVideoTrans384x224SW() 
{
#define COLORMIX(a, b) ( ((((a & 0xF81F) + (b & 0xF81F)) >> 1) & 0xF81F) | ((((a & 0x07E0) + (b & 0x07E0)) >> 1) & 0x07E0) )
	// CPS1 & CPS2 384x224
	unsigned short * p = &VideoBuffer[2560];
	unsigned short * q = BurnVideoBuffer;
	
	for (int i=0; i<224; i++)
		for (int j=0; j<64; j++) {
			//*((UINT64 *)p) = *((UINT64 *)q);
			p[0] = q[0];
			p[1] = q[1];
			p[2] = q[2];
			p[3] = COLORMIX(q[3],q[4]);
			p[4] = q[5];
			p += 5;
			q += 6;
		}
}

static void BurnerVideoTrans304x224() 
{
	// SNK Neogeo 308x224
	unsigned short * p = &VideoBuffer[2568];
	unsigned short * q = &BurnVideoBuffer[0];
	for (int i=0; i<224; i ++) {
		gp2x_memcpy( p, q, 304 * 2 );
		p += 320;
		q += 304;
	}
}

/*
static void BurnerVideoTrans320x224() 
{
	// Cave gaia 320x224
	unsigned short * p = &VideoBuffer[8][0];
	unsigned short * q = &BurnVideoBuffer[0];
	gp2x_memcpy( p, q, 320 * 224 * 2 );
}

static void BurnerVideoTrans320x240() 
{
	// Cave & Toaplan 320x240
	unsigned short * p = &VideoBuffer[0][0];
	unsigned short * q = &BurnVideoBuffer[0];
	gp2x_memcpy( p, q, 320 * 240 * 2 );
}

*/

static void BurnerVideoTrans256x224() 
{
	// 256x224
	unsigned short * p = &VideoBuffer[2592];
	unsigned short * q = &BurnVideoBuffer[0];
	for (int i=0; i<224; i ++) {
		gp2x_memcpy( p, q, 256 * 2 );
		p += 320;
		q += 256;
	}
}

static void BurnerVideoTrans384x240Flipped() 
{
	// 384x240
	register unsigned short * p = &VideoBuffer[0];
	register unsigned short * q = &BurnVideoBuffer[92159]; //384*240-1
	for (int x = 92160; x > 0; x --) {
		*p++ = *q--;
	}
}

static void BurnerVideoTrans256x224Flipped() 
{
	// 256x224
	register unsigned short * p = &VideoBuffer[2592];
	register unsigned short * q = &BurnVideoBuffer[57343]; //256*224-1
	for (int y = 224; y > 0; y--, p += 64) {
		for (int x = 256; x > 0; x --) {
			*p++ = *q--;
		}
	}
}

static void BurnerVideoTrans256x256() 
{
	// 256x256
	unsigned short * p = &VideoBuffer[32];
	unsigned short * q = &BurnVideoBuffer[0];
	for (int i=0; i<240; i ++) {
		gp2x_memcpy( p, q, 256 * 2 );
		p += 320;
		q += 256;
	}
}

static void BurnerVideoTrans448x224() 
{
	// IGS 448x224
/*
	unsigned short * p = &VideoBuffer[2560];
	unsigned short * q = BurnVideoBuffer;
	
	for (int i=0; i<224; i++)
		for (int j=0; j<64; j++) {
			p[0] = q[0];
			p[1] = q[1];
			p[2] = q[3];
			p[3] = q[4];
			p[4] = q[6];
			p += 5;
			q += 7;
		}
*/
	gp2x_memcpy(&VideoBuffer[0],BurnVideoBuffer,448*224*2);
}

static void BurnerVideoTrans352x240() 
{
	// V-System 352x240
	unsigned short * p = &VideoBuffer[0];
	unsigned short * q = BurnVideoBuffer;
	
	for (int i=0; i<240; i++)
		for (int j=0; j<32; j++) {
			p[0] = q[0];
			p[1] = q[1];
			p[2] = q[2];
			p[3] = q[3];
			p[4] = q[4];
			p[5] = q[5];
			p[6] = q[6];
			p[7] = q[7];
			p[8] = q[8];
			p[9] = q[10];
			p += 10;
			q += 11;
		}
}

int VideoInit()
{
	BurnDrvGetFullSize(&VideoBufferWidth, &VideoBufferHeight);
	
//sq
	printf("Screen Size: %d x %d\n", VideoBufferWidth, VideoBufferHeight);
	
	nBurnBpp = 2;
	BurnHighCol = HighCol16;
	
	BurnRecalcPal();
	
	if (VideoBufferWidth == 384 && VideoBufferHeight == 224) {
		// CPS1 & CPS2
		nBurnPitch  = VideoBufferWidth * 2;
		switch(config_options.option_rescale)
		{
			case 0:
				BurnVideoBuffer = (unsigned short *)malloc( VideoBufferWidth * VideoBufferHeight * 2 );
				BurnVideoBufferAlloced = true;
				BurnerVideoTrans = BurnerVideoTrans384x224Clip;
				PhysicalBufferWidth = 320;
			break;
			case 1:
				BurnVideoBuffer = (unsigned short *)malloc( VideoBufferWidth * VideoBufferHeight * 2 );
				BurnVideoBufferAlloced = true;
				BurnerVideoTrans = BurnerVideoTrans384x224SW;
				PhysicalBufferWidth = 320;
			break;
			default:
				BurnVideoBuffer = &VideoBuffer[3072];
				BurnVideoBufferAlloced = false;
				BurnerVideoTrans = BurnerVideoTransDemo;
				PhysicalBufferWidth	= VideoBufferWidth;						
		}
	} 
	else if (VideoBufferWidth == 384 && VideoBufferHeight == 240) {
		// Cave
		nBurnPitch  = VideoBufferWidth * 2;

		if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED)
		{
			BurnVideoBuffer = (unsigned short *)malloc( VideoBufferWidth * VideoBufferHeight * 2 );
			BurnVideoBufferAlloced = true;
			BurnerVideoTrans = BurnerVideoTrans384x240Flipped;				
		}
		else
		{
			BurnVideoBuffer = &VideoBuffer[0];
			BurnVideoBufferAlloced = false;
			BurnerVideoTrans = BurnerVideoTransDemo;
		}
		PhysicalBufferWidth	= VideoBufferWidth;						
	} 
	else if (VideoBufferWidth == 304 && VideoBufferHeight == 224) {
		// Neogeo
		BurnVideoBuffer = (unsigned short *)malloc( VideoBufferWidth * VideoBufferHeight * 2 );
		BurnVideoBufferAlloced = true;
		nBurnPitch  = VideoBufferWidth * 2;
		BurnerVideoTrans = BurnerVideoTrans304x224;
		PhysicalBufferWidth = 320;
	} 
	else if (VideoBufferWidth == 320 && VideoBufferHeight == 224) {
		// Cave gaia & Neogeo with NEO_DISPLAY_OVERSCAN
		BurnVideoBuffer = &VideoBuffer[2560];
		BurnVideoBufferAlloced = false;
		nBurnPitch  = VideoBufferWidth * 2;
		BurnerVideoTrans = BurnerVideoTransDemo;
		PhysicalBufferWidth = 320;
	} 
	else if (VideoBufferWidth == 320 && VideoBufferHeight == 240) {
		// Cave & Toaplan
		BurnVideoBuffer = &VideoBuffer[0];
		BurnVideoBufferAlloced = false;
		nBurnPitch  = VideoBufferWidth * 2;
		BurnerVideoTrans = BurnerVideoTransDemo;
		PhysicalBufferWidth = 320;
	} 	
	else if (VideoBufferWidth == 256 && VideoBufferHeight == 224) {
		// Galpanic
		BurnVideoBuffer = (unsigned short *)malloc( VideoBufferWidth * VideoBufferHeight * 2 );
		BurnVideoBufferAlloced = true;
		nBurnPitch  = VideoBufferWidth * 2;
		if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED)
			BurnerVideoTrans = BurnerVideoTrans256x224Flipped;
		else		
			BurnerVideoTrans = BurnerVideoTrans256x224;
		PhysicalBufferWidth = 320;
	} 
	else if (VideoBufferWidth == 256 && VideoBufferHeight == 256) {
		BurnVideoBuffer = (unsigned short *)malloc( VideoBufferWidth * VideoBufferHeight * 2 );
		BurnVideoBufferAlloced = true;
		nBurnPitch  = VideoBufferWidth * 2;
		BurnerVideoTrans = BurnerVideoTrans256x256;
		PhysicalBufferWidth = 320;
	} 
	else if (VideoBufferWidth == 448 && VideoBufferHeight == 224) {
		// IGS
		nBurnPitch  = VideoBufferWidth * 2;
		BurnVideoBuffer = &VideoBuffer[3584];
		BurnVideoBufferAlloced = false;
		BurnerVideoTrans = BurnerVideoTransDemo;
		PhysicalBufferWidth	= VideoBufferWidth;						
	} 
	else if (VideoBufferWidth == 352 && VideoBufferHeight == 240) {
		// V-Systom
		BurnVideoBuffer = (unsigned short *)malloc( VideoBufferWidth * VideoBufferHeight * 2 );
		BurnVideoBufferAlloced = true;
		nBurnPitch  = VideoBufferWidth * 2;
		BurnerVideoTrans = BurnerVideoTrans352x240;
		PhysicalBufferWidth = 320;
	} 
	else {
		BurnVideoBuffer = NULL;
		BurnVideoBufferAlloced = false;
		nBurnPitch  = VideoBufferWidth * 2;
		BurnerVideoTrans = BurnerVideoTransDemo;
		PhysicalBufferWidth = 320;
	}
		
	return 0;
}

// 'VideoBuffer' is updated each frame due to double buffering, so we sometimes need to ensure BurnVideoBuffer is updated too.
void VideoBufferUpdate (void)
{
	if (BurnVideoBufferAlloced == false) 
	{
		if (VideoBufferWidth == 384 && VideoBufferHeight == 224)
		{
			BurnVideoBuffer = &VideoBuffer[3072];
		}
		else if (VideoBufferWidth == 384 && VideoBufferHeight == 240)
		{
			BurnVideoBuffer = &VideoBuffer[0];
		}
		else if (VideoBufferWidth == 320 && VideoBufferHeight == 240) 
		{
			BurnVideoBuffer = &VideoBuffer[0];
		}
		else if (VideoBufferWidth == 320 && VideoBufferHeight == 224)
		{
			BurnVideoBuffer = &VideoBuffer[2560];
		}
		else if (VideoBufferWidth == 448)
		{
			BurnVideoBuffer = &VideoBuffer[3584];
		}
		else
		{
			BurnVideoBuffer = NULL;
		}
	}
}

void VideoTrans()
{
	BurnerVideoTrans();
}

void VideoExit()
{
	if ( BurnVideoBufferAlloced )
		free(BurnVideoBuffer);
	BurnVideoBuffer = NULL;
	BurnVideoBufferAlloced = false;
	BurnerVideoTrans = BurnerVideoTransDemo;
}

void ChangeFrameskip()
{
	bShowFPS = !bShowFPS;
//	DrawRect((uint16 *) (unsigned short *) &VideoBuffer[0],0, 0, 60, 9, 0,VideoBufferWidth);
	gp2x_clear_framebuffers();
	nFramesRendered = 0;
}
