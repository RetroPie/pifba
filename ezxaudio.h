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
 * $Id: ezxaudio.h,v 0.10 2006/07/07 $
 */
 
#ifndef _EMU_EZX_AUDIO_H_
#define _EMU_EZX_AUDIO_H_

#include <sys/soundcard.h>

int ezx_open_dsp (int, int, int fmt = AFMT_S16_LE, int dsp_fragment = 0);
void ezx_close_dsp (int );
int ezx_play_dsp( int , void *, int );
void ezx_change_volume (int value);

#endif // _EMU_EZX_AUDIO_H_
