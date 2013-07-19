#ifndef _GP2XSDK_
#define _GP2XSDK_

#include <math.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
//sq #include "usbjoy.h"

enum {
	GP2X_UP=0x1,
	GP2X_UP_LEFT=1<<1,
	GP2X_LEFT=1<<2,
	GP2X_DOWN_LEFT=1<<3,
	GP2X_DOWN=1<<4,
	GP2X_DOWN_RIGHT=1<<5,
	GP2X_RIGHT=1<<6,
	GP2X_UP_RIGHT=1<<7,
    GP2X_START=1<<8,
	GP2X_SELECT=1<<9,
	GP2X_L=1<<10,
	GP2X_R=1<<11,
    GP2X_A=1<<12,
	GP2X_B=1<<13,
	GP2X_X=1<<14,
	GP2X_Y=1<<15
};

// Default key and joystick mappings

#define RPI_KEY_A       SDLK_LCTRL
#define RPI_KEY_B       SDLK_SPACE
#define RPI_KEY_X       SDLK_LALT
#define RPI_KEY_Y       SDLK_LSHIFT
#define RPI_KEY_L       SDLK_z
#define RPI_KEY_R       SDLK_x
#define RPI_KEY_START   SDLK_RETURN
#define RPI_KEY_SELECT  SDLK_TAB
#define RPI_KEY_LEFT    SDLK_LEFT
#define RPI_KEY_RIGHT   SDLK_RIGHT
#define RPI_KEY_UP      SDLK_UP
#define RPI_KEY_DOWN    SDLK_DOWN
#define RPI_KEY_QUIT    SDLK_ESCAPE
#define RPI_KEY_ACCEL   SDLK_BACKSPACE

#define RPI_JOY_A       3
#define RPI_JOY_B       2
#define RPI_JOY_X       1
#define RPI_JOY_Y       0
#define RPI_JOY_L       4
#define RPI_JOY_R       6
#define RPI_JOY_START   9
#define RPI_JOY_SELECT  8
#define RPI_JOY_QUIT    5
#define RPI_JOY_ACCEL   7
#define RPI_JOY_QLOAD   10
#define RPI_JOY_QSAVE   11

// Key constants
#define A_1 0
#define B_1 1
#define X_1 2
#define Y_1 3
#define L_1 4
#define R_1 5
#define A_2 6
#define B_2 7
#define X_2 8
#define Y_2 9
#define L_2 10
#define R_2 11
#define START_1 12
#define SELECT_1 13
#define START_2 14
#define SELECT_2 15
#define UP_1 16
#define DOWN_1 17
#define LEFT_1 18
#define RIGHT_1 19
#define UP_2 20
#define DOWN_2 21
#define LEFT_2 22
#define RIGHT_2 23
#define LU_2 24
#define LD_2 25
#define RU_2 26
#define RD_2 27

#define ACCEL 50
#define QUIT 51
#define HELP 52

#define QLOAD 55
#define QSAVE 56

#define SHIFTL 30
#define SHIFTR 31
#define CTRLL 32
#define CTRLR 33

// Axis positions
#define CENTER  0
#define LEFT    1
#define RIGHT   2
#define UP      1
#define DOWN    2


#ifdef __cplusplus
 extern "C" {
#endif

void pi_initialize();
void pi_terminate(void);
int init_SDL(void);
void deinit_SDL(void);
void pi_deinit(void);
void pi_setvideo_mode(int hw, int vh);
unsigned long pi_joystick_read(void);
void pi_video_flip(void);
void pi_clear_framebuffers();
void * UpperMalloc(size_t size);
void UpperFree(void* mem);
int GetUpperSize(void* mem);
#ifdef __cplusplus
 } // End of extern "C"
#endif

#endif
