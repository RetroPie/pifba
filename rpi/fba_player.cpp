/*
 * FinalBurn Alpha for MOTO EZX Modile Phone
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
 * $Id: fba_player.cpp,v 0.10 2006/12/03 $
 */

#define CALC_FPS

#include <stdio.h>
#include <stdlib.h>
#include "fba_player.h"
#include "font.h"
#include "snd.h"

#include <SDL.h>

#include "burnint.h"
#include "config.h"
#include "cache.h"

extern "C"
{
#include "gp2xsdk.h"
};

void uploadfb(void);

extern unsigned int nFramesRendered;
static int frame_count = 0;
unsigned int FBA_KEYPAD[4];
unsigned char ServiceRequest = 0;
unsigned char P1P2Start = 0;
unsigned short titlefb[320][240];
extern bool bShowFPS;
void ChangeFrameskip();
extern SDL_Joystick *joys[4];
extern char joyCount;
extern CFG_OPTIONS config_options;

void logoutput(const char *text,...);
void logflush(void);

void do_keypad()
{
	unsigned long joy;
	int bVert = 0;
	
	FBA_KEYPAD[0] = 0;
	FBA_KEYPAD[1] = 0;
	FBA_KEYPAD[2] = 0;
	FBA_KEYPAD[3] = 0;
	ServiceRequest = 0;
	P1P2Start = 0;
	
	//for (int i=0;i<joyCount;i++) {
	//Always check for two players, i.e. keyboard input
	for (int i=0;i<joyCount;i++) {

		joy = pi_joystick_read(i);

		if ( joy & GP2X_UP ) 	FBA_KEYPAD[i] |= bVert?0x0004:0x0001;
		if ( joy & GP2X_DOWN ) 	FBA_KEYPAD[i] |= bVert?0x0008:0x0002;
		if ( joy & GP2X_LEFT ) 	FBA_KEYPAD[i] |= bVert?0x0002:0x0004;
		if ( joy & GP2X_RIGHT ) FBA_KEYPAD[i] |= bVert?0x0001:0x0008;
		
		if ( joy & GP2X_SELECT )	FBA_KEYPAD[i] |= 0x0010;	
		if ( joy & GP2X_START )		FBA_KEYPAD[i] |= 0x0020;	
		
		if ( joy & GP2X_A )	FBA_KEYPAD[i] |= 0x0040;	// A
		if ( joy & GP2X_X )	FBA_KEYPAD[i] |= 0x0080;	// B
		if ( joy & GP2X_B )	FBA_KEYPAD[i] |= 0x0100;	// C
		if ( joy & GP2X_Y )	FBA_KEYPAD[i] |= 0x0200;	// D
		if ( joy & GP2X_L )	FBA_KEYPAD[i] |= 0x0400;    // E
		if ( joy & GP2X_R )	FBA_KEYPAD[i] |= 0x0800;    // F

		if ( joy & GP2X_L && joy & GP2X_R)
		{
			if ( joy & GP2X_SELECT) ServiceRequest = 1;
		}
	//sq	else
	//sq		if (joy & GP2X_START && joy & GP2X_SELECT) P1P2Start = 1;
		if ( joy & GP2X_START && joy & GP2X_SELECT) GameLooping = false;

	}
    
//sq
//	for (int i=0;i<joyCount;i++)
//	{
//        int numButtons = joy_buttons(joys[i]);
//		if (numButtons > 8)
//			numButtons = 8;
//		joy_update(joys[i]);
//		if(joy_getaxe(JOYUP, joys[i])) FBA_KEYPAD[i] |= bVert?0x0004:0x0001;
//		if(joy_getaxe(JOYDOWN, joys[i])) FBA_KEYPAD[i] |= bVert?0x0008:0x0002;
//		if(joy_getaxe(JOYLEFT, joys[i])) FBA_KEYPAD[i] |= bVert?0x0002:0x0004;
//		if(joy_getaxe(JOYRIGHT, joys[i])) FBA_KEYPAD[i] |= bVert?0x0001:0x0008;
//
//		for (int nButton = 0; nButton < numButtons; nButton++) {
//			if(joy_getbutton(nButton, joys[i]))
//				FBA_KEYPAD[i] |= joyMap[nButton];
//		}
//	}
    
}

int DrvInit(int nDrvNum, bool bRestore);
int DrvExit();

int RunReset();
int RunOneFrame(bool bDraw, int fps);

int VideoInit();
void VideoExit();
 
int InpInit();
int InpExit();
void InpDIP();

void update_audio_stream(INT16 *buffer);
void pi_process_events (void);

extern char szAppRomPath[];
extern int nBurnFPS;
int fps=0;

