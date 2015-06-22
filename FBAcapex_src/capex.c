/*  CAPEX for FBA2X

    Copyright (C) 2007  JyCet
	Copyright (C) 2008 Juanvvc. Adapted from capex for cps2emu by Jycet
	Copyright (C) 2013 Squid. Rewritten for Raspberry Pi.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <SDL.h>

#include <fcntl.h>
#include <sys/mman.h>

#include <glib.h>
#include <bcm_host.h>


//#include "./GFX/gfx_BG.h"
//#include "./GFX/gfx_CAPEX.h"
//#include "./GFX/gfx_FONT.h"
//#include "./GFX/gfx_SELECTEUR.h"

#include "capex.h"

#include "keyconstants.h"

SDL_Event event;

SDL_Surface *screen;
SDL_Surface *bg;
SDL_Surface *bgs;
SDL_Surface *bg_temp;
SDL_Surface *font;
SDL_Surface *bar;
SDL_Surface *preview;
SDL_Surface *title;
SDL_Surface *help;
SDL_Surface *credit;
//sq SDL_Surface *Tmp;

SDL_RWops *rw;

unsigned int font6x[255];
int i;
unsigned int ii;
unsigned char flag_preview;
char g_string[255];
char ar;
char * path;
char * argument[20];
unsigned char flag_save;
unsigned char offset_x , offset_y ;


unsigned char joy_buttons[2][32];
unsigned char joy_axes[4][2];

#define JOYLR 0
#define JOYUD 1

int joyaxis_LR[4], joyaxis_UD[4];

SDL_Joystick *joy[1];

unsigned char *sdl_keys;

static unsigned long fe_timer_read(void);
static void fe_ProcessEvents (void);
unsigned long pi_joystick_read(void);

static void dispmanx_init(void);
static void dispmanx_deinit(void);
static void dispmanx_display(void);
static void initSDL(void);

#define NUMKEYS 256
static Uint16 pi_key[NUMKEYS];
static Uint16 pi_joy[NUMKEYS];

char abspath[1000];

struct data
{
	unsigned int nb_rom;
	unsigned int nb_cache;
	unsigned int nb_list[NB_FILTRE];
	char *name[NB_MAX_GAMES];
	char *zip[NB_MAX_GAMES];
	char *status[NB_MAX_GAMES];
	char *parent[NB_MAX_GAMES];
	unsigned char state[NB_MAX_GAMES];
	unsigned int length[NB_MAX_GAMES];
	unsigned int long_max;
} data;

unsigned int listing_tri[NB_FILTRE][NB_MAX_GAMES];

struct options
{
	unsigned int y;
	unsigned int num;
	unsigned int offset_num;
	
	unsigned char sound;
	unsigned int samplerate;
	unsigned char rescale;
	unsigned char showfps;
	unsigned char linescroll;
	unsigned char frontend;
	unsigned char tweak;
	unsigned char hiscore;
	
	unsigned char nb;
	unsigned char listing;
	unsigned int display_border;
} options;

struct conf
{
	unsigned char exist;
	unsigned char sound;
	unsigned int samplerate;
	unsigned char rescale;
	unsigned char showfps;
	unsigned char tweak;
	unsigned char hiscore;
} conf;

struct capex
{
	unsigned char list;
	unsigned int sely;
	unsigned int selnum;
	unsigned int seloffset_num;
} capex;

struct selector
{
	unsigned int y;
	unsigned int crt_x;
	unsigned int num;
	unsigned int offset_num;
} selector;

//struct run
//{
//	unsigned char y;
//	signed char num;
//}run;

#include "capex_write.h"
#include "capex_read.h"
#include "capex_tri.h"
#include "capex_pixel.h"

void free_memory(void)
{
	SDL_FreeSurface(screen); screen=0;
	SDL_FreeSurface(bg); bg=0;
	SDL_FreeSurface(bgs); bgs=0;
	SDL_FreeSurface(bg_temp); bg_temp=0;
	SDL_FreeSurface(font); font=0;
	SDL_FreeSurface(bar); bar=0;
	SDL_FreeSurface(preview); preview=0;
	SDL_FreeSurface(title); title=0;
	SDL_FreeSurface(help); help=0;
	SDL_FreeSurface(credit); credit=0;
	for (ii=0;ii<data.nb_list[0];++ii){
		free(data.name[ii]);
		free(data.zip[ii]);
		free(data.status[ii]);
		free(data.parent[ii]);
		data.name[ii]=data.zip[ii]=data.status[ii]=data.parent[ii]=0;
	}
}

int exit_prog(void)
{
	//menage avant execution
	free_memory();

	dispmanx_deinit();

	SDL_JoystickClose(0);
	SDL_Quit();

	exit(0);

}

//Draw string from font bitmap to a surface, no clipping
void put_string(char *string, unsigned int pos_x, unsigned int pos_y, unsigned char colour, SDL_Surface *s)
{
	SDL_Rect Src;
	SDL_Rect Dest;

	Src.y = colour;
	Src.w = FONT_WIDTH;
	Src.h = FONT_HEIGHT;
	Dest.y = pos_y;

	while(*string)
	{
		if (font6x[*string]){
			Src.x = font6x[*string];
			Dest.x = pos_x;
			SDL_BlitSurface(font, &Src, s, &Dest);
		}
		++string;
		pos_x += FONT_WIDTH;
	}
}

//Draw string from font bitmap to screen, with clipping
void put_stringM(char *string, int pos_x, int pos_y, unsigned int size, unsigned int colour)
{
	SDL_Rect Src;
	SDL_Rect Dest;
	unsigned int caratere;
	
	if( size > selector.crt_x ){
		
		string += selector.crt_x ;
		
		Src.y = colour;
		Src.w = FONT_WIDTH;
		Src.h = FONT_HEIGHT;
		Dest.y = pos_y;
		
		if ( (size-selector.crt_x) > 80 ) {
			//Truncate the list to window size
			for( caratere=selector.crt_x ; caratere<(selector.crt_x+80) ; ++caratere)
			{
				if (font6x[*string]){
					Src.x = font6x[*string];
					Dest.x = pos_x;
					SDL_BlitSurface(font, &Src, screen, &Dest);
				}
				++string;
				pos_x += FONT_WIDTH;
			}
		}
		else 
		{
			while(*string)
			{
				if (font6x[*string]){
					Src.x = font6x[*string];
					Dest.x = pos_x;
					SDL_BlitSurface(font, &Src, screen, &Dest);
				}
				++string;
				pos_x += FONT_WIDTH;
			}
		}
	}
}

char ss_prg_credit(void)
{
	#define CREDIT_X	68
	#define CREDIT_Y	62
	#define CREDIT_L	184
	#define CREDIT_H	116
	
	unsigned int counter = 1;
	Uint32 PBLACK = SDL_MapRGB(credit->format, 0, 0, 0);	
	Uint32 PWHITE = SDL_MapRGB(credit->format, 255, 255, 255 );
	
	//capture screen actuel
	drawSprite( screen , credit , 0 , 0 , 0 , 0 , 640 , 480 );
	//detourage
	ligneV( credit , CREDIT_X-1 , CREDIT_Y , CREDIT_H , PBLACK );
	ligneV( credit , CREDIT_X+CREDIT_L , CREDIT_Y , CREDIT_H , PBLACK );
	ligneH( credit , CREDIT_X , CREDIT_Y-1 , CREDIT_L , PBLACK);
	ligneH( credit , CREDIT_X , CREDIT_Y+CREDIT_H , CREDIT_L , PBLACK);
	//cadre
	carre_plein( credit , CREDIT_X, CREDIT_Y, CREDIT_L, CREDIT_H, PWHITE);
	carre_plein( credit , CREDIT_X+1, CREDIT_Y+1, CREDIT_L-2, CREDIT_H-2, PBLACK);
	
	put_string( "CREDIT" , 142 , CREDIT_Y+8 , WHITE , credit );
	put_string( "CAPEX v0.7 by .... Juanvvc" , CREDIT_X+8 , CREDIT_Y+24 , BLUE , credit );
	put_string( "for FBA Emu by .... Juanvvc" , CREDIT_X+8 , CREDIT_Y+34 , BLUE , credit );
	put_string( "Code & design by ..... JyCet" , CREDIT_X+8 , CREDIT_Y+44 , GREEN , credit );
	put_string( "Preview pack by ... Juanvvc" , CREDIT_X+8 , CREDIT_Y+54 , GREEN , credit );
	put_string( "External skin by ... Pedator" , CREDIT_X+8 , CREDIT_Y+64 , GREEN , credit );
	//put_string( "Betatest by ... Yod4z and me" , CREDIT_X+8 , CREDIT_Y+84 , GREEN , credit );
	put_string( "Press (START) to quit CAPEX" , 79 , CREDIT_Y+100 , WHITE , credit );

	while(1)
	{
		drawSprite( credit , screen , 0 , 0 , 0 , 0 , 640 , 480 );
		//SDL_Flip(screen);
		dispmanx_display();
		
		SDL_PollEvent(&event);
		if (event.type==SDL_JOYBUTTONDOWN){
			if (counter==0 ) {
				if ( event.jbutton.button==GP2X_BUTTON_START ){
					return 1 ;
				}else return 0;
			}
			++counter;
		}else if (event.type==SDL_JOYBUTTONUP){
			counter=0;// reinitialisation joystick
		}
	}
}
void ss_prg_help(void)
{
	#define HELP_X	106
	#define HELP_Y	124
	#define HELP_L	428
	#define HELP_H	232
	
	unsigned int counter = 1;
	Uint32 PBLACK = SDL_MapRGB(help->format, 0, 0, 0);	
	Uint32 PWHITE = SDL_MapRGB(help->format, 255, 255, 255 );
	
	//capture screen actuel
	drawSprite( screen , help , 0 , 0 , 0 , 0 , 640 , 480 );
	//detourage
	ligneV( help , HELP_X-1 , HELP_Y , HELP_H , PBLACK );
	ligneV( help , HELP_X+HELP_L , HELP_Y , HELP_H , PBLACK );
	ligneH( help , HELP_X , HELP_Y-1 , HELP_L , PBLACK);
	ligneH( help , HELP_X , HELP_Y+HELP_H , HELP_L , PBLACK);
	//cadre
	carre_plein( help , HELP_X, HELP_Y, HELP_L, HELP_H, PWHITE);
	carre_plein( help , HELP_X+1, HELP_Y+1, HELP_L-2, HELP_H-2, PBLACK);
	
	put_string( "COLOR HELP" , 260 , HELP_Y+16 , WHITE , help );
	put_string( "RED      missing" , HELP_X+16 , HELP_Y+48 , RED , help );
	put_string( "ORANGE   clone rom detected" , HELP_X+16 , HELP_Y+68 , ORANGE , help );
	put_string( "YELLOW   parent or clone detected" , HELP_X+16 , HELP_Y+88 , YELLOW , help );
	put_string( "         & parent detected" , HELP_X+16 , HELP_Y+108 , YELLOW , help );
	put_string( "GREEN    clone & parent & cache" , HELP_X+16 , HELP_Y+128 , GREEN , help );
	put_string( "         detected" , HELP_X+16 , HELP_Y+148 , GREEN , help );
	put_string( "BLUE     parent & cache detected" , HELP_X+16 , HELP_Y+168 , BLUE , help );
	put_string( "Any button to return" , 200 , HELP_Y+200 , WHITE , help );

	int Hquit = 0;
	while(!Hquit)
	{
		drawSprite( help , screen , 0 , 0 , 0 , 0 , 640 , 480 );
		//sq SDL_Flip(screen);
		dispmanx_display();
		
		SDL_PollEvent(&event);
		if (event.type==SDL_JOYBUTTONDOWN){
			if (counter==0) Hquit = 1 ;
			++counter;
		} 
		else if (event.type==SDL_JOYBUTTONUP){
			counter=0;// reinitialisation joystick
		}
	}
}

void init_title(void)
{
	FILE *fp;

	//load background image
	if ((fp = fopen( "./skin/capex_bg.bmp" , "r")) != NULL){
		//sq Tmp = SDL_LoadBMP( "./skin/capex_bg.bmp" );
		bg = SDL_LoadBMP( "./skin/capex_bg.bmp" );
		fclose(fp);
	}
	
	//load selector image
	if ((fp = fopen( "./skin/capex_selector.bmp" , "r")) != NULL){
		//sq Tmp = SDL_LoadBMP( "./skin/capex_selector.bmp" );
		bar = SDL_LoadBMP( "./skin/capex_selector.bmp" );
		fclose(fp);
	}
	SDL_SetColorKey(bar ,SDL_SRCCOLORKEY,SDL_MapRGB(bar ->format,255,0,255));
	
	//load title/icon
	if ((fp = fopen( "./skin/capex_title.bmp" , "r")) != NULL){
		//sq Tmp = SDL_LoadBMP( "./skin/capex_title.bmp" );
		title = SDL_LoadBMP( "./skin/capex_title.bmp" );
		fclose(fp);
	}
	
	if ((fp = fopen( "./skin/gfx_FONT.bmp" , "r")) != NULL){
		font = SDL_LoadBMP( "./skin/gfx_FONT.bmp" );
		fclose(fp);
	}
	else 
	{
		printf("\n\nERROR: gfx_FONT.bmp file is missing, installed incorrectly!\n");
		exit_prog();
	}
	
	SDL_FreeRW (rw);
	//sq set transparent colour to black
	SDL_SetColorKey(font,SDL_SRCCOLORKEY,SDL_MapRGB(font->format,0,0,0));

	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 16, 0xf800, 0x07e0, 0x001f, 0x0000);
	bg_temp = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 16, 0xf800, 0x07e0, 0x001f, 0x0000);
	bgs = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 16, 0xf800, 0x07e0, 0x001f, 0x0000);
	help = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 16, 0xf800, 0x07e0, 0x001f, 0x0000);
	credit = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 16, 0xf800, 0x07e0, 0x001f, 0x0000);

	drawSprite( bg , bgs , 0 , 0 , 0 , 0 , 640 , 480 );
	
	prepare_window( bgs , bg , 248 , 6 , 384 , 224 );
	prepare_window( bgs , bg , 8 , 125, 232 , 105);
	drawSprite( title , bg, 0, 0, 7, 6, 234, 112);
	
}

void prep_bg_list(void)
{
	drawSprite( bg , bg_temp , 0 , 0 , 0 , 0 , 640 , 480 );
	prepare_window( bgs , bg_temp , 8 , 238 , 624 , 236 );
	load_preview(selector.num);
		
	put_string( "ROM" , 12 , 130 , WHITE , bg_temp);
	switch(capex.list)
	{
		case 0:
			sprintf((char*)g_string, "Database: %d" , data.nb_list[0] );
			break;
		case 1:
			sprintf((char*)g_string, "Missing: %d of %d" , data.nb_list[1], data.nb_list[0] );
			break;
		case 2:
			sprintf((char*)g_string, "Available: %d of %d" , data.nb_list[2], data.nb_list[0] );
			break;
		case 3:
			sprintf((char*)g_string, "Available: %d of %d" , data.nb_list[3], data.nb_list[0] );
			break;
	}
	put_string( g_string , 12 , 210 , WHITE , bg_temp );
}

void display_BG(void)
{
	FILE *fp2;

    drawSprite( bg_temp , screen , 0 , 0 , 0 , 0 , 640 , 480 );
		
    //.fba files prioritise over .rom files
    sprintf((char*)g_string, "roms/%s.fba" , data.zip[listing_tri[capex.list][selector.num]]);
    if ((fp2 = fopen(g_string, "r")) != NULL){
        sprintf((char*)g_string, "%s.fba" , data.zip[listing_tri[capex.list][selector.num]]);
        fclose(fp2);
    }
	else {
        sprintf((char*)g_string, "%s.zip" , data.zip[listing_tri[capex.list][selector.num]]);
    }
    
    put_string( g_string , 64 , 130 , WHITE , screen );
    
    if ( strcmp( data.parent[listing_tri[capex.list][selector.num]] , "fba" ) == 0 ){
        put_string( "Parent rom" , 12 , 150 , WHITE , screen );
    }else{
        sprintf((char*)g_string, "Clone of %s" , data.parent[listing_tri[capex.list][selector.num]]);
        put_string( g_string , 12 , 150 , WHITE , screen );
    }		
    
    if ( data.status[listing_tri[capex.list][selector.num]] != NULL ){
        put_string( data.status[listing_tri[capex.list][selector.num]] , 12 , 190 , WHITE , screen );
    }

}

void display_line_options(unsigned char num, unsigned int y)
{
	#define OPTIONS_START_X	16
	#define CONF_START_X	544

	switch (num)
	{
//		case OPTION_NUM_FBA2X_SOUND:
//			if (options.sound) put_string( "Sound: Enable" , OPTIONS_START_X , y , WHITE , screen );
//			else put_string( "Sound: Disable" , OPTIONS_START_X , y , WHITE , screen );
//			if (conf.exist){
//				put_string( abreviation_cf[0][conf.sound] , CONF_START_X , y , GREEN , screen );
//			}else{
//				put_string( "-" , CONF_START_X , y , RED , screen );
//			}
//			break;
//		case OPTION_NUM_FBA2X_RESCALE:
//			/*if (options.rescale == 3) put_string( "Window: Hardware Horizontal Resizing" , OPTIONS_START_X , y , WHITE , screen );
//			else */
//			if (options.rescale == 2) put_string( "Window: Hardware Full Resising" , OPTIONS_START_X , y , WHITE , screen );
//			else if (options.rescale == 1) put_string( "Window: Software" , OPTIONS_START_X , y , WHITE , screen );
//			else put_string( "Window: No" , OPTIONS_START_X , y , WHITE , screen );
//			if (conf.exist){
//				put_string( abreviation_cf[1][conf.rescale] , CONF_START_X , y , GREEN , screen );
//			}else{
//				put_string( "-" , CONF_START_X , y , RED , screen );
//			}
//			break;
		case OPTION_NUM_FBA2X_SHOWFPS:
			if (options.showfps) 
				put_string( "Show FPS: Enable" , OPTIONS_START_X , y , WHITE , screen );
			else 
				put_string( "Show FPS: Disable" , OPTIONS_START_X , y , WHITE , screen );
			break;
		case OPTION_NUM_CAPEX_LIST:
			if (capex.list == 3) 
				put_string( "Listing: Available with no clones" , OPTIONS_START_X , y , WHITE , screen );
			else 
				if (capex.list == 2) put_string( "Listing: Available only" , OPTIONS_START_X , y , WHITE , screen );
			else 
				if (capex.list == 1) put_string( "Listing: Missing only" , OPTIONS_START_X , y , WHITE , screen );
			else 
				put_string( "Listing view: All" , OPTIONS_START_X , y , WHITE , screen );
			break;
		case OPTION_NUM_RETURN:
			put_string( "Return to the game list" , OPTIONS_START_X , y , WHITE , screen );
			break;
	}
}

