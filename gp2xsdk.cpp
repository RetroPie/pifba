#include "gp2xsdk.h"
#include "gp2xmemfuncs.h"
#include "burner.h"
#include "config.h"

#include <bcm_host.h>
#include <SDL.h>
#include <assert.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

static SDL_Surface* sdlscreen = NULL;

#define BLOCKSIZE 1024
//#define SetTaken(Start, Size) TakenSize[(Start - 0x2000000) / BLOCKSIZE] = (Size - 1) / BLOCKSIZE + 1

#define logerror printf

static int mem_fd = -1;
//void *UpperMem;
//int TakenSize[0x2000000 / BLOCKSIZE];
unsigned short *VideoBuffer = NULL;
static int screen_mode = 0;
volatile static unsigned short *gp2xregs = NULL;
unsigned long gp2x_physvram[4]={0,0,0,0};
unsigned short *framebuffer[4]={0,0,0,0};
static int currentframebuffer = 0;
struct usbjoy *joys[4];
char joyCount = 0;

extern CFG_OPTIONS config_options;

static int surface_width;
static int surface_height;

void Disable_940(void)
{
	gp2xregs[0x3B42>>1];
	gp2xregs[0x3B42>>1]=0;
	gp2xregs[0x3B46>>1]=0xffff;	
	gp2xregs[0x3B48>>1]|= (1 << 7);
	gp2xregs[0x904>>1]&=0xfffe;
}

//void InitMemPool() {
//  //Try to apply MMU hack.
//  UpperMem = mmap(0, 0x2000000, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0x2000000);
//  gp2x_memset(TakenSize, 0, sizeof(TakenSize));
//
//  //SetTaken(0x3000000, 0x80000); // Video decoder (you could overwrite this, but if you
//                                // don't need the memory then be nice and don't)
//  //SetTaken(0x3101000, 153600);  // Primary frame buffer
//  //SetTaken(0x3381000, 153600);  // Secondary frame buffer (if you don't use it, uncomment)
//  SetTaken(0x3600000, 0x80000);  // Sound buffer
//  SetTaken(0x3F2E000, 0xD4000);  // Video Buffers
//}

//void DestroyMemPool() {
//  munmap (UpperMem,0x2000000);
//  UpperMem = NULL;
//}

void gp2x_initialize()
{
	for (int i=1; i<5; i++)
	{
		struct usbjoy *joy = joy_open(i);
		if(joy != NULL)
		{
			joys[joyCount] = joy;
			joyCount++;
		}
	}

//	mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
//
//	framebuffer[0]=(unsigned short *)mmap(0, 0x35000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, (gp2x_physvram[0]=0x04000000-(0x35000*4) )); 
//	framebuffer[1]=(unsigned short *)mmap(0, 0x35000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, (gp2x_physvram[1]=0x04000000-(0x35000*3) )); 
//	framebuffer[2]=(unsigned short *)mmap(0, 0x35000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, (gp2x_physvram[2]=0x04000000-(0x35000*2) )); 
//	framebuffer[3]=(unsigned short *)mmap(0, 0x35000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, (gp2x_physvram[3]=0x04000000-(0x35000) )); 
//	InitMemPool();
//	gp2xregs = (unsigned short *)mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0xC0000000);
//	if(gp2xregs)
//	{
//		Disable_940();
//		int tMRD = 0, tRFC = 0, tRP = 1, tRCD = 1;
//		int tRC = 6, tRAS = 3, tWR = 0;
//		gp2xregs[0x28e2>>1] = 0;
//		gp2xregs[0x28e6>>1] = 0;
//		if(gp2xregs[0x2816>>1] == 319) { // LCD
//			screen_mode = 0;
//			gp2xregs[0x28e4>>1] = 319;
//			gp2xregs[0x28e8>>1] = 239;
//		} else { // TV
//			gp2xregs[0x28e4>>1] = 719;
//			if (gp2xregs[0x2818>>1]  == 287) { // PAL
//				screen_mode = 1;
//				gp2xregs[0x28e8>>1] = 287;
//			} else if (gp2xregs[0x2818>>1]  == 239) { // NTSC
//				screen_mode = 2;
//				gp2xregs[0x28e8>>1] = 239;
//			}
//		}
//
//		gp2xregs[0x3802>>1] = ((tMRD & 0xF) << 12) | ((tRFC & 0xF) << 8) | ((tRP & 0xF) << 4) | (tRCD & 0xF);
//		gp2xregs[0x3804>>1] = /*0x9000 |*/ ((tRC & 0xF) << 8) | ((tRAS & 0xF) << 4) | (tWR & 0xF);
//
//		if(mem_fd < 0) {
//			fprintf (stderr, "Couldn't open /dev/mem device.\n");
//			exit (1);
//		} else {
//			int mmu_fd;
//			mmu_fd = open("/dev/mmuhack", O_RDWR);
//			if(mmu_fd < 0) {
//				system("/sbin/insmod -f mmuhack.o");
//				mmu_fd = open("/dev/mmuhack", O_RDWR);
//			}
//			if(mmu_fd >= 0) close(mmu_fd);
//		}
        init_SDL();
		gp2x_setvideo_mode(320,240);
		gp2x_clear_framebuffers();
		gp2x_video_flip();
	
}

