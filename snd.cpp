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

#include <alsa/asoundlib.h>

extern "C" {
#include "fifo_buffer.h"
#include "thread.h"
}

// buffer over/underflow counts
static int fifo_underrun;
static int fifo_overrun;
static int snd_underrun;

static int samples_per_frame;

typedef struct alsa
{
	snd_pcm_t *pcm;
	bool has_float;
	volatile bool thread_dead;
	
	size_t buffer_size_bytes;
	size_t period_size_bytes;
	snd_pcm_uframes_t period_size_frames;
	
	fifo_buffer_t *buffer;
	sthread_t *worker_thread;
	slock_t *fifo_lock;
	scond_t *cond;
	slock_t *cond_lock;
} alsa_t;

static alsa_t *g_alsa;

static alsa_t *alsa_init(void);
static void alsa_worker_thread(void *data);
static void alsa_free(void *data);

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

#define logerror printf
#define TRY_ALSA(x) if (x < 0) { \
goto error; \
}

//void *soundthread_f (void *a)
//{
//	while (!exitSoundThread)
//	{
//		AudioCurBlock++;
//
//		if (AudioCurBlock >= 8) AudioCurBlock = 0;
//		
//		write(dspfd, (void *)pOutput[AudioCurBlock], EzxAudioBuffer[1]);
//		ioctl(dspfd, SOUND_PCM_SYNC, 0); 
//	}
//	return NULL;
//}

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
//sq		nBurnSoundLen = ((nBurnSoundRate * 100 /*+ 3000*/) / nBurnFPS );
		samples_per_frame = (nBurnSoundRate / (nBurnFPS/100) );
//sq	pBurnSoundOut	= NULL;
    
        // attempt to initialize SDL
        // alsa_init will also ammend the samples_per_frame
        g_alsa = alsa_init();
        
        nBurnSoundLen = samples_per_frame;
        
        pBurnSoundOut = (short *)malloc(nBurnSoundLen*4);
        
        AudioCurBlock	= 0;
        
        dspfd = -1;
	}

	return 0;
}

int SndOpen()
{
    if (g_alsa)
        return 0;
    else
        return -1;
    
//unsigned int BufferSize;
//unsigned int bufferStart;
//
//	BufferSize = (nBurnSoundLen * nAudioChannels * AUDIO_BLOCKS)*2+8;
//	EzxAudioBuffer= (unsigned short *)malloc(BufferSize);
//	gp2x_memset(EzxAudioBuffer,0,BufferSize);
//	EzxAudioBuffer[1]=(EzxAudioBuffer[0]=(nBurnSoundLen * nAudioChannels * 2));
//	EzxAudioBuffer[2]=(1000000000/nBurnSoundRate)&0xFFFF;
//	EzxAudioBuffer[3]=(1000000000/nBurnSoundRate)>>16;
//	bufferStart = (unsigned int)&EzxAudioBuffer[4];
//	pOutput[0] = (short*)bufferStart;
//	pOutput[1] = (short*)(bufferStart+1*EzxAudioBuffer[1]);
//	pOutput[2] = (short*)(bufferStart+2*EzxAudioBuffer[1]);
//	pOutput[3] = (short*)(bufferStart+3*EzxAudioBuffer[1]);
//	pOutput[4] = (short*)(bufferStart+4*EzxAudioBuffer[1]);
//	pOutput[5] = (short*)(bufferStart+5*EzxAudioBuffer[1]);
//	pOutput[6] = (short*)(bufferStart+6*EzxAudioBuffer[1]);
//	pOutput[7] = (short*)(bufferStart+7*EzxAudioBuffer[1]);
//	if ( !GameMute )
//	{
//	int frag = 10 + config_options.option_samplerate;
//    	frag |= 2 << 16;
//        
//return 0;
//
////		dspfd = ezx_open_dsp ( nBurnSoundRate, nAudioChannels, AUDIO_FORMAT, frag );
////		// printf("SOUND: Init done (%d)\n", dspfd);
////		if (dspfd >= 0)
////		{
////			pthread_create(&soundthread_p, NULL, &soundthread_f, NULL);
////			return 0;
////		}
////		else
//		{
//			nBurnSoundRate	= 0;
//			nBurnSoundLen	= 0;
//		}
//	}
//	return -1;
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
//	exitSoundThread = true;
//	usleep(1000000);
//	if (dspfd >= 0)
//		ezx_close_dsp(dspfd);
//
//	pBurnSoundOut = NULL;
//	dspfd = -1;
//	free(EzxAudioBuffer);
    
    alsa_free(g_alsa);
    if(pBurnSoundOut)
        free(pBurnSoundOut);
    pBurnSoundOut=NULL;
}