void ss_prg_options(void)
{
	int Quit;
	unsigned int y;
	unsigned int option_start;

	Uint32 Joypads;
	unsigned long keytimer=0;
	int keydirection=0, last_keydirection=0;

	flag_save = GREEN;
	options.y = START_Y;
	options.num = 0;
	options.offset_num = 0;
	
	prep_bg_list();
		
	Quit=0;
	while(!Quit)
	{
        
		display_BG();
		
		drawSprite(bar , screen, 0, 0, 8, options.y-1, 624,  LIST_LINE_HEIGHT+2);
		
		option_start = START_Y;
		for ( y = options.offset_num ; y<(options.offset_num+7) ; ++y){
			display_line_options( y , option_start );
			option_start += LIST_LINE_HEIGHT;
		}

		//SDL_Flip(screen);
		dispmanx_display();
        
        usleep(70000);
        
        while(1)
        {
            usleep(10000);
            
            fe_ProcessEvents();
            Joypads = pi_joystick_read();
            
            last_keydirection=keydirection;
            keydirection=0;
            
            //Any keyboard key pressed?
            if(Joypads & GP2X_LEFT || Joypads & GP2X_RIGHT ||
               Joypads & GP2X_UP || Joypads & GP2X_DOWN || Joypads & GP2X_A)
            {
                keydirection=1;
                break;
            }
            
            //Game selected
            //if(Joypads & GP2X_START || Joypads & GP2X_B) break;
			if(Joypads) break;
            
            //Used to delay the initial key press, but
            //once pressed and held the delay will clear
            keytimer = fe_timer_read() + (1000000/2);
            
        }
        
        //Key delay
        if(keydirection && last_keydirection && fe_timer_read() < keytimer) continue;
        
        int updown=0;
        
        if(Joypads & GP2X_DOWN) {
            if ( options.num == NOMBRE_OPTIONS ){
                options.y = START_Y;
                options.num = 0;
                options.offset_num = 0;
            }
            else {
                if (options.num < NOMBRE_OPTIONS || options.offset_num == ( NOMBRE_OPTIONS - 5 ) ){
                    if ( options.num < NOMBRE_OPTIONS ){
                        options.y += LIST_LINE_HEIGHT;
                        ++options.num;
                    }
                }
                else  {
                    ++options.offset_num;
                    ++options.num;
                }
            }
        }

        if(Joypads & GP2X_UP) {
            if ( options.num == 0 ){
                options.y = START_Y + (NOMBRE_OPTIONS*LIST_LINE_HEIGHT) ;
                options.num = NOMBRE_OPTIONS;
                options.offset_num = 0;
            }else{
                if ( options.num > ( NOMBRE_OPTIONS - 6 ) || options.offset_num == 0 ){
                    if (options.num>0){
                        options.y -= LIST_LINE_HEIGHT;
                        --options.num;
                    }
                }else{
                    --options.offset_num;
                    --options.num;
                }
            }
        }
        
		if (Joypads & GP2X_SELECT && Joypads & GP2X_START) {
			exit_prog();
		}

        if(Joypads & GP2X_A || Joypads & GP2X_START) {
            switch(options.num){
//                case OPTION_NUM_FBA2X_SOUND:
//                    flag_save = RED;
//                    ++options.sound;
//                    options.sound &= 1;
//                    break;
//                case OPTION_NUM_FBA2X_RESCALE:
//                    flag_save = RED;
//                    ++options.rescale;
//                    if ( options.rescale == 3 ) options.rescale = 0;
//                    break;
                case OPTION_NUM_FBA2X_SHOWFPS:
                    flag_save = RED;
                    ++options.showfps;
                    options.showfps &= 1;
                    break;
                case OPTION_NUM_CAPEX_LIST:
                    flag_save = RED;
                    ++capex.list;
                    if ( capex.list == NB_FILTRE ) capex.list = 0;
                    selector.y = START_Y;
                    selector.crt_x=0;
                    selector.num = 0;
                    selector.offset_num = 0;
                    break;
//                case OPTION_NUM_SAVE:
//                   write_cfg();
//                    flag_save = GREEN;
//                    break;
//                case OPTION_NUM_SAVE_CF:
//                    write_cf( );
//                    flag_save = GREEN;
//                    load_cf();
//                    load_cfg();
//                    break;
                case OPTION_NUM_RETURN:
                    write_cfg();
                    prep_bg_list();
                    Quit = 1;
                    break;
            }
        }


	}
}