void gp2x_terminate(char *frontend)
{
//struct stat info;
//
//	for (int i=0; i<joyCount; i++)
//	{
//		joy_close(joys[i]);
//	}
//	DestroyMemPool();
//	gp2x_setvideo_mode(320,240);
//	munmap(framebuffer[0],0x40000);
//	munmap(framebuffer[1],0x40000);
//	if(gp2xregs) munmap((void *)gp2xregs, 0x10000);
//	if(mem_fd >= 0) close(mem_fd);
//	system("/sbin/rmmod mmuhack");
//	
//	if( (lstat(frontend, &info) == 0) && S_ISREG(info.st_mode) )
//	{
//	char path[256];
//	char *p;
//		strcpy(path, frontend);
//		p = strrchr(path, '/');
//		if(p == NULL) p = strrchr(path, '\\');
//		if(p != NULL)
//		{
//			*p = '\0';
//			chdir(path);
//		}
//		execl(frontend, frontend, NULL);
//	}
//	else
//	{
//		chdir("/usr/gp2x");
//		execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
//	}
    
    gp2x_deinit();
    init_SDL();

}

// create two resources for 'page flipping'
static DISPMANX_RESOURCE_HANDLE_T   resource0;
static DISPMANX_RESOURCE_HANDLE_T   resource1;
static DISPMANX_RESOURCE_HANDLE_T   resource_bg;

// these are used for switching between the buffers
static DISPMANX_RESOURCE_HANDLE_T cur_res;
static DISPMANX_RESOURCE_HANDLE_T prev_res;
static DISPMANX_RESOURCE_HANDLE_T tmp_res;

DISPMANX_ELEMENT_HANDLE_T dispman_element;
DISPMANX_ELEMENT_HANDLE_T dispman_element_bg;
DISPMANX_DISPLAY_HANDLE_T dispman_display;
DISPMANX_UPDATE_HANDLE_T dispman_update;

void gles2_create(int display_width, int display_height, int bitmap_width, int bitmap_height, int depth);
void gles2_destroy();
void gles2_palette_changed();

EGLDisplay display = NULL;
EGLSurface surface = NULL;
static EGLContext context = NULL;
static EGL_DISPMANX_WINDOW_T nativewindow;


void exitfunc()
{
	SDL_Quit();
	bcm_host_deinit();
}

SDL_Joystick* myjoy[4];

