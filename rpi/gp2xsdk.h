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

//player 2 keyboard defaults to disabled
#define RPI_KEY_A_2       999
#define RPI_KEY_B_2       999
#define RPI_KEY_X_2       999
#define RPI_KEY_Y_2       999
#define RPI_KEY_L_2       999
#define RPI_KEY_R_2       999
#define RPI_KEY_START_2   999
#define RPI_KEY_SELECT_2  999
#define RPI_KEY_LEFT_2    999
#define RPI_KEY_RIGHT_2   999
#define RPI_KEY_UP_2      999
#define RPI_KEY_DOWN_2    999

#define RPI_KEY_QUIT    SDLK_ESCAPE
//#define RPI_KEY_ACCEL   SDLK_BACKSPACE
#define RPI_KEY_QLOAD   SDLK_F1
#define RPI_KEY_QSAVE   SDLK_F2

//Default joy button configuration
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
#define START_1 6
#define SELECT_1 7
#define UP_1 8
#define DOWN_1 9
#define LEFT_1 10
#define RIGHT_1 11

#define A_2 20
#define B_2 21
#define X_2 22
#define Y_2 23
#define L_2 24
#define R_2 25
#define START_2 26
#define SELECT_2 27
#define UP_2 28
#define DOWN_2 29
#define LEFT_2 30
#define RIGHT_2 31

#define A_3 32
#define B_3 33
#define X_3 34
#define Y_3 35
#define L_3 36
#define R_3 37
#define START_3 38
#define SELECT_3 39
#define UP_3 40
#define DOWN_3 41
#define LEFT_3 42
#define RIGHT_3 43

#define A_4 44
#define B_4 45
#define X_4 46
#define Y_4 47
#define L_4 48
#define R_4 49
#define START_4 50
#define SELECT_4 51
#define UP_4 52
#define DOWN_4 53
#define LEFT_4 54
#define RIGHT_4 55

#define ACCEL 60
#define QUIT 61
#define HELP 62

#define QLOAD 63
#define QSAVE 64


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
unsigned long pi_joystick_read(int which1);
void pi_video_flip(void);
void * UpperMalloc(size_t size);
void UpperFree(void* mem);
int GetUpperSize(void* mem);
#ifdef __cplusplus
 } // End of extern "C"
#endif

#endif