void ss_prog_run(void)
{
	char arglist[1000];
	FILE *fp2;

	strcpy(arglist, "./fba2x ");
	
    //.fba files prioritise over .rom files
    sprintf((char*)g_string, "roms/%s.fba" , data.zip[listing_tri[capex.list][selector.num]]);
    if ((fp2 = fopen(g_string, "r")) != NULL)
		fclose(fp2);
	else 
        sprintf((char*)g_string, "roms/%s.zip" , data.zip[listing_tri[capex.list][selector.num]]);

	strcat(arglist, g_string);
	strcat(arglist, " ");

    if (options.showfps){
		strcat(arglist, "--showfps");
		strcat(arglist, " ");
    }

    free_memory();
 	dispmanx_deinit();	
	SDL_JoystickClose(0);
    SDL_Quit();

	//Run FBA and wait
	system(arglist);

}

static GKeyFile *gkeyfile=0;

static void open_config_file(void)
{
    GError *error = NULL;

    gkeyfile = g_key_file_new ();
    if (!(int)g_key_file_load_from_file (gkeyfile, "fba2x.cfg", G_KEY_FILE_NONE, &error))
    {
        gkeyfile=0;
    }
}

static void close_config_file(void)
{
    if(gkeyfile)
        g_key_file_free(gkeyfile);
}

