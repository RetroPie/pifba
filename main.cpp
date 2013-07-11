/*
 * NES for MOTO EZX Modile Phone
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
 * SPECIAL THANKS:
 *   Sam Revitch  	http://lsb.blogdns.net/ezx-devkit 
 *
 * $Id: main.cpp,v 0.10 2006/06/07 $
 *
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include "main.h"
#include "fba_player.h"
#include "gp2xmemfuncs.h"
#include "burner.h"
#include "snd.h"
#include "config.h"

extern "C"
{
#include "gp2xsdk.h"
};

CFG_OPTIONS config_options;

unsigned short *fb;

int FindDrvByFileName(const char * fn)
{
	char sfn[60] = {0, };
	for (int i=strlen(fn)-1; i>=0; i-- ) {
		if (fn[i] == '/' || fn[i] == '\\' ) {
			strcpy( sfn, fn + i + 1 );
			break;
		}
	}
	if (sfn[0] == 0 ) strcpy( sfn, fn );
	char * p = strrchr( sfn, '.' );
	if (p) *p = 0;
	
	for (nBurnDrvSelect=0; nBurnDrvSelect<nBurnDrvCount; nBurnDrvSelect++)
		if ( strcasecmp(sfn, BurnDrvGetText(DRV_NAME)) == 0 )
			return nBurnDrvSelect;
	nBurnDrvSelect = 0;
	return -1;
}


void parse_cmd(int argc, char *argv[], char *path)
{
	int option_index, c;
	int val;
	char *p;

	static struct option long_opts[] = {
		{"sound", 0, &config_options.option_sound_enable, 1},
		{"no-sound", 0, &config_options.option_sound_enable, 0},
		{"samplerate", required_argument, 0, 'r'},
		{"no-rescale", 0, &config_options.option_rescale, 0},
		{"sw-rescale", 0, &config_options.option_rescale, 1},
		{"hw-rescale", 0, &config_options.option_rescale, 2},
		{"showfps", 0, &config_options.option_showfps, 1},
		{"no-showfps", 0, &config_options.option_showfps, 0},
		{"frontend", required_argument, 0, 'f'}
	};

	option_index=optind=0;

	while((c=getopt_long(argc, argv, "", long_opts, &option_index))!=EOF) {
		switch(c) {
			case 'r':
				if(!optarg) continue;
				if(strcmp(optarg, "11025") == 0) config_options.option_samplerate = 0;
				if(strcmp(optarg, "22050") == 0) config_options.option_samplerate = 1;
				if(strcmp(optarg, "44100") == 0) config_options.option_samplerate = 2;
				break;
			case 'f':
				if(!optarg) continue;
				p = strrchr(optarg, '/');
				if(p == NULL)
					sprintf(config_options.option_frontend, "%s%s", "./", optarg);
				else
					strcpy(config_options.option_frontend, optarg);
				break;
		}
	}

	if(optind < argc) {
		strcpy(path, argv[optind]);
	}
}

/*
 * application main() 
 */

int main( int argc, char **argv )
{ 	
	char path[MAX_PATH];

//sq	gp2x_initialize();

	if (argc < 2)
	{
		int c;
		printf ("Usage: %s <path to rom><shortname>.zip\n   ie: %s ./uopoko.zip\n Note: Path and .zip extension are mandatory.\n\n",argv[0], argv[0]);
		printf ("Supported (but not necessarily working via fba-gp2x) roms:\n\n");
		BurnLibInit();
		for (nBurnDrvSelect=0; nBurnDrvSelect<nBurnDrvCount; nBurnDrvSelect++)
		{
			printf ("%-20s ", BurnDrvGetTextA(DRV_NAME)); c++;
			if (c == 3)
			{
				c = 0;
				printf ("\n");
			}
		}
		printf ("\n\n");
		return 0;
	}

	//Initialize configuration options
	config_options.option_sound_enable = 1;
//sq config_options.option_sound_enable = 0;
	config_options.option_rescale = 2;
	config_options.option_samplerate = 2;
	config_options.option_showfps = 0;
	config_options.option_display_border = 30;
	strcpy(config_options.option_frontend, "");
	parse_cmd(argc, argv,path);

	pi_initialize();

	//Initialize sound thread
	run_fba_emulator (path);
	pi_terminate(config_options.option_frontend);
}