int init_SDL(void)
{
	myjoy[0]=0;
	myjoy[1]=0;
	myjoy[2]=0;
	myjoy[3]=0;
    
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return(0);
    }
    sdlscreen = SDL_SetVideoMode(0,0, 16, SDL_SWSURFACE);
    
	//We handle up to four joysticks
	if(SDL_NumJoysticks())
	{
		int i;
    	SDL_JoystickEventState(SDL_ENABLE);
		
		for(i=0;i<SDL_NumJoysticks();i++) {
			myjoy[i]=SDL_JoystickOpen(i);
            
			//Check for valid joystick, some keyboards
			//aren't SDL compatible
			if(myjoy[i])
			{
				if (SDL_JoystickNumAxes(myjoy[i]) > 6)
				{
					SDL_JoystickClose(myjoy[i]);
					myjoy[i]=0;
					logerror("Error detected invalid joystick/keyboard\n");
					break;
				}
			}
		}
		if(myjoy[0])
			logerror("Found %d joysticks\n",SDL_NumJoysticks());
	}
	SDL_EventState(SDL_ACTIVEEVENT,SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT,SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE,SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT,SDL_IGNORE);
	SDL_ShowCursor(SDL_DISABLE);
    
    //Initialise dispmanx
    bcm_host_init();
    
	//Clean exits, hopefully!
	atexit(exitfunc);
    
    return(1);
}

void deinit_SDL(void)
{
    if(sdlscreen)
    {
        SDL_FreeSurface(sdlscreen);
        sdlscreen = NULL;
    }
    SDL_Quit();
    
    bcm_host_deinit();
}

void gp2x_deinit(void)
{
	int ret;
    
    gles2_destroy();
    // Release OpenGL resources
    eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    eglDestroySurface( display, surface );
    eglDestroyContext( display, context );
    eglTerminate( display );
    
	dispman_update = vc_dispmanx_update_start( 0 );
	ret = vc_dispmanx_element_remove( dispman_update, dispman_element );
	ret = vc_dispmanx_element_remove( dispman_update, dispman_element_bg );
	ret = vc_dispmanx_update_submit_sync( dispman_update );
	ret = vc_dispmanx_resource_delete( resource0 );
	ret = vc_dispmanx_resource_delete( resource1 );
	ret = vc_dispmanx_resource_delete( resource_bg );
	ret = vc_dispmanx_display_close( dispman_display );
    
	if(VideoBuffer) free(VideoBuffer);
	VideoBuffer=0;
}

static uint32_t display_adj_width, display_adj_height;		//display size minus border

void gp2x_setvideo_mode(int width, int height)
{
    
	int ret;
	uint32_t display_width, display_height;
	uint32_t display_x=0, display_y=0;
	float display_ratio,game_ratio;
    
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;
    
	surface_width = width;
	surface_height = height;
    
	VideoBuffer=(unsigned short *) calloc(1, width*height*2);
    
	// get an EGL display connection
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(display != EGL_NO_DISPLAY);
    
	// initialize the EGL display connection
	EGLBoolean result = eglInitialize(display, NULL, NULL);
	assert(EGL_FALSE != result);
    
	// get an appropriate EGL frame buffer configuration
	EGLint num_config;
	EGLConfig config;
	static const EGLint attribute_list[] =
	{
	    EGL_RED_SIZE, 8,
	    EGL_GREEN_SIZE, 8,
	    EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
	    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	    EGL_NONE
	};
	result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);
    
	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);
    
	// create an EGL rendering context
	static const EGLint context_attributes[] =
	{
	    EGL_CONTEXT_CLIENT_VERSION, 2,
	    EGL_NONE
	};
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
	assert(context != EGL_NO_CONTEXT);
    
	// create an EGL window surface
	int32_t success = graphics_get_display_size(0, &display_width, &display_height);
	assert(success >= 0);
    
	display_adj_width = display_width - (config_options.option_display_border * 2);
	display_adj_height = display_height - (config_options.option_display_border * 2);
    