static int get_integer_conf (const char *section, const char *option, int defval)
{
    GError *error=NULL;
    int tempint;

    if(!gkeyfile) return defval;

    tempint = g_key_file_get_integer(gkeyfile, section, option, &error);
    if (!error)
        return tempint;
    else
        return defval;
}

void pi_initialize_input()
{
    memset(joy_buttons, 0, 32*2);
    memset(joy_axes, 0, 8*2);
    memset(pi_key, 0, NUMKEYS*2);
    memset(pi_joy, 0, NUMKEYS*2);

    //Open config file for reading below
    open_config_file();

    //Configure keys from config file or defaults
    pi_key[A_1] = get_integer_conf("Keyboard", "A_1", RPI_KEY_A);
    pi_key[B_1] = get_integer_conf("Keyboard", "B_1", RPI_KEY_B);
    pi_key[X_1] = get_integer_conf("Keyboard", "X_1", RPI_KEY_X);
    pi_key[Y_1] = get_integer_conf("Keyboard", "Y_1", RPI_KEY_Y);
    pi_key[L_1] = get_integer_conf("Keyboard", "L_1", RPI_KEY_L);
    pi_key[R_1] = get_integer_conf("Keyboard", "R_1", RPI_KEY_R);
    pi_key[START_1] = get_integer_conf("Keyboard", "START_1", RPI_KEY_START);
    pi_key[SELECT_1] = get_integer_conf("Keyboard", "SELECT_1", RPI_KEY_SELECT);
    pi_key[LEFT_1] = get_integer_conf("Keyboard", "LEFT_1", RPI_KEY_LEFT);
    pi_key[RIGHT_1] = get_integer_conf("Keyboard", "RIGHT_1", RPI_KEY_RIGHT);
    pi_key[UP_1] = get_integer_conf("Keyboard", "UP_1", RPI_KEY_UP);
    pi_key[DOWN_1] = get_integer_conf("Keyboard", "DOWN_1", RPI_KEY_DOWN);

    pi_key[QUIT] = get_integer_conf("Keyboard", "QUIT", RPI_KEY_QUIT);
    pi_key[ACCEL] = get_integer_conf("Keyboard", "ACCEL", RPI_KEY_ACCEL);

    //Configure joysticks from config file or defaults
    pi_joy[A_1] = get_integer_conf("Joystick", "A_1", RPI_JOY_A);
    pi_joy[B_1] = get_integer_conf("Joystick", "B_1", RPI_JOY_B);
    pi_joy[X_1] = get_integer_conf("Joystick", "X_1", RPI_JOY_X);
    pi_joy[Y_1] = get_integer_conf("Joystick", "Y_1", RPI_JOY_Y);
    pi_joy[L_1] = get_integer_conf("Joystick", "L_1", RPI_JOY_L);
    pi_joy[R_1] = get_integer_conf("Joystick", "R_1", RPI_JOY_R);
    pi_joy[START_1] = get_integer_conf("Joystick", "START_1", RPI_JOY_START);
    pi_joy[SELECT_1] = get_integer_conf("Joystick", "SELECT_1", RPI_JOY_SELECT);

    pi_joy[QUIT] = get_integer_conf("Joystick", "QUIT", RPI_JOY_QUIT);
    pi_joy[ACCEL] = get_integer_conf("Joystick", "ACCEL", RPI_JOY_ACCEL);

    pi_joy[QLOAD] = get_integer_conf("Joystick", "QLOAD", RPI_JOY_QLOAD);
    pi_joy[QSAVE] = get_integer_conf("Joystick", "QSAVE", RPI_JOY_QSAVE);

	//Read joystick axis to use, default to 0 & 1
	joyaxis_LR[0] = get_integer_conf("Joystick", "JA_LR", 0);
	joyaxis_UD[0] = get_integer_conf("Joystick", "JA_UD", 1);

	options.display_border = get_integer_conf("Graphics", "DisplayBorder", 0);

    close_config_file();

}