int SegAim()
{
  int aim=AudioCurBlock;

  aim--; if (aim<0) aim+=8;

  return aim;
}

#define min(a, b) ((a) < (b) ? (a) : (b))

static void alsa_worker_thread(void *data)
{
	alsa_t *alsa = (alsa_t*)data;
	int wait_on_buffer=1;
	size_t fifo_size;
    
	UINT8 *buf = (UINT8 *)calloc(1, alsa->period_size_bytes);
	if (!buf)
	{
		logerror("failed to allocate audio buffer");
		goto end;
	}
	
	while (!alsa->thread_dead)
	{
		slock_lock(alsa->fifo_lock);
		size_t avail = fifo_read_avail(alsa->buffer);
        
		//First run wait until the buffer is filled with a few frames
		if(avail < alsa->period_size_bytes*3 && wait_on_buffer)
		{
			slock_unlock(alsa->fifo_lock);
			continue;
		}
		wait_on_buffer=0;
        
		if(avail < alsa->period_size_bytes)
		{
			slock_unlock(alsa->fifo_lock);
			fifo_size = 0;
		}
		else
		{
			fifo_size = min(alsa->period_size_bytes, avail);
			if(fifo_size > alsa->period_size_bytes)
				fifo_size = alsa->period_size_bytes;
			fifo_read(alsa->buffer, buf, fifo_size);
			scond_signal(alsa->cond);
			slock_unlock(alsa->fifo_lock);
		}
	    
		// If underrun, fill rest with silence.
 		if(alsa->period_size_bytes != fifo_size) {
			memset(buf + fifo_size, 0, alsa->period_size_bytes - fifo_size);
 			fifo_underrun++;
 		}
        
		snd_pcm_sframes_t frames = snd_pcm_writei(alsa->pcm, buf, alsa->period_size_frames);
		if (frames == -EPIPE || frames == -EINTR || frames == -ESTRPIPE)
		{
			snd_underrun++;
			if (snd_pcm_recover(alsa->pcm, frames, 1) < 0)
			{
				logerror("[ALSA]: (#2) Failed to recover from error (%s)\n",
                         snd_strerror(frames));
				break;
			}
            
			continue;
		}
		else if (frames < 0)
		{
			logerror("[ALSA]: Unknown error occured (%s).\n", snd_strerror(frames));
			break;
		}
	}
	
end:
	slock_lock(alsa->cond_lock);
	alsa->thread_dead = true;
	scond_signal(alsa->cond);
	slock_unlock(alsa->cond_lock);
	free(buf);
}

static ssize_t alsa_write(void *data, const void *buf, size_t size)
{
	alsa_t *alsa = (alsa_t*)data;
	
	if (alsa->thread_dead)
		return -1;
	
	slock_lock(alsa->fifo_lock);
	size_t avail = fifo_write_avail(alsa->buffer);
	size_t write_amt = min(avail, size);
	if(write_amt)
		fifo_write(alsa->buffer, buf, write_amt);
	slock_unlock(alsa->fifo_lock);
	if(write_amt<size)
	{
		fifo_overrun++;
	}
	return write_amt;
}

int update_audio_stream(INT16 *buffer)
{
    alsa_write(g_alsa, buffer, (nBurnSoundLen * nAudioChannels * sizeof(signed short)) );
}

