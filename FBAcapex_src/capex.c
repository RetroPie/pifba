/*  CAPEX for FBA2X

    Copyright (C) 2007  JyCet
	Copyright (C) 2008 Juanvvc. Adapted from capex for cps2emu by Jycet

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

// CAPEX
// FBA2X Frontend
// JYCET & Juanvvc 2K8
// v0.1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <SDL.h>

#include <fcntl.h>
#include <sys/mman.h>

#include <glib.h>


#include "./GFX/gfx_BG.h"
#include "./GFX/gfx_CAPEX.h"
#include "./GFX/gfx_FONT.h"
#include "./GFX/gfx_SELECTEUR.h"

#include "capex.h"
#ifdef GP2X
#include "cpuclock.h"
#endif

#include "keyconstants.h"

SDL_Event event;

SDL_Surface *screen;
SDL_Surface *bg;
SDL_Surface *bgs;
SDL_Surface *bg_temp;
SDL_Surface *font;
SDL_Surface *barre;
SDL_Surface *preview;
SDL_Surface *title;
SDL_Surface *help;
SDL_Surface *credit;
SDL_Surface *Tmp;

SDL_RWops *rw;

unsigned int font6x[255];
int i;
unsigned int ii;
unsigned char flag_preview;
char g_string[255];
char ar;
char * path;
char * argument[20];
//unsigned char title_y;
unsigned char flag_save;
unsigned char offset_x , offset_y ;


unsigned char joy_buttons[2][32];
unsigned char joy_axes[2][8];

int joyaxis_LR, joyaxis_UD;

unsigned char *sdl_keys;

static unsigned long fe_timer_read(void);
static void fe_ProcessEvents (void);
unsigned long pi_joystick_read(void);

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
	unsigned int longueur[NB_MAX_GAMES];
	unsigned int long_max;
}data;

unsigned int listing_tri[NB_FILTRE][NB_MAX_GAMES];

struct options
{
	unsigned char y;
	unsigned char num;
	unsigned int offset_num;
	
	unsigned int cpu;
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
}options;

struct conf
{
	unsigned char exist;
//sq	char *cf;
	unsigned int cpu;
	unsigned char sound;
	unsigned int samplerate;
	unsigned char rescale;
	unsigned char showfps;
	unsigned char tweak;
	unsigned char hiscore;
} conf;

struct capex
{
	unsigned char clock;
	unsigned char tweak;
	unsigned char delayspeed;
	signed char repeatspeed;
	unsigned char list;
	unsigned char sely;
	unsigned int selnum;
	unsigned int seloffset_num;
	signed char FXshadow;
	float FLshadow;
	unsigned char skin;
}capex;

struct selector
{
	unsigned char y;
	unsigned char crt_x;
	unsigned int num;
	unsigned int offset_num;
}selector;

struct run
{
	unsigned char y;
	signed char num;
}run;

#include "capex_ecriture.h"
#include "capex_lecture.h"
#include "capex_tri.h"
#include "capex_pixel.h"

void free_memory(void)
{
	SDL_FreeSurface(screen); screen=0;
	SDL_FreeSurface(bg); bg=0;
	SDL_FreeSurface(bgs); bgs=0;
	SDL_FreeSurface(bg_temp); bg_temp=0;
	SDL_FreeSurface(font); font=0;
	SDL_FreeSurface(barre); barre=0;
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

	SDL_JoystickClose(0);
	SDL_Quit();

	exit(0);

}

void put_string(char *string, unsigned int pos_x, unsigned int pos_y, unsigned char couleur, SDL_Surface *s)
{
	SDL_Rect Src;
	SDL_Rect Dest;

	Src.y = couleur;
	Src.w = 5;
	Src.h = 9;
	Dest.y = pos_y;

	while(*string)
	{
		if (font6x[*string]){
			Src.x = font6x[*string];
			Dest.x = pos_x;
			SDL_BlitSurface(font, &Src, s, &Dest);
		}
		++string;
		pos_x += 6;
	}
}

void put_stringM(char *string, unsigned int pos_x, unsigned int pos_y, unsigned int taille, unsigned char couleur)
{
	SDL_Rect Src;
	SDL_Rect Dest;
	unsigned char caratere;
	
	if( taille > selector.crt_x ){
		
		string += selector.crt_x ;
		
		Src.y = couleur;
		Src.w = 5;
		Src.h = 9;
		Dest.y = pos_y;

		
		if ( (taille-selector.crt_x) > 52 ) {
			for( caratere=selector.crt_x ; caratere<(selector.crt_x+48) ; ++caratere)
			{
				if (font6x[*string]){
					Src.x = font6x[*string];
					Dest.x = pos_x;
					SDL_BlitSurface(font, &Src, screen, &Dest);
				}
				++string;
				pos_x += 6;
			}
			for( caratere=0 ; caratere<3 ; ++caratere)
			{
				if (font6x[*string]){
					Src.x = font6x[46];
					Dest.x = pos_x;
					SDL_BlitSurface(font, &Src, screen, &Dest);
				}
				pos_x += 6;
			}
		}else{
			while(*string)
			{
				if (font6x[*string]){
					Src.x = font6x[*string];
					Dest.x = pos_x;
					SDL_BlitSurface(font, &Src, screen, &Dest);
				}
				++string;
				pos_x += 6;
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
	drawSprite( screen , credit , 0 , 0 , 0 , 0 , 320 , 240 );
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
		drawSprite( credit , screen , 0 , 0 , 0 , 0 , 320 , 240 );
		SDL_Flip(screen);
		
		SDL_PollEvent(&event);
		if (event.type==SDL_JOYBUTTONDOWN){
			if (counter==0 || (counter>capex.delayspeed && ((counter&joy_speed[capex.repeatspeed])==0))){
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
	#define HELP_X	53
	#define HELP_Y	62
	#define HELP_L	214
	#define HELP_H	116
	
	unsigned int counter = 1;
	Uint32 PBLACK = SDL_MapRGB(help->format, 0, 0, 0);	
	Uint32 PWHITE = SDL_MapRGB(help->format, 255, 255, 255 );
	
	//capture screen actuel
	drawSprite( screen , help , 0 , 0 , 0 , 0 , 320 , 240 );
	//detourage
	ligneV( help , HELP_X-1 , HELP_Y , HELP_H , PBLACK );
	ligneV( help , HELP_X+HELP_L , HELP_Y , HELP_H , PBLACK );
	ligneH( help , HELP_X , HELP_Y-1 , HELP_L , PBLACK);
	ligneH( help , HELP_X , HELP_Y+HELP_H , HELP_L , PBLACK);
	//cadre
	carre_plein( help , HELP_X, HELP_Y, HELP_L, HELP_H, PWHITE);
	carre_plein( help , HELP_X+1, HELP_Y+1, HELP_L-2, HELP_H-2, PBLACK);
	
	put_string( "COLOR HELP" , 130 , HELP_Y+8 , WHITE , help );
	put_string( "RED      missing" , HELP_X+8 , HELP_Y+24 , RED , help );
	put_string( "ORANGE   clone rom detected" , HELP_X+8 , HELP_Y+34 , ORANGE , help );
	put_string( "YELLOW   parent or clone detected" , HELP_X+8 , HELP_Y+44 , YELLOW , help );
	put_string( "         & parent detected" , HELP_X+8 , HELP_Y+54 , YELLOW , help );
	put_string( "GREEN    clone & parent & cache" , HELP_X+8 , HELP_Y+64 , GREEN , help );
	put_string( "         detected" , HELP_X+8 , HELP_Y+74 , GREEN , help );
	put_string( "BLUE     parent & cache detected" , HELP_X+8 , HELP_Y+84 , BLUE , help );
	put_string( "Any button to return" , 100 , HELP_Y+100 , WHITE , help );

	int Hquit = 0;
	while(!Hquit)
	{
		drawSprite( help , screen , 0 , 0 , 0 , 0 , 320 , 240 );
		SDL_Flip(screen);
		
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

	//load background interne ou skin
	rw = SDL_RWFromMem(gfx_BG,sizeof(gfx_BG)/sizeof(unsigned char));
	Tmp = SDL_LoadBMP_RW(rw,0);
	if ((fp = fopen( "./skin/capex_bg.bmp" , "r")) != NULL){
		Tmp = SDL_LoadBMP( "./skin/capex_bg.bmp" );
		fclose(fp);
	}
	bg = SDL_DisplayFormat(Tmp);
	SDL_FreeSurface(Tmp);
	
	//load selector interne ou skin
	rw = SDL_RWFromMem(gfx_SELECTEUR,sizeof(gfx_SELECTEUR)/sizeof(unsigned char));
	Tmp = SDL_LoadBMP_RW(rw,0);
	if ((fp = fopen( "./skin/capex_selector.bmp" , "r")) != NULL){
		Tmp = SDL_LoadBMP( "./skin/capex_selector.bmp" );
		fclose(fp);
	}
	barre = SDL_DisplayFormat(Tmp);
	SDL_FreeSurface(Tmp);
	SDL_SetColorKey(barre ,SDL_SRCCOLORKEY,SDL_MapRGB(barre ->format,255,0,255));
	
	//load title interne ou skin
	rw = SDL_RWFromMem(gfx_CAPEX,sizeof(gfx_CAPEX)/sizeof(unsigned char));
	Tmp = SDL_LoadBMP_RW(rw,0);
	if ((fp = fopen( "./skin/capex_title.bmp" , "r")) != NULL){
		Tmp = SDL_LoadBMP( "./skin/capex_title.bmp" );
		fclose(fp);
	}
	title = SDL_DisplayFormat(Tmp);
	SDL_FreeSurface(Tmp);
	SDL_SetColorKey(title ,SDL_SRCCOLORKEY,SDL_MapRGB(title ->format,0,255,255));
	
	rw = SDL_RWFromMem(gfx_FONT,sizeof(gfx_FONT)/sizeof(unsigned char));
	Tmp = SDL_LoadBMP_RW(rw,0);
	font = SDL_DisplayFormat(Tmp);
	SDL_FreeSurface(Tmp);
	SDL_FreeRW (rw);
	SDL_SetColorKey(font,SDL_SRCCOLORKEY,SDL_MapRGB(font->format,255,0,255));

	bg_temp = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0xf800, 0x07e0, 0x001f, 0x0000);
	bgs = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0xf800, 0x07e0, 0x001f, 0x0000);
	help = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0xf800, 0x07e0, 0x001f, 0x0000);
	credit = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0xf800, 0x07e0, 0x001f, 0x0000);

	drawSprite( bg , bgs , 0 , 0 , 0 , 0 , 320 , 240 );
	if (capex.FXshadow < 100) pixel_gamma( bgs );
	
	preparation_fenetre( bgs , bg , 124 , 3 , 192 , 112 );
	preparation_fenetre( bgs , bg , 4 , 53 , 116 , 62 );
	drawSprite( title , bg, 0, 0, 0, 0, 123, 52);
	
}

void prep_bg_run(void)
{
	drawSprite( bg , bg_temp , 0 , 0 , 0 , 0 , 320 , 240 );
	preparation_fenetre( bgs , bg_temp , 4 , 119 , 312 , 37 );
	if ( flag_preview )	drawSprite(preview, bg_temp, 0, 0, 220-preview->w/2, 3, 192, 112);
		
	put_string( "FBA2X clock:" , 6 , 55 , WHITE , bg_temp);
	put_string( "ROM" , 6 , 65 , WHITE , bg_temp);
		switch(capex.list)
		{
			case 0:
				sprintf((char*)g_string, "Database: %d roms" , data.nb_list[0] );
				break;
			case 1:
				sprintf((char*)g_string, "Missing: %d roms" , data.nb_list[1] );
				break;
			case 2:
				sprintf((char*)g_string, "Available: %d roms" , data.nb_list[2] );
				break;
			case 3:
				sprintf((char*)g_string, "Available: %d roms" , data.nb_list[3] );
				break;
		}
		put_string( g_string , 6 , 105 , WHITE , bg_temp );
}

void prep_bg_options(void)
{
	drawSprite( bg , bg_temp , 0 , 0 , 0 , 0 , 320 , 240 );
	preparation_fenetre( bgs , bg_temp , 4 , 119 , 260 , 118 );
	preparation_fenetre( bgs , bg_temp , 269 , 119 , 47 , 118 );
	if ( flag_preview )	drawSprite(preview, bg_temp, 0, 0, 220-preview->w/2, 3, 192, 112);
		
	put_string( "ROM" , 6 , 65 , WHITE , bg_temp);
		switch(capex.list)
		{
			case 0:
				sprintf((char*)g_string, "Database: %d roms" , data.nb_list[0] );
				break;
			case 1:
				sprintf((char*)g_string, "Missing: %d roms" , data.nb_list[1] );
				break;
			case 2:
				sprintf((char*)g_string, "Available: %d roms" , data.nb_list[2] );
				break;
			case 3:
				sprintf((char*)g_string, "Available: %d roms" , data.nb_list[3] );
				break;
		}
		put_string( g_string , 6 , 105 , WHITE , bg_temp );
}

void prep_bg_list(void)
{
	drawSprite( bg , bg_temp , 0 , 0 , 0 , 0 , 320 , 240 );
	preparation_fenetre( bgs , bg_temp , 4 , 119 , 312 , 118 );
	if ( flag_preview )	drawSprite(preview, bg_temp, 0, 0, 220-preview->w/2, 3, 192, 112);
		
	put_string( "ROM" , 6 , 65 , WHITE , bg_temp);
    switch(capex.list)
    {
        case 0:
            sprintf((char*)g_string, "Database: %d roms" , data.nb_list[0] );
            break;
        case 1:
            sprintf((char*)g_string, "Missing: %d roms" , data.nb_list[1] );
            break;
        case 2:
            sprintf((char*)g_string, "Available: %d roms" , data.nb_list[2] );
            break;
        case 3:
            sprintf((char*)g_string, "Available: %d roms" , data.nb_list[3] );
            break;
    }
    put_string( g_string , 6 , 105 , WHITE , bg_temp );
}

void display_BG(void)
{
	FILE *fp2;

    drawSprite( bg_temp , screen , 0 , 0 , 0 , 0 , 320 , 240 );
		
//sqdebug
//	sprintf((char*)g_string, "%d %d" , selector.num, selector.offset_num );
//	put_string( g_string , 30 , 55 , WHITE , screen );
    
    //.fba files prioritise over .rom files
    sprintf((char*)g_string, "roms/%s.fba" , data.zip[listing_tri[capex.list][selector.num]]);
    if ((fp2 = fopen(g_string, "r")) != NULL){
        sprintf((char*)g_string, "%s.fba" , data.zip[listing_tri[capex.list][selector.num]]);
        fclose(fp2);
    }
	else {
        sprintf((char*)g_string, "%s.zip" , data.zip[listing_tri[capex.list][selector.num]]);
    }
    
    put_string( g_string , 30 , 65 , WHITE , screen );
    
    if ( strcmp( data.parent[listing_tri[capex.list][selector.num]] , "fba" ) == 0 ){
        put_string( "Parent rom" , 6 , 75 , WHITE , screen );
    }else{
        sprintf((char*)g_string, "Clone of %s" , data.parent[listing_tri[capex.list][selector.num]]);
        put_string( g_string , 6 , 75 , WHITE , screen );
    }		
    
    if ( data.status[listing_tri[capex.list][selector.num]] != NULL ){
        put_string( data.status[listing_tri[capex.list][selector.num]] , 6 , 95 , WHITE , screen );
    }

}

void display_line_options(unsigned char num, unsigned char y)
{
	#define OPTIONS_START_X	8
	#define CONF_START_X	272

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
			if (options.showfps) put_string( "Show FPS: Enable" , OPTIONS_START_X , y , WHITE , screen );
			else put_string( "Show FPS: Disable" , OPTIONS_START_X , y , WHITE , screen );
			if (conf.exist){
				put_string( abreviation_cf[0][conf.showfps] , CONF_START_X , y , GREEN , screen );
				
			}else{
				put_string( "-" , CONF_START_X , y , RED , screen );
			}
			break;
		case OPTION_NUM_CAPEX_LIST:
			if (capex.list == 3) put_string( "Listing view: Available without clones" , OPTIONS_START_X , y , WHITE , screen );
			else if (capex.list == 2) put_string( "Listing view: Available only" , OPTIONS_START_X , y , WHITE , screen );
			else if (capex.list == 1) put_string( "Listing view: Missing only" , OPTIONS_START_X , y , WHITE , screen );
			else put_string( "Listing view: All" , OPTIONS_START_X , y , WHITE , screen );
			break;
//		case OPTION_NUM_SAVE:
//			put_string( "Save all settings" , OPTIONS_START_X , y , flag_save , screen );
//			break;
//		case OPTION_NUM_SAVE_CF:
//			sprintf((char*)g_string, "Save FBA2X settings in %s.cf file" , conf.cf );
//			put_string( g_string , OPTIONS_START_X , y , flag_save , screen );
//			break;
		case OPTION_NUM_RETURN:
			put_string( "Return to the game list" , OPTIONS_START_X , y , WHITE , screen );
			break;
	}
}

void ss_prg_options(void)
{
	int Quit;
//	unsigned int counter = 1;
	unsigned char y;

	Uint32 Joypads;
	unsigned long keytimer=0;
	int keydirection=0, last_keydirection=0;

	flag_save = GREEN;
	options.y = START_Y-1;
	options.num = 0;
	options.offset_num = 0;
	
	unsigned option_start;
	
	prep_bg_options();
		
	Quit=0;
	while(!Quit)
	{
        
		display_BG();
		
		drawSprite(barre , screen, 0, 0, 4, options.y, 260, 10);
		drawSprite(barre , screen, 0, 0, 269, options.y, 47, 10);
		
		option_start = START_Y;
		for ( y = options.offset_num ; y<(options.offset_num+13) ; ++y){
			display_line_options( y , option_start );
			option_start += 9;
		}

		SDL_Flip(screen);
        
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
            //++ options.num;
            //options.y += 9;
            if ( options.num == NOMBRE_OPTIONS ){
                options.y = START_Y-1;
                options.num = 0;
                options.offset_num = 0;
            }
            else {
//                if (options.num < 6 || options.offset_num == ( NOMBRE_OPTIONS - 5 ) ){
                if (options.num < NOMBRE_OPTIONS || options.offset_num == ( NOMBRE_OPTIONS - 5 ) ){
                    if ( options.num < NOMBRE_OPTIONS ){
                        options.y += 9;
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
            //-- options.num;
            //options.y -= 9;
            if ( options.num == 0 ){
//                options.y = START_Y -1 + (12*9) ;
                options.y = START_Y -1 + (NOMBRE_OPTIONS*9) ;
                options.num = NOMBRE_OPTIONS;
                options.offset_num = 0;
            }else{
                if ( options.num > ( NOMBRE_OPTIONS - 6 ) || options.offset_num == 0 ){
                    if (options.num>0){
                        options.y -= 9;
                        --options.num;
                    }
                }else{
                    --options.offset_num;
                    --options.num;
                }
            }
        }
        
        if(Joypads & GP2X_A) {
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
                    selector.y = START_Y-1;
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

////    if (options.sound){
////        argument[ ar ] = "--sound";
////        ++ar;
////
////    }else{
////        argument[ ar ] = "--no-sound";
////        ++ar;
////    }

    if (options.showfps){
		strcat(arglist, "--showfps");
		strcat(arglist, " ");
    }

    free_memory();
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
	joyaxis_LR = get_integer_conf("Joystick", "JA_LR", 0);
	joyaxis_UD = get_integer_conf("Joystick", "JA_UD", 1);

    close_config_file();

}


int main(int argc, char *argv[])
{
//    sleep(10);
    
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

	pi_initialize_input();

while(1)
{

	//Initialise everything SDL
	//sq SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO );
  	screen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE|SDL_NOFRAME);
	SDL_ShowCursor (0); //desactive souris
	SDL_JoystickOpen(0);

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
	for (i=32;i<255;++i) font6x[i]=(i-32)*6;
		
	//Load initial game row position from saved config values
    selector.y = capex.sely;
    selector.num = capex.selnum;
    selector.offset_num = capex.seloffset_num;
    
    selector.crt_x=0;
    
	flag_preview = 0;
	load_preview(selector.num);
	//sq load_cf();
	
	prep_bg_list();
	
	Quit = 0;
	while (!Quit)
	{
		display_BG();
		
		drawSprite(barre , screen, 0, 0, 4, selector.y, 312, 10);
		
		if (ErrorQuit == 1) put_string("Missing file zipname.fba" , 8 , 220 , 0 , screen );
		else if (ErrorQuit == 2) put_string("Missing file rominfo.fba" , 8 , 220 , 0 , screen );
		else{
			zipnum = START_Y;
			if ( data.nb_list[capex.list] < 14){
				for ( y = 0 ; y<data.nb_list[capex.list] ; ++y){
					put_stringM(data.name[listing_tri[capex.list][y]] , 8 , zipnum, data.longueur[listing_tri[capex.list][y]] , data.state[listing_tri[capex.list][y]] );
					zipnum += 9;
				}
			}else{
				for ( y = selector.offset_num ; y<(selector.offset_num+13) ; ++y){
					put_stringM(data.name[listing_tri[capex.list][y]] , 8 , zipnum, data.longueur[listing_tri[capex.list][y]] , data.state[listing_tri[capex.list][y]] );
					zipnum += 9;
				}
			}
		}
		
		SDL_Flip(screen);
        
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
				selector.y = START_Y-1;
				selector.num = 0;
				selector.offset_num = 0;
			}
			else {
				if (data.nb_list[capex.list] < 14) { 
					if (selector.num < (data.nb_list[capex.list]-1) ) {
						selector.y+=9;
						++selector.num;
					}
				}
                else {
					if (selector.num<7 || selector.offset_num==(data.nb_list[capex.list]-13)) {
						if (selector.num < (data.nb_list[capex.list]-1)){
							selector.y+=9;
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
            //load_cf();
		}
        
		if(Joypads & GP2X_UP) {
			updown=1;
			if ( selector.num==0 ){
				selector.num = data.nb_list[capex.list] - 1 ;
				if (data.nb_list[capex.list] < 14){
					selector.y = START_Y -1 + ( ( data.nb_list[capex.list] - 1 ) *9 );
					//selector.offset_num = 0;
				}
                else {
					selector.y = START_Y -1 + (12*9) ;
					selector.offset_num = data.nb_list[capex.list] - 13;
				}
			}
            else {
				if ( selector.num > (data.nb_list[capex.list]-7) || selector.offset_num==0) {
					if (selector.num>0){
						selector.y-=9;
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
            if ( selector.num < (data.nb_list[capex.list]-14)) {
                selector.y = START_Y-1;
                selector.num += 13;
                selector.offset_num = selector.num;
                if(selector.offset_num > data.nb_list[capex.list]-14) selector.offset_num=data.nb_list[capex.list]-14;
            } else {
                selector.y = START_Y-1;
                selector.num = data.nb_list[capex.list]-13;
                selector.offset_num = selector.num;
            }
            load_preview(selector.num);
            //load_cf();
        }
        
        if (Joypads & GP2X_LEFT && !updown) {
            if ( selector.num >= 13) {
                selector.y = START_Y-1;
                selector.num -= 13;
                selector.offset_num = selector.num;
            } else {
                selector.y = START_Y-1;
                selector.offset_num = 0;
                selector.num = 0;
            }
            load_preview(selector.num);
            //load_cf();
        }

		if (Joypads & GP2X_SELECT || Joypads & GP2X_ESCAPE) {
            capex.sely = selector.y;
            capex.selnum = selector.num;
            capex.seloffset_num = selector.offset_num;
            write_cfg();    //Write the current selection to the config
			exit_prog();
            Quit=1;
        }
                
        if (Joypads & GP2X_A ){
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
                
        if( Joypads & GP2X_START ) ss_prg_options();

	}

}

//	exit_prog();
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
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_JOYBUTTONDOWN:
                joy_buttons[event.jbutton.which][event.jbutton.button] = 1;
                break;
            case SDL_JOYBUTTONUP:
                joy_buttons[event.jbutton.which][event.jbutton.button] = 0;
                break;
           case SDL_JOYAXISMOTION:
                if(event.jaxis.axis == joyaxis_LR) {
                    if(event.jaxis.value > -10000 && event.jaxis.value < 10000)
                        joy_axes[event.jbutton.which][joyaxis_LR] = CENTER;
                    else if(event.jaxis.value > 10000)
                        joy_axes[event.jbutton.which][joyaxis_LR] = RIGHT;
                    else
                        joy_axes[event.jbutton.which][joyaxis_LR] = LEFT;
                }
                if(event.jaxis.axis == joyaxis_UD) {
                    if(event.jaxis.value > -10000 && event.jaxis.value < 10000)
                        joy_axes[event.jbutton.which][joyaxis_UD] = CENTER;
                    else if(event.jaxis.value > 10000)
                        joy_axes[event.jbutton.which][joyaxis_UD] = DOWN;
                    else
                        joy_axes[event.jbutton.which][joyaxis_UD] = UP;
                }
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
	if (joy_axes[0][joyaxis_UD] == UP)         val |= GP2X_UP;
	if (joy_axes[0][joyaxis_UD] == DOWN)       val |= GP2X_DOWN;
	if (joy_axes[0][joyaxis_LR] == LEFT)       val |= GP2X_LEFT;
	if (joy_axes[0][joyaxis_LR] == RIGHT)      val |= GP2X_RIGHT;


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

