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
	int display_smooth_stretch;
	int display_effect;
	int maintain_aspect_ratio;
	int display_rotate;
} CFG_OPTIONS;

#endif