int main(int argc, char *argv[])
{
//    sleep(12);
    
	int Quit,ErrorQuit ;
	//int Write = 0;
	unsigned int zipnum;
	unsigned int y;

    Uint32 Joypads;
    unsigned long keytimer=0;
    int keydirection=0, last_keydirection=0;
        
    //Set the cwd directory to where the binary is
    realpath(argv[0], abspath);
    char *dirsep = strrchr(abspath, '/');
    if( dirsep != 0 ) *dirsep = 0;
    chdir(abspath);

	while(1)
	{
	
		pi_initialize_input();

		initSDL();
		dispmanx_init();
	
		load_cfg();
		init_title();
		
		if (read_zipname()) ErrorQuit = 1;
		else{
			tri_alphabeticAZ(0,data.nb_list[0]);
			if ( read_rominfo() ) ErrorQuit = 2;
			else{
				read_playable_roms();
				ErrorQuit = 0;
			}
		}
	
		data.nb_list[1] = 0;
		data.nb_list[2] = 0;
		data.nb_list[3] = 0;
		for ( ii=0 ; ii<data.nb_list[0] ; ++ii){
			if (data.state[listing_tri[0][ii]] == RED ) {
				listing_tri[1][data.nb_list[1]] = listing_tri[0][ii];
				++data.nb_list[1];
			}else{
				listing_tri[2][data.nb_list[2]] = listing_tri[0][ii];
				++data.nb_list[2];
				
				//Load list without any clones
				if (strcmp( data.parent[listing_tri[0][ii]] , "fba" ) == 0 ) {
					listing_tri[3][data.nb_list[3]] = listing_tri[0][ii];
					++data.nb_list[3];
				}
			}
		}
		
		//precalc font6 x coordinates
		for (i=0;i<32;++i) font6x[i]=0;
		for (i=32;i<255;++i) font6x[i]=(i-32)*FONT_WIDTH;
			
		//Load initial game row position from saved config values
	    selector.y = capex.sely;
	    selector.num = capex.selnum;
	    selector.offset_num = capex.seloffset_num;
	    
	    selector.crt_x=0;
	    
		flag_preview = 0;
		load_preview(selector.num);
		
		prep_bg_list();
		
		Quit = 0;
		while (!Quit)
		{
			display_BG();
			
			drawSprite(bar , screen, 0, 0, 8, selector.y-1, 624, LIST_LINE_HEIGHT+2);
			
			if (ErrorQuit == 1) 
				put_string("Missing file zipname.fba" , 16 , 440 , 0 , screen );
			else 
				if (ErrorQuit == 2) 
					put_string("Missing file rominfo.fba" , 16 , 440 , 0 , screen );
			else 
			{
				zipnum = START_Y;
				if ( data.nb_list[capex.list] <= LIST_MAX_ROWS){
					for ( y = 0 ; y<data.nb_list[capex.list] ; ++y){
						put_stringM(data.name[listing_tri[capex.list][y]] , 16 , zipnum, data.length[listing_tri[capex.list][y]] , data.state[listing_tri[capex.list][y]] );
						zipnum += LIST_LINE_HEIGHT;
					}
				}else{
					for ( y = selector.offset_num ; y<(selector.offset_num+LIST_MAX_ROWS) ; ++y){
						put_stringM(data.name[listing_tri[capex.list][y]] , 16 , zipnum, data.length[listing_tri[capex.list][y]] , data.state[listing_tri[capex.list][y]] );
						zipnum += LIST_LINE_HEIGHT;
					}
				}
			}

{
//SDL_SaveBMP(screen, "scrshot.bmp");
//exit_prog();
}
			
			//SDL_Flip(screen);
			dispmanx_display();
	        
	        usleep(70000);
	   
	        while(1)
	        {
	            usleep(10000);
	            
	            fe_ProcessEvents();
	            Joypads = pi_joystick_read();
	            
	            last_keydirection=keydirection;
	            keydirection=0;
	            
	            //Any keyboard key pressed?
	            if(Joypads & GP2X_LEFT || Joypads & GP2X_RIGHT ||
	               Joypads & GP2X_UP || Joypads & GP2X_DOWN)
	            {
	                keydirection=1;
	                break;
	            }
	            
	            //Game selected
	            //if(Joypads & GP2X_START || Joypads & GP2X_B) break;
				if(Joypads) break;
	            
	            //Used to delay the initial key press, but
	            //once pressed and held the delay will clear
	            keytimer = fe_timer_read() + (1000000/2);
	            
	        }
	        
	        //Key delay
	        if(keydirection && last_keydirection && fe_timer_read() < keytimer) continue;
	        
	        int updown=0;
	        
	       	if(Joypads & GP2X_DOWN) {
				updown=1;
				if ( selector.num == (data.nb_list[capex.list]-1)) { //was bottom of list so move to top
					selector.y = START_Y;
					selector.num = 0;
					selector.offset_num = 0;
				}
				else {
					if (data.nb_list[capex.list] < LIST_MAX_ROWS+1) { 
						if (selector.num < (data.nb_list[capex.list]-1) ) {
							selector.y+=LIST_LINE_HEIGHT;
							++selector.num;
						}
					}
	                else {
						if (selector.num<7 || selector.offset_num==(data.nb_list[capex.list]-LIST_MAX_ROWS)) {
							if (selector.num < (data.nb_list[capex.list]-1)){
								selector.y+=LIST_LINE_HEIGHT;
								++selector.num;
							}
						}
	               		else {
							++selector.offset_num;
							++selector.num;
						}
					}
				}
	            load_preview(selector.num);
			}
	        
			if(Joypads & GP2X_UP) {
				updown=1;
				if ( selector.num==0 ){
					selector.num = data.nb_list[capex.list] - 1 ;
					if (data.nb_list[capex.list] < LIST_MAX_ROWS+1){
						selector.y = START_Y + ( ( data.nb_list[capex.list] - 1 ) * LIST_LINE_HEIGHT );
						//selector.offset_num = 0;
					}
	                else {
						//sq selector.y = START_Y -1 + (24*LIST_LINE_HEIGHT) ;
						selector.y = START_Y + ((LIST_MAX_ROWS-1)*LIST_LINE_HEIGHT) ;
						selector.offset_num = data.nb_list[capex.list] - LIST_MAX_ROWS;
					}
				}
	            else {
					if ( selector.num > (data.nb_list[capex.list]-7) || selector.offset_num==0) {
						if (selector.num>0){
							selector.y-=LIST_LINE_HEIGHT;
							--selector.num;
						}
					}
	                else {
						--selector.offset_num;
						--selector.num;
					}
				}
	            load_preview(selector.num);
	            //load_cf();
			}
	
	        if (Joypads & GP2X_RIGHT && !updown) {
	            if ( selector.num < (data.nb_list[capex.list]-(LIST_MAX_ROWS+1))) {
	                selector.y = START_Y;
	                selector.num += LIST_MAX_ROWS;
	                selector.offset_num = selector.num;
	                if(selector.offset_num > data.nb_list[capex.list]-(LIST_MAX_ROWS+1)) 
						selector.offset_num=data.nb_list[capex.list]-(LIST_MAX_ROWS+1);
	            } else {
	                selector.y = START_Y;
	                selector.num = data.nb_list[capex.list]-LIST_MAX_ROWS;
	                selector.offset_num = selector.num;
	            }
	            load_preview(selector.num);
	        }
	        
	        if (Joypads & GP2X_LEFT && !updown) {
	            if ( selector.num >= LIST_MAX_ROWS) {
	                selector.y = START_Y;
	                selector.num -= LIST_MAX_ROWS;
	                selector.offset_num = selector.num;
	            } else {
	                selector.y = START_Y;
	                selector.offset_num = 0;
	                selector.num = 0;
	            }
	            load_preview(selector.num);
	        }
	
			if ((Joypads & GP2X_SELECT && Joypads & GP2X_START) || Joypads & GP2X_ESCAPE) {
	            capex.sely = selector.y;
	            capex.selnum = selector.num;
	            capex.seloffset_num = selector.offset_num;
	            write_cfg();    //Write the current selection to the config
				exit_prog();
	            Quit=1;
	        }
	                
	        if ( Joypads & GP2X_A || Joypads & GP2X_START ){
				if ( data.state[listing_tri[capex.list][selector.num]] > RED ){
	                capex.sely = selector.y;
	                capex.selnum = selector.num;
	                capex.seloffset_num = selector.offset_num;
	                write_cfg();    //Write the current selection to the config
					Quit=1;
					ss_prog_run();
				}
			}
	                
	        if( Joypads & GP2X_X ) ss_prg_help(); 
	                
	        if( Joypads & GP2X_SELECT ) {
	            capex.sely = selector.y;
	            capex.selnum = selector.num;
	            capex.seloffset_num = selector.offset_num;
	            write_cfg();    //Write the current selection to the config
				ss_prg_options();
			}
	
		}
	
	}

	return 0;
}