//	if (options.display_smooth_stretch)
	{
		//We use the dispmanx scaler to smooth stretch the display
		//so GLES2 doesn't have to handle the performance intensive postprocessing
        
	    uint32_t sx, sy;
        
	 	// Work out the position and size on the display
	 	display_ratio = (float)display_width/(float)display_height;
	 	game_ratio = (float)width/(float)height;
        
		display_x = sx = display_adj_width;
		display_y = sy = display_adj_height;
        
	 	if (game_ratio>display_ratio)
			sy = (float)display_adj_width/(float)game_ratio;
	 	else
			sx = (float)display_adj_height*(float)game_ratio;
        
		// Centre bitmap on screen
	 	display_x = (display_x - sx) / 2;
	 	display_y = (display_y - sy) / 2;
        
		vc_dispmanx_rect_set( &dst_rect,
                             display_x + config_options.option_display_border,
                             display_y + config_options.option_display_border,
                             sx, sy);
	}
//	else
//		vc_dispmanx_rect_set( &dst_rect, config_options.option_display_border,
                            config_options.option_display_border,
//                          display_adj_width, display_adj_height);
    
//	if (options.display_smooth_stretch)
		vc_dispmanx_rect_set( &src_rect, 0, 0, width << 16, height << 16);
//	else
//		vc_dispmanx_rect_set( &src_rect, 0, 0, display_adj_width << 16, display_adj_height << 16);
    
	dispman_display = vc_dispmanx_display_open(0);
	dispman_update = vc_dispmanx_update_start(0);
	dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display,
                                              10, &dst_rect, 0, &src_rect,
                                              DISPMANX_PROTECTION_NONE, NULL, NULL, DISPMANX_NO_ROTATE);
    
	//Black background surface dimensions
	vc_dispmanx_rect_set( &dst_rect, 0, 0, display_width, display_height );
	vc_dispmanx_rect_set( &src_rect, 0, 0, 128 << 16, 128 << 16);
    
	//Create a blank background for the whole screen, make sure width is divisible by 32!
	uint32_t crap;
	resource_bg = vc_dispmanx_resource_create(VC_IMAGE_RGB565, 128, 128, &crap);
	dispman_element_bg = vc_dispmanx_element_add(  dispman_update, dispman_display,
                                                 9, &dst_rect, resource_bg, &src_rect,
                                                 DISPMANX_PROTECTION_NONE, 0, 0,
                                                 (DISPMANX_TRANSFORM_T) 0 );
    
	nativewindow.element = dispman_element;
//	if (options.display_smooth_stretch) {
		nativewindow.width = width;
		nativewindow.height = height;
//	}
//	else {
//		nativewindow.width = display_adj_width;
//		nativewindow.height = display_adj_height;
//	}
//    
	vc_dispmanx_update_submit_sync(dispman_update);
    
	surface = eglCreateWindowSurface(display, config, &nativewindow, NULL);
	assert(surface != EGL_NO_SURFACE);
    
	// connect the context to the surface
	result = eglMakeCurrent(display, surface, surface, context);
	assert(EGL_FALSE != result);
    
	//Smooth stretch the display size for GLES2 is the size of the bitmap
	//otherwise let GLES2 upscale (NEAR) to the size of the display
//	if (options.display_smooth_stretch) 
		gles2_create(width, height, width, height, 16);
//	else
//		gles2_create(display_adj_width, display_adj_height, width, height, bitmap->depth);
}




//void gp2x_setvideo_mode(int hw, int vh)
//{
//int hsc, vsc;
//
//	hw *= 2;
//	switch(screen_mode) {
//		case 1: // TV PAL
//			hsc = (float)(489.0 * hw) / 640;
//			vsc = (float)(vh * (hw * 274.0)/320) / 240;
//			break;
//		case 2: // TV NTSC
//			hsc = (float)(489.0 * hw) / 640;
//			vsc = (float)(vh * (hw * 331.0)/320) / 240;
//			break;
//		default: // LCD
//			hsc = (1024 * hw) / 640;
//			vsc = (vh * hw) / 240;
//	}
//	gp2xregs[0x2906>>1] = hsc & 0xFFFF;
//	gp2xregs[0x2908>>1] = vsc & 0xFFFF;
//	gp2xregs[0x290A>>1] = vsc >> 16;
//	gp2xregs[0x290C>>1] = hw;
//}

