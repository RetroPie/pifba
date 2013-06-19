#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef struct
{
	int option_sound_enable;
	int option_rescale;
	int option_samplerate;
	int option_showfps;
	int option_display_border;
	char option_frontend[MAX_PATH];
} CFG_OPTIONS;

#endif