static unsigned long fe_timer_read(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    
    return ((unsigned long long)now.tv_sec * 1000000LL + (now.tv_nsec / 1000LL));
}

static void fe_ProcessEvents (void)
{
    int num=0, i;
    int hatmovement=0;

    //Process four joysticks
    for(i=0;i<1;i++) {
        joy_axes[i][JOYLR] = CENTER;
        joy_axes[i][JOYUD] = CENTER;

        if(SDL_JoystickNumHats(joy[i]) > 0) {
            Uint8 hat = SDL_JoystickGetHat(joy[i], 0);
            if (hat != 0) {
                hatmovement=1;
                if(hat & SDL_HAT_UP) { joy_axes[i][JOYUD] = UP; }
                if(hat & SDL_HAT_DOWN) { joy_axes[i][JOYUD] = DOWN; }
                if(hat & SDL_HAT_LEFT) { joy_axes[i][JOYLR] = LEFT; }
                if(hat & SDL_HAT_RIGHT) { joy_axes[i][JOYLR] = RIGHT; }
            }
        }

        int axis = SDL_JoystickNumAxes(joy[i]);

        //HAT movement overrides analogue as analogue always shows a value
        if(axis > 0 && !hatmovement) {
            Sint16 x_move = SDL_JoystickGetAxis(joy[i], joyaxis_LR[i]);
            Sint16 y_move = SDL_JoystickGetAxis(joy[i], joyaxis_UD[i]);

            if(x_move != 0) {
                if(x_move > -10000 && x_move < 10000)
                    joy_axes[i][JOYLR] = CENTER;
                else if(x_move > 10000)
                    joy_axes[i][JOYLR] = RIGHT;
                else
                    joy_axes[i][JOYLR] = LEFT;
            }

            if(y_move != 0) {
                if(y_move > -10000 && y_move < 10000)
                    joy_axes[i][JOYUD] = CENTER;
                else if(y_move > 10000)
                    joy_axes[i][JOYUD] = DOWN;
                else
                    joy_axes[i][JOYUD] = UP;
            }
        }

    }


    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_JOYBUTTONDOWN:
                joy_buttons[event.jbutton.which][event.jbutton.button] = 1;
                break;

            case SDL_JOYBUTTONUP:
                joy_buttons[event.jbutton.which][event.jbutton.button] = 0;
                break;

            case SDL_KEYDOWN:
                sdl_keys = SDL_GetKeyState(NULL);
                break;
            case SDL_KEYUP:
                sdl_keys = SDL_GetKeyState(NULL);
                break;
        }
    }

}