unsigned long gp2x_joystick_read(void)
{
//  unsigned long value=(gp2xregs[0x1198>>1] & 0x00FF);
//
//  if(value==0xFD) value=0xFA;
//  if(value==0xF7) value=0xEB;
//  if(value==0xDF) value=0xAF;
//  if(value==0x7F) value=0xBE;
//
//  return ~((gp2xregs[0x1184>>1] & 0xFF00) | value | (gp2xregs[0x1186>>1] << 16));
    return 0;
}

void gles2_draw(void * screen, int width, int height, int depth);
extern EGLDisplay display;
extern EGLSurface surface;

void gp2x_video_flip()
{
//	extern int throttle;
    static int throttle=1;
	static int save_throttle=0;
    
	if (throttle != save_throttle)
	{
		if(throttle)
			eglSwapInterval(display, 1);
		else
			eglSwapInterval(display, 0);
        
		save_throttle=throttle;
	}
    
    //Draw to the screen
   	gles2_draw(VideoBuffer, surface_width, surface_height, 16);
    eglSwapBuffers(display, surface);
}


//void gp2x_video_flip()
//{
//unsigned int address;
//
//	address=(unsigned int)gp2x_physvram[currentframebuffer];
//	gp2xregs[0x290E>>1]=(unsigned short)(address & 0xffff);
//	gp2xregs[0x2910>>1]=(unsigned short)(address >> 16);
//	gp2xregs[0x2912>>1]=(unsigned short)(address & 0xffff);
//	gp2xregs[0x2914>>1]=(unsigned short)(address >> 16);
//	currentframebuffer = ++currentframebuffer % 4;
//	VideoBuffer = framebuffer[currentframebuffer];
//}

void gp2x_clear_framebuffers()
{
//	gp2x_memset(framebuffer[0],0,0x35000);
//	gp2x_memset(framebuffer[1],0,0x35000);
//	gp2x_memset(framebuffer[2],0,0x35000);
//	gp2x_memset(framebuffer[3],0,0x35000);
}

void * UpperMalloc(size_t size)
{
    return (void*)calloc(1, size);
//  int i = 0;
//ReDo:
//  for (; TakenSize[i]; i += TakenSize[i]);
//  if (i >= 0x2000000 / BLOCKSIZE) {
//    printf("UpperMalloc out of mem!");
//    return NULL;
//  }
//  int BSize = (size - 1) / BLOCKSIZE + 1;
//  for(int j = 1; j < BSize; j++) {
//    if (TakenSize[i + j]) {
//      i += j;
//      goto ReDo; //OMG Goto, kill me.
//    }
//  }
//  
//  TakenSize[i] = BSize;
//  void* mem = ((char*)UpperMem) + i * BLOCKSIZE;
//  gp2x_memset(mem, 0, size);
//  return mem;
}

//Releases UpperMalloced memory
void UpperFree(void* mem)
{
    free(mem);
//  int i = (((int)mem) - ((int)UpperMem));
//  if (i < 0 || i >= 0x2000000) {
//    fprintf(stderr, "UpperFree of not UpperMalloced mem: %p\n", mem);
//  } else {
//    if (i % BLOCKSIZE)
//      fprintf(stderr, "delete error: %p\n", mem);
//    TakenSize[i / BLOCKSIZE] = 0;
//  }
}

////Returns the size of a UpperMalloced block.
//int GetUpperSize(void* mem)
//{
//  int i = (((int)mem) - ((int)UpperMem));
//  if (i < 0 || i >= 0x2000000) {
//    fprintf(stderr, "GetUpperSize of not UpperMalloced mem: %p\n", mem);
//    return -1;
//  }
//  return TakenSize[i / BLOCKSIZE] * BLOCKSIZE;
//}
