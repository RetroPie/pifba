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
 * $Id: ezxaudio.cpp,v 0.10 2006/07/07 $
 */
 
#include "ezxaudio.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sched.h>

#define SOUND_MIXER_HAPTICS_ON		0xe6
#define SOUND_MIXER_HAPTICS_OFF		0xe7
#define SOUND_MIXER_HAPTICS_FIL		0xf7		// e680 sound haptics filter control gpio state interface

#define SOUND_MIXER_HEADSET_STATUS	0xed

#define SOUND_MIXER_WRITE_HAPTICS_ON    MIXER_WRITE(SOUND_MIXER_HAPTICS_ON)
#define SOUND_MIXER_WRITE_HAPTICS_OFF   MIXER_WRITE(SOUND_MIXER_HAPTICS_OFF)
#define SOUND_MIXER_READ_HAPTICS_FIL	MIXER_READ(SOUND_MIXER_HAPTICS_FIL)  

#define SOUND_MIXER_WRITE_OUTSRC 		MIXER_WRITE(SOUND_MIXER_OUTSRC)
#define SOUND_MIXER_READ_HEADSET_STATUS	MIXER_READ(SOUND_MIXER_HEADSET_STATUS)

#define SOUND_MIXER_WRITE_OGAIN			MIXER_WRITE(SOUND_MIXER_OGAIN)

typedef enum{
    NO_HEADSET,
    MONO_HEADSET,
    STEREO_HEADSET
}input_enum;

typedef enum{
    HEADSET_OUT,			/* ear-phone : stereo headset */
    LOUDERSPEAKER_OUT,		/* A2_OUTPUT in a760 */
    EARPIECE_OUT,		    /* A1_OUTPUT in a760 */
    CARKIT_OUT,				/* A4_OUTPUT in a760 */
    HEADJACK_OUT,			/* mono headjack, HEADJACK_OUTPUT in a760 */
    BLUETOOTH_OUT,			/* bluetooth headset */
    LOUDERSPEAKER_MIXED_OUT	/* e680 louderspeaker out, boomer input is stereo+mono, mono is aimed for haptics signals */
}output_enum;

extern int GameGain;		/* main.cpp */
static int mixfd = -1;

int ezx_open_dsp (int rate, int chs, int fmt, int dsp_fragment)
{
  int x, y;
  int dspfd = -1;
  
  dspfd = open("/dev/dsp", O_WRONLY);
  if (dspfd < 0)
  {
    printf("can't open /dev/dsp");
    goto error;
  }
  
  x = fmt; // fmt; //AFMT_S16_LE;
  if (ioctl (dspfd, SNDCTL_DSP_SETFMT, &x) < 0)
  {
    printf("can't set format to 16-bit");
    goto error;
  }

  if ( dsp_fragment ) {
	//  x = 9 | 8 << 16;
  	x = dsp_fragment;
  	if (ioctl (dspfd, SNDCTL_DSP_SETFRAGMENT, &x) < 0)
  	{
      printf("can't set fragment size");
      goto error;
    }
  }

  x = chs - 1;
  if (ioctl (dspfd, SNDCTL_DSP_STEREO, &x) < 0)
  {
    printf("can't set to stereo");
    goto error;
  }
  
  x = rate; 
  if (ioctl (dspfd, SNDCTL_DSP_SPEED, &x) < 0)
  {
    printf("can't set sample rate");
    goto error;
  }
  
  mixfd = open("/dev/mixer", O_WRONLY);
  if (mixfd < 0)
  {
    printf("can't open /dev/mixer");
    goto error;
  }
	ezx_change_volume(0);
#if 0  
  int hstest;
  if ( ioctl(mixfd, SOUND_MIXER_READ_HEADSET_STATUS, &hstest) == -1 ) {
	printf("can't get SOUND_MIXER_READ_HEADSET_STATUS");
	goto error;
  }
  
  x = LOUDERSPEAKER_OUT;
  y = GameGain;
  if (y > 80) y = 80;
  if (y < 10) y = 10;
  
  if ( hstest == STEREO_HEADSET ) {
	x = HEADSET_OUT;
	y -= 10;
  }
  ioctl(mixfd, SOUND_MIXER_WRITE_OUTSRC, &x);
  ioctl(mixfd, SOUND_MIXER_WRITE_OGAIN, &y);
#endif

  printf("sound init done");
  return dspfd;
  
error:
  if (dspfd >= 0) close(dspfd);
  if (mixfd >= 0) close(mixfd);
  return -1;
}

void ezx_close_dsp (int dsp)
{
	close( mixfd );
	close(dsp);
}

int ezx_play_dsp( int dsp, void *buf, int size)
{
	return  (dsp < 0) ? size : write(dsp, buf, size);
}

void ezx_change_volume (int value)
{
static int volume = 50;
	volume += value;
	if (volume < 0)
		volume = 0;
	else if (volume > 100)
		volume = 100;
	int v=(((volume*0x50)/100)<<8)|((volume*0x50)/100);
	ioctl(mixfd, SOUND_MIXER_WRITE_PCM, &v);
}