unsigned long pi_joystick_read(void)
{
    unsigned long val=0;

    if (joy_buttons[0][pi_joy[L_1]])       val |= GP2X_L;
    if (joy_buttons[0][pi_joy[R_1]])       val |= GP2X_R;
    if (joy_buttons[0][pi_joy[X_1]])       val |= GP2X_X;
    if (joy_buttons[0][pi_joy[Y_1]])       val |= GP2X_Y;
    if (joy_buttons[0][pi_joy[B_1]])       val |= GP2X_B;
    if (joy_buttons[0][pi_joy[A_1]])       val |= GP2X_A;
    if (joy_buttons[0][pi_joy[START_1]])   val |= GP2X_START;
    if (joy_buttons[0][pi_joy[SELECT_1]])  val |= GP2X_SELECT;
	if (joy_axes[0][JOYUD] == UP)         val |= GP2X_UP;
	if (joy_axes[0][JOYUD] == DOWN)       val |= GP2X_DOWN;
	if (joy_axes[0][JOYLR] == LEFT)       val |= GP2X_LEFT;
	if (joy_axes[0][JOYLR] == RIGHT)      val |= GP2X_RIGHT;


    if(sdl_keys)
    {
        if (sdl_keys[pi_key[L_1]] == SDL_PRESSED)       val |= GP2X_L;
        if (sdl_keys[pi_key[R_1]] == SDL_PRESSED)       val |= GP2X_R;
        if (sdl_keys[pi_key[X_1]] == SDL_PRESSED)       val |= GP2X_X;
        if (sdl_keys[pi_key[Y_1]] == SDL_PRESSED)       val |= GP2X_Y;
        if (sdl_keys[pi_key[B_1]] == SDL_PRESSED)       val |= GP2X_B;
        if (sdl_keys[pi_key[A_1]] == SDL_PRESSED)       val |= GP2X_A;
        if (sdl_keys[pi_key[START_1]] == SDL_PRESSED)   val |= GP2X_START;
        if (sdl_keys[pi_key[SELECT_1]] == SDL_PRESSED)  val |= GP2X_SELECT;
        if (sdl_keys[pi_key[UP_1]] == SDL_PRESSED)      val |= GP2X_UP;
        if (sdl_keys[pi_key[DOWN_1]] == SDL_PRESSED)    val |= GP2X_DOWN;
        if (sdl_keys[pi_key[LEFT_1]] == SDL_PRESSED)    val |= GP2X_LEFT;
        if (sdl_keys[pi_key[RIGHT_1]] == SDL_PRESSED)   val |= GP2X_RIGHT;
        if (sdl_keys[pi_key[QUIT]] == SDL_PRESSED)      val |= GP2X_ESCAPE;
    }

    return(val);
}


