#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "burner.h"
#include "snd.h"
#include "config.h"

extern bool GameMute;
extern CFG_OPTIONS config_options;
extern int nBurnFPS;

int dspfd = -1;

unsigned short *EzxAudioBuffer;
volatile short *pOutput[8];

static int AudioCurBlock = 0;
static int nAudioChannels=2;

static bool exitSoundThread = false;
static pthread_t soundthread_p = (pthread_t) -1;

void *soundthread_f (void *a)
{
	while (!exitSoundThread)
	{
		AudioCurBlock++;

		if (AudioCurBlock >= 8) AudioCurBlock = 0;
		
		write(dspfd, (void *)pOutput[AudioCurBlock], EzxAudioBuffer[1]);
		ioctl(dspfd, SOUND_PCM_SYNC, 0); 
	}
	return NULL;
}

int SndInit()
{
	if (config_options.option_sound_enable)
	{
		if (BurnDrvGetHardwareCode() == HARDWARE_CAPCOM_CPS1)
		{
			nBurnSoundRate = 11025;
			nAudioChannels = 1;
		}
		else
		{
			switch(config_options.option_samplerate)
			{
				case 1:
					nBurnSoundRate = 22050;
				break;
				case 2:
					nBurnSoundRate = 44100;
				break;
				default:
					nBurnSoundRate = 11025;
				break;
			}
		}
		nBurnSoundLen = ((nBurnSoundRate * 100 /*+ 3000*/) / nBurnFPS );
	}
	pBurnSoundOut	= NULL;
	
	AudioCurBlock	= 0;
	
	dspfd = -1;

	return 0;
}

int SndOpen()
{
unsigned int BufferSize;
unsigned int bufferStart;

	BufferSize = (nBurnSoundLen * nAudioChannels * AUDIO_BLOCKS)*2+8;
	EzxAudioBuffer= (unsigned short *)malloc(BufferSize);
	gp2x_memset(EzxAudioBuffer,0,BufferSize);
	EzxAudioBuffer[1]=(EzxAudioBuffer[0]=(nBurnSoundLen * nAudioChannels * 2));
	EzxAudioBuffer[2]=(1000000000/nBurnSoundRate)&0xFFFF;
	EzxAudioBuffer[3]=(1000000000/nBurnSoundRate)>>16;
	bufferStart = (unsigned int)&EzxAudioBuffer[4];
	pOutput[0] = (short*)bufferStart;
	pOutput[1] = (short*)(bufferStart+1*EzxAudioBuffer[1]);
	pOutput[2] = (short*)(bufferStart+2*EzxAudioBuffer[1]);
	pOutput[3] = (short*)(bufferStart+3*EzxAudioBuffer[1]);
	pOutput[4] = (short*)(bufferStart+4*EzxAudioBuffer[1]);
	pOutput[5] = (short*)(bufferStart+5*EzxAudioBuffer[1]);
	pOutput[6] = (short*)(bufferStart+6*EzxAudioBuffer[1]);
	pOutput[7] = (short*)(bufferStart+7*EzxAudioBuffer[1]);
	if ( !GameMute )
	{
	int frag = 10 + config_options.option_samplerate;
    	frag |= 2 << 16;
        
return 0;

//		dspfd = ezx_open_dsp ( nBurnSoundRate, nAudioChannels, AUDIO_FORMAT, frag );
//		// printf("SOUND: Init done (%d)\n", dspfd);
//		if (dspfd >= 0)
//		{
//			pthread_create(&soundthread_p, NULL, &soundthread_f, NULL);
//			return 0;
//		}
//		else
		{
			nBurnSoundRate	= 0;
			nBurnSoundLen	= 0;
		}
	}
	return -1;
}

void SndClose()
{
	if (dspfd >= 0)
		ezx_close_dsp(dspfd);
	dspfd = -1;
}

void SndPlay()
{
}

void SndExit()
{
	exitSoundThread = true;
	usleep(1000000);
	if (dspfd >= 0)
		ezx_close_dsp(dspfd);

	pBurnSoundOut = NULL;
	dspfd = -1;
	free(EzxAudioBuffer);
}

int SegAim()
{
  int aim=AudioCurBlock; 

  aim--; if (aim<0) aim+=8;

  return aim;
}