static alsa_t *alsa_init(void)
{
	alsa_t *alsa = (alsa_t*)calloc(1, sizeof(alsa_t));
	if (!alsa)
		return NULL;
    
	fifo_underrun=0;
	fifo_overrun=0;
	snd_underrun=0;
	
	snd_pcm_hw_params_t *params = NULL;
	
	const char *alsa_dev = "default";
	
	snd_pcm_uframes_t buffer_size_frames;
	
	TRY_ALSA(snd_pcm_open(&alsa->pcm, alsa_dev, SND_PCM_STREAM_PLAYBACK, 0));
    
	TRY_ALSA(snd_pcm_hw_params_malloc(&params));
    
	//latency is one frame times by a multiplier (higher improves crackling?)
	TRY_ALSA(snd_pcm_set_params(alsa->pcm,
								SND_PCM_FORMAT_S16,
								SND_PCM_ACCESS_RW_INTERLEAVED,
								nAudioChannels,
								nBurnSoundRate,
								0,
								((float)1000000 / (float)(nBurnFPS/100)) * 4)) ;
    
	TRY_ALSA(snd_pcm_get_params ( alsa->pcm, &buffer_size_frames, &alsa->period_size_frames ));
    
	//SQ Adjust MAME sound engine to what ALSA says its frame size is, ALSA
	//tends to be even whereas MAME uses odd - based on the frame & sound rates.
	samples_per_frame = (int)alsa->period_size_frames;
    
	logerror("ALSA: Period size: %d frames\n", (int)alsa->period_size_frames);
	logerror("ALSA: Buffer size: %d frames\n", (int)buffer_size_frames);
    
	alsa->buffer_size_bytes = snd_pcm_frames_to_bytes(alsa->pcm, buffer_size_frames);
	alsa->period_size_bytes = snd_pcm_frames_to_bytes(alsa->pcm, alsa->period_size_frames);
    
	logerror("ALSA: Period size: %d bytes\n", (int)alsa->period_size_bytes);
	logerror("ALSA: Buffer size: %d bytes\n", (int)alsa->buffer_size_bytes);
    
	TRY_ALSA(snd_pcm_prepare(alsa->pcm));
    
	snd_pcm_hw_params_free(params);
    
	//Write initial blank sound to stop underruns?
	{
		void *tempbuf;
		tempbuf=calloc(1, alsa->period_size_bytes*3);
		snd_pcm_writei (alsa->pcm, tempbuf, 2 * alsa->period_size_frames);
		free(tempbuf);
	}
    
	alsa->fifo_lock = slock_new();
	alsa->cond_lock = slock_new();
	alsa->cond = scond_new();
	alsa->buffer = fifo_new(alsa->buffer_size_bytes*6);
	if (!alsa->fifo_lock || !alsa->cond_lock || !alsa->cond || !alsa->buffer)
		goto error;
	
	alsa->worker_thread = sthread_create(alsa_worker_thread, alsa);
	if (!alsa->worker_thread)
	{
		logerror("error initializing worker thread\n");
		goto error;
	}
	
	return alsa;
	
error:
	logerror("ALSA: Failed to initialize...\n");
	if (params)
		snd_pcm_hw_params_free(params);
	
	alsa_free(alsa);
	
	return NULL;
    
}

static void alsa_free(void *data)
{
	alsa_t *alsa = (alsa_t*)data;
	
	if (alsa)
	{
		if (alsa->worker_thread)
		{
			alsa->thread_dead = true;
			sthread_join(alsa->worker_thread);
		}
		if (alsa->buffer)
			fifo_free(alsa->buffer);
		if (alsa->cond)
			scond_free(alsa->cond);
		if (alsa->fifo_lock)
			slock_free(alsa->fifo_lock);
		if (alsa->cond_lock)
			slock_free(alsa->cond_lock);
		if (alsa->pcm)
		{
			snd_pcm_drop(alsa->pcm);
			snd_pcm_close(alsa->pcm);
		}
		free(alsa);
	}
}