DISPMANX_RESOURCE_HANDLE_T fe_resource;
DISPMANX_ELEMENT_HANDLE_T fe_element;
DISPMANX_DISPLAY_HANDLE_T fe_display;
DISPMANX_UPDATE_HANDLE_T fe_update;

static void initSDL(void)
{
	//Initialise everything SDL
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0 )
    {
        printf("Could not initialize SDL(%s)\n", SDL_GetError());
        exit(1);
    }

    screen = SDL_SetVideoMode(0, 0, 16, SDL_SWSURFACE);
	joy[0] = SDL_JoystickOpen(0);

    SDL_EventState(SDL_ACTIVEEVENT,SDL_IGNORE);
    SDL_EventState(SDL_SYSWMEVENT,SDL_IGNORE);
    SDL_EventState(SDL_VIDEORESIZE,SDL_IGNORE);
    SDL_EventState(SDL_USEREVENT,SDL_IGNORE);
    SDL_ShowCursor(SDL_DISABLE);
}

static void dispmanx_init(void)
{
    int ret;
    uint32_t crap;
    uint32_t display_width=0, display_height=0;

    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    bcm_host_init();

    graphics_get_display_size(0 /* LCD */, &display_width, &display_height);

    fe_display = vc_dispmanx_display_open( 0 );

    fe_resource = vc_dispmanx_resource_create(VC_IMAGE_RGB565, 640, 480, &crap);

    //sq vc_dispmanx_rect_set( &dst_rect, 0, 0, display_width, display_height);
    vc_dispmanx_rect_set( &dst_rect, options.display_border, options.display_border, 
						display_width-(options.display_border*2), display_height-(options.display_border*2));
    vc_dispmanx_rect_set( &src_rect, 0, 0, 640 << 16, 480 << 16);

    //Make sure mame and background overlay the menu program
    fe_update = vc_dispmanx_update_start( 0 );

    // create the 'window' element - based on the first buffer resource (resource0)
    fe_element = vc_dispmanx_element_add(  fe_update,
           fe_display, 1, &dst_rect, fe_resource, &src_rect,
           DISPMANX_PROTECTION_NONE, 0, 0, (DISPMANX_TRANSFORM_T) 0 );

    ret = vc_dispmanx_update_submit_sync( fe_update );

}

static void dispmanx_deinit(void)
{
    int ret;

    fe_update = vc_dispmanx_update_start( 0 );
    ret = vc_dispmanx_element_remove( fe_update, fe_element );
    ret = vc_dispmanx_update_submit_sync( fe_update );
    ret = vc_dispmanx_resource_delete( fe_resource );
    ret = vc_dispmanx_display_close( fe_display );

	bcm_host_deinit();
}

static void dispmanx_display(void)
{
    VC_RECT_T dst_rect;

    vc_dispmanx_rect_set( &dst_rect, 0, 0, 640, 480 );

    // begin display update
    fe_update = vc_dispmanx_update_start( 0 );

    // blit image to the current resource
	SDL_LockSurface(screen);
    vc_dispmanx_resource_write_data( fe_resource, VC_IMAGE_RGB565, screen->pitch, screen->pixels, &dst_rect );
	SDL_UnlockSurface(screen);

    vc_dispmanx_update_submit_sync( fe_update );
}