void show_rom_loading_text(char * szText, int nSize, int nTotalSize)
{
	static long long size = 0;
	//printf("!!! %s, %d / %d\n", szText, size + nSize, nTotalSize);

	DrawRect((uint16 *) titlefb, 20, 120, 300, 20, 0, 320);
	
	if (szText)
		DrawString (szText, (uint16 *) titlefb, 20, 120, 320);
	
	if (nTotalSize == 0) {
		size = 0;
		DrawRect((uint16 *) titlefb, 20, 140, 280, 12, 0x00FFFFFF, 320);
		DrawRect((uint16 *) titlefb, 21, 141, 278, 10, 0x00808080, 320);
	} else {
		size += nSize;
		if (size > nTotalSize) size = nTotalSize;
		DrawRect((uint16 *) titlefb, 21, 141, size * 278 / nTotalSize, 10, 0x00FFFF00, 320);
	}

	memcpy (VideoBuffer, titlefb, 320*240*2); 
	pi_video_flip();
}



void run_fba_emulator(const char *fn)
{
	char build_version[] = "Finalburn Alpha Plus for Pi  ("__DATE__")";

	// process rom path and name
	char romname[MAX_PATH];
	if (BurnCacheInit(fn, romname))
		goto finish;

	if(config_options.option_showfps)
		bShowFPS=true;

	BurnLibInit();
	
	// find rom by name
	for (nBurnDrvSelect=0; nBurnDrvSelect<nBurnDrvCount; nBurnDrvSelect++)
		if ( strcasecmp(romname, BurnDrvGetTextA(DRV_NAME)) == 0 )
			break;
	if (nBurnDrvSelect >= nBurnDrvCount) {
		// unsupport rom ...
		nBurnDrvSelect = ~0U;
		logoutput ("rom not supported!\n");
		printf ("rom not supported!\n");
		goto finish;
	}
	
	logoutput("Attempt to initialise '%s'\n", BurnDrvGetTextA(DRV_FULLNAME));
	
	memset (titlefb, 0, 320*240*2);
	DrawString (build_version, (uint16*)&titlefb, 10, 20, 320);
	DrawString ("Based on FinalBurnAlpha", (uint16*)&titlefb, 10, 35, 320);
	DrawString ("Now loading ... ", (uint16 *)&titlefb, 10, 105, 320);	
	show_rom_loading_text("Open Zip", 0, 0);
	memcpy (VideoBuffer, titlefb, 320*240*2); 
	pi_video_flip();
	 	
	InpInit();
	InpDIP();
	
	VideoInit();

	if (DrvInit(nBurnDrvSelect, false) != 0) {
		logoutput ("Driver initialisation failed! Likely causes are:\n- Corrupt/Missing ROM(s)\n- I/O Error\n- Memory error\n\n");
		printf ("Driver initialisation failed! Likely causes are:\n- Corrupt/Missing ROM(s)\n- I/O Error\n- Memory error\n\n");
		goto finish;
	}

	RunReset();

	frame_count = 0;
	GameLooping = true;

	EZX_StartTicks();

	logoutput ("Lets go!\n");
	logflush();

	if (config_options.option_sound_enable)
	{
		int timer = 0, tick=0, i=0, fps = 0;
		bool bRenderFrame;

		if (SndOpen() == 0)
		{		
			while (GameLooping)
			{
					if (bShowFPS)
					{
						timer = EZX_GetTicks();
						if(timer-tick>1000000)
						{
							fps = nFramesRendered;
							nFramesRendered = 0;
							tick = timer;
						}
					}
					//We need to render more audio:  
		
					bRenderFrame=true; // Render last frame
					RunOneFrame(bRenderFrame,fps);

					update_audio_stream(pBurnSoundOut);
					pi_process_events();
			}
		}
	}
	else
	{
		int now, done=0, timer = 0, ticks=0, tick=0, i=0, fps = 0;
		unsigned int frame_limit = nBurnFPS/100, frametime = 100000000/nBurnFPS;
		
		while (GameLooping)
		{
			timer = EZX_GetTicks()/frametime;;
			if(timer-tick>frame_limit && bShowFPS)
			{
				fps = nFramesRendered;
				nFramesRendered = 0;
				tick = timer;
			}
			now = timer;
			ticks=now-done;
			if(ticks<1) continue;
			if(ticks>10) ticks=10;
			for (i=0; i<ticks-1; i++)
			{
				RunOneFrame(false,fps);	
			} 
			if(ticks>=1)
			{
				RunOneFrame(true,fps);	
			}
			
			done = now;
		}
	}
	
	logoutput ("Finished emulating\n");
	
finish:
	logoutput("---- Shutdown Finalburn Alpha plus ----\n\n");
	DrvExit();
	BurnLibExit();

	if (config_options.option_sound_enable)
		SndExit();
	VideoExit();
	InpExit();
	BurnCacheExit();
}
