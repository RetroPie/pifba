#include "gp2xsdk.h"
#include "burner.h"
#include "config.h"

#include <bcm_host.h>
#include <SDL.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

static SDL_Surface* sdlscreen = NULL;

void logoutput(const char *text,...);

unsigned short *VideoBuffer = NULL;
SDL_Joystick *joy[4];
char joyCount = 0;

extern CFG_OPTIONS config_options;

extern bool bShowFPS;

static int surface_width;
static int surface_height;

unsigned char joy_buttons[4][32];
unsigned char joy_axes[4][2];

int joyaxis_LR[4], joyaxis_UD[4];

#define JOYLR 0
#define JOYUD 1

static GKeyFile *gkeyfile=0;

static void open_config_file(void)
{
    GError *error = NULL;
    
    gkeyfile = g_key_file_new ();
    if (!(int)g_key_file_load_from_file (gkeyfile, config_options.config_file, G_KEY_FILE_NONE, &error))
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

#define NUMKEYS 256
static Uint16 pi_key[NUMKEYS];
static Uint16 pi_joy[NUMKEYS];

void pi_initialize_input()
{
    memset(joy_buttons, 0, 32*4);
    memset(joy_axes, 0, 4*2);
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

    pi_key[A_2] = get_integer_conf("Keyboard", "A_2", RPI_KEY_A_2);
    pi_key[B_2] = get_integer_conf("Keyboard", "B_2", RPI_KEY_B_2);
    pi_key[X_2] = get_integer_conf("Keyboard", "X_2", RPI_KEY_X_2);
    pi_key[Y_2] = get_integer_conf("Keyboard", "Y_2", RPI_KEY_Y_2);
    pi_key[L_2] = get_integer_conf("Keyboard", "L_2", RPI_KEY_L_2);
    pi_key[R_2] = get_integer_conf("Keyboard", "R_2", RPI_KEY_R_2);
    pi_key[START_2] = get_integer_conf("Keyboard", "START_2", RPI_KEY_START_2);
    pi_key[SELECT_2] = get_integer_conf("Keyboard", "SELECT_2", RPI_KEY_SELECT_2);
    pi_key[LEFT_2] = get_integer_conf("Keyboard", "LEFT_2", RPI_KEY_LEFT_2);
    pi_key[RIGHT_2] = get_integer_conf("Keyboard", "RIGHT_2", RPI_KEY_RIGHT_2);
    pi_key[UP_2] = get_integer_conf("Keyboard", "UP_2", RPI_KEY_UP_2);
    pi_key[DOWN_2] = get_integer_conf("Keyboard", "DOWN_2", RPI_KEY_DOWN_2);
    
    pi_key[QUIT] = get_integer_conf("Keyboard", "QUIT", RPI_KEY_QUIT);
        
    //Configure joysticks from config file or defaults
    pi_joy[A_1] = get_integer_conf("Joystick", "A_1", RPI_JOY_A);
    pi_joy[B_1] = get_integer_conf("Joystick", "B_1", RPI_JOY_B);
    pi_joy[X_1] = get_integer_conf("Joystick", "X_1", RPI_JOY_X);
    pi_joy[Y_1] = get_integer_conf("Joystick", "Y_1", RPI_JOY_Y);
    pi_joy[L_1] = get_integer_conf("Joystick", "L_1", RPI_JOY_L);
    pi_joy[R_1] = get_integer_conf("Joystick", "R_1", RPI_JOY_R);
    pi_joy[START_1] = get_integer_conf("Joystick", "START_1", RPI_JOY_START);
    pi_joy[SELECT_1] = get_integer_conf("Joystick", "SELECT_1", RPI_JOY_SELECT);

    pi_joy[A_2] = get_integer_conf("Joystick", "A_2", RPI_JOY_A);
    pi_joy[B_2] = get_integer_conf("Joystick", "B_2", RPI_JOY_B);
    pi_joy[X_2] = get_integer_conf("Joystick", "X_2", RPI_JOY_X);
    pi_joy[Y_2] = get_integer_conf("Joystick", "Y_2", RPI_JOY_Y);
    pi_joy[L_2] = get_integer_conf("Joystick", "L_2", RPI_JOY_L);
    pi_joy[R_2] = get_integer_conf("Joystick", "R_2", RPI_JOY_R);
    pi_joy[START_2] = get_integer_conf("Joystick", "START_2", RPI_JOY_START);
    pi_joy[SELECT_2] = get_integer_conf("Joystick", "SELECT_2", RPI_JOY_SELECT);

    pi_joy[A_3] = get_integer_conf("Joystick", "A_3", RPI_JOY_A);
    pi_joy[B_3] = get_integer_conf("Joystick", "B_3", RPI_JOY_B);
    pi_joy[X_3] = get_integer_conf("Joystick", "X_3", RPI_JOY_X);
    pi_joy[Y_3] = get_integer_conf("Joystick", "Y_3", RPI_JOY_Y);
    pi_joy[L_3] = get_integer_conf("Joystick", "L_3", RPI_JOY_L);
    pi_joy[R_3] = get_integer_conf("Joystick", "R_3", RPI_JOY_R);
    pi_joy[START_3] = get_integer_conf("Joystick", "START_3", RPI_JOY_START);
    pi_joy[SELECT_3] = get_integer_conf("Joystick", "SELECT_3", RPI_JOY_SELECT);

    pi_joy[A_4] = get_integer_conf("Joystick", "A_4", RPI_JOY_A);
    pi_joy[B_4] = get_integer_conf("Joystick", "B_4", RPI_JOY_B);
    pi_joy[X_4] = get_integer_conf("Joystick", "X_4", RPI_JOY_X);
    pi_joy[Y_4] = get_integer_conf("Joystick", "Y_4", RPI_JOY_Y);
    pi_joy[L_4] = get_integer_conf("Joystick", "L_4", RPI_JOY_L);
    pi_joy[R_4] = get_integer_conf("Joystick", "R_4", RPI_JOY_R);
    pi_joy[START_4] = get_integer_conf("Joystick", "START_4", RPI_JOY_START);
    pi_joy[SELECT_4] = get_integer_conf("Joystick", "SELECT_4", RPI_JOY_SELECT);
    
    pi_joy[QUIT] = get_integer_conf("Joystick", "QUIT", RPI_JOY_QUIT);
    pi_joy[ACCEL] = get_integer_conf("Joystick", "ACCEL", RPI_JOY_ACCEL);
    
    pi_joy[QLOAD] = get_integer_conf("Joystick", "QLOAD", RPI_JOY_QLOAD);
    pi_joy[QSAVE] = get_integer_conf("Joystick", "QSAVE", RPI_JOY_QSAVE);

    //Read joystick axis to use, default to 0 & 1
    joyaxis_LR[0] = get_integer_conf("Joystick", "JA_LR_1", 0);
    joyaxis_UD[0] = get_integer_conf("Joystick", "JA_UD_1", 1);
    joyaxis_LR[1] = get_integer_conf("Joystick", "JA_LR_2", 0);
    joyaxis_UD[1] = get_integer_conf("Joystick", "JA_UD_2", 1);
    joyaxis_LR[2] = get_integer_conf("Joystick", "JA_LR_3", 0);
    joyaxis_UD[2] = get_integer_conf("Joystick", "JA_UD_3", 1);
    joyaxis_LR[3] = get_integer_conf("Joystick", "JA_LR_4", 0);
    joyaxis_UD[3] = get_integer_conf("Joystick", "JA_UD_4", 1);

    close_config_file();
    
}

void pi_parse_config_file (void)
{

    open_config_file();

    config_options.display_smooth_stretch = get_integer_conf("Graphics", "DisplaySmoothStretch", 1);
    config_options.option_display_border = get_integer_conf("Graphics", "DisplayBorder", 0);
    config_options.display_effect = get_integer_conf("Graphics", "DisplayEffect", 0);
    config_options.maintain_aspect_ratio = get_integer_conf("Graphics", "MaintainAspectRatio", 1);
    config_options.display_rotate = get_integer_conf("Graphics", "DisplayAutoRotate", 0);

    close_config_file();

}


void pi_initialize()
{

    pi_initialize_input();
    pi_parse_config_file();
    
    init_SDL();
    
    //Initialise display just for the rom loading screen first.
    pi_setvideo_mode(320,240);
    pi_video_flip();
    
}

void pi_terminate(void)
{
    struct stat info;

    pi_deinit();
    deinit_SDL();

    exit(0);
}

// create two resources for 'page flipping'
static DISPMANX_RESOURCE_HANDLE_T   resource0;
static DISPMANX_RESOURCE_HANDLE_T   resource1;
static DISPMANX_RESOURCE_HANDLE_T   resource_bg;

// these are used for switching between the buffers
//static DISPMANX_RESOURCE_HANDLE_T cur_res;
//static DISPMANX_RESOURCE_HANDLE_T prev_res;
//static DISPMANX_RESOURCE_HANDLE_T tmp_res;

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

int init_SDL(void)
{
    joy[0]=0;
    joy[1]=0;
    joy[2]=0;
    joy[3]=0;
    
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
            joy[i]=SDL_JoystickOpen(i);
            
            //Check for valid joystick, some keyboards
            //aren't SDL compatible
            if(joy[i])
            {
                if (SDL_JoystickNumAxes(joy[i]) > 28)
                {
                    SDL_JoystickClose(joy[i]);
                    joy[i]=0;
                    logoutput("Error detected invalid joystick/keyboard\n");
                }
                else
                    joyCount++;
            }
        }
        if(joyCount)
            logoutput("Found %d joystick(s)\n", joyCount);
    }
    if(joyCount < 2)
        joyCount = 2;

    //sq frig number of players for keyboard
    //joyCount=2;

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

static uint32_t display_adj_width, display_adj_height;      //display size minus border

void pi_setvideo_mode(int width, int height)
{
    
    uint32_t display_width, display_height;
    uint32_t display_x=0, display_y=0;
    float display_ratio,game_ratio;
    
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;
    
    surface_width = width;
    surface_height = height;
    
    VideoBuffer=(unsigned short *) calloc(1, width*height*4);
    
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
    
    if (config_options.display_smooth_stretch)
    {
        //We use the dispmanx scaler to smooth stretch the display
        //so GLES2 doesn't have to handle the performance intensive postprocessing
        
        uint32_t sx, sy;
        
        // Work out the position and size on the display
        display_ratio = display_width/(float)display_height;
        game_ratio = width/(float)height;
        
        display_x = sx = display_adj_width;
        display_y = sy = display_adj_height;
        
        if(config_options.maintain_aspect_ratio || game_ratio < 1) {
                if (game_ratio > display_ratio)
                    sy = display_adj_width / game_ratio;
                else
                    sx = display_adj_height * game_ratio;
        }
        
        // Centre bitmap on screen
        display_x = (display_x - sx) / 2;
        display_y = (display_y - sy) / 2;
        
        vc_dispmanx_rect_set( &dst_rect,
                             display_x + config_options.option_display_border,
                             display_y + config_options.option_display_border,
                             sx, sy);
    }
    else
        vc_dispmanx_rect_set( &dst_rect, config_options.option_display_border,
                            config_options.option_display_border,
                          display_adj_width, display_adj_height);
    
    if (config_options.display_smooth_stretch)
        vc_dispmanx_rect_set( &src_rect, 0, 0, width << 16, height << 16);
    else
        vc_dispmanx_rect_set( &src_rect, 0, 0, display_adj_width << 16, display_adj_height << 16);
    
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
    if (config_options.display_smooth_stretch) {
        nativewindow.width = width;
        nativewindow.height = height;
    }
    else {
        nativewindow.width = display_adj_width;
        nativewindow.height = display_adj_height;
    }
    
    vc_dispmanx_update_submit_sync(dispman_update);
    
    surface = eglCreateWindowSurface(display, config, &nativewindow, NULL);
    assert(surface != EGL_NO_SURFACE);
    
    // connect the context to the surface
    result = eglMakeCurrent(display, surface, surface, context);
    assert(EGL_FALSE != result);
    
    //Smooth stretch the display size for GLES2 is the size of the bitmap
    //otherwise let GLES2 upscale (NEAR) to the size of the display
    if (config_options.display_smooth_stretch) 
        gles2_create(width, height, width, height, 16);
    else
        gles2_create(display_adj_width, display_adj_height, width, height, 16);
}

void pi_deinit(void)
{
    gles2_destroy();
    // Release OpenGL resources
    eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    eglDestroySurface( display, surface );
    eglDestroyContext( display, context );
    eglTerminate( display );
    
    dispman_update = vc_dispmanx_update_start( 0 );
    vc_dispmanx_element_remove( dispman_update, dispman_element );
    vc_dispmanx_element_remove( dispman_update, dispman_element_bg );
    vc_dispmanx_update_submit_sync( dispman_update );
    vc_dispmanx_resource_delete( resource0 );
    vc_dispmanx_resource_delete( resource1 );
    vc_dispmanx_resource_delete( resource_bg );
    vc_dispmanx_display_close( dispman_display );
    
    if(VideoBuffer) free(VideoBuffer);
    VideoBuffer=0;
}

void gles2_draw(void * screen, int width, int height, int depth);
extern EGLDisplay display;
extern EGLSurface surface;

void pi_video_flip()
{
    //  extern int throttle;
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

int StatedLoad(int nSlot);
int StatedSave(int nSlot);

unsigned char *sdl_keys;

void pi_process_events (void)
{
    int num=0, i;
    int hatmovement=0;

    //Process four joysticks
    for(i=0;i<4;i++) {
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
                
                if (event.key.keysym.sym == SDLK_0)
                    bShowFPS = !bShowFPS;
                
//                if (event.key.keysym.sym == SDLK_F1)  num = 1;
//                else if (event.key.keysym.sym == SDLK_F2) num = 2;
//                else if (event.key.keysym.sym == SDLK_F3) num = 3;
//                else if (event.key.keysym.sym == SDLK_F4) num = 4;
//                if (num) {
//                    if (event.key.keysym.mod & KMOD_SHIFT)
//                        StatedSave(num);
//                    else
//                        StatedLoad(num);
//                }
                break;
            case SDL_KEYUP:
                sdl_keys = SDL_GetKeyState(NULL);
                break;
        }
        
    }
    
    //Check START+R,L for quicksave/quickload. Needs to go here outside of the internal processing
//  if (joy_buttons[0][pi_joy[QLOAD]] || (joy_buttons[0][pi_joy[SELECT_1]] && joy_buttons[0][pi_joy[L_1]] )) {
//      char fname[256];
//      strcpy(fname, S9xGetFilename (".000"));
//      S9xLoadSnapshot (fname);
//  }
//  if (joy_buttons[0][pi_joy[QSAVE]] || (joy_buttons[0][pi_joy[SELECT_1]] && joy_buttons[0][pi_joy[R_1]] )) {
//      char fname[256];
//      strcpy(fname, S9xGetFilename (".000"));
//      S9xFreezeGame (fname);
//  }
    
}

extern bool GameLooping;

unsigned long pi_joystick_read(int which1)
{
    unsigned long val=0;
    
    //Handle four players
    if(which1 > 3) return val;
    
    if (which1 == 0) {
        if (joy_buttons[0][pi_joy[L_1]])        val |= GP2X_L;
        if (joy_buttons[0][pi_joy[R_1]])        val |= GP2X_R;
        if (joy_buttons[0][pi_joy[X_1]])        val |= GP2X_X;
        if (joy_buttons[0][pi_joy[Y_1]])        val |= GP2X_Y;
        if (joy_buttons[0][pi_joy[B_1]])        val |= GP2X_B;
        if (joy_buttons[0][pi_joy[A_1]])        val |= GP2X_A;
        if (joy_buttons[0][pi_joy[START_1]])    val |= GP2X_START;
        if (joy_buttons[0][pi_joy[SELECT_1]])   val |= GP2X_SELECT;
        if (joy_axes[0][JOYUD] == UP)           val |= GP2X_UP;
        if (joy_axes[0][JOYUD] == DOWN)         val |= GP2X_DOWN;
        if (joy_axes[0][JOYLR] == LEFT)         val |= GP2X_LEFT;
        if (joy_axes[0][JOYLR] == RIGHT)        val |= GP2X_RIGHT;
    } 
    else if (which1 == 1) {
        if (joy_buttons[1][pi_joy[L_2]])        val |= GP2X_L;
        if (joy_buttons[1][pi_joy[R_2]])        val |= GP2X_R;
        if (joy_buttons[1][pi_joy[X_2]])        val |= GP2X_X;
        if (joy_buttons[1][pi_joy[Y_2]])        val |= GP2X_Y;
        if (joy_buttons[1][pi_joy[B_2]])        val |= GP2X_B;
        if (joy_buttons[1][pi_joy[A_2]])        val |= GP2X_A;
        if (joy_buttons[1][pi_joy[START_2]])    val |= GP2X_START;
        if (joy_buttons[1][pi_joy[SELECT_2]])   val |= GP2X_SELECT;
        if (joy_axes[1][JOYUD] == UP)           val |= GP2X_UP;
        if (joy_axes[1][JOYUD] == DOWN)         val |= GP2X_DOWN;
        if (joy_axes[1][JOYLR] == LEFT)         val |= GP2X_LEFT;
        if (joy_axes[1][JOYLR] == RIGHT)        val |= GP2X_RIGHT;
    }
    else if (which1 == 2) {
        if (joy_buttons[2][pi_joy[L_3]])        val |= GP2X_L;
        if (joy_buttons[2][pi_joy[R_3]])        val |= GP2X_R;
        if (joy_buttons[2][pi_joy[X_3]])        val |= GP2X_X;
        if (joy_buttons[2][pi_joy[Y_3]])        val |= GP2X_Y;
        if (joy_buttons[2][pi_joy[B_3]])        val |= GP2X_B;
        if (joy_buttons[2][pi_joy[A_3]])        val |= GP2X_A;
        if (joy_buttons[2][pi_joy[START_3]])    val |= GP2X_START;
        if (joy_buttons[2][pi_joy[SELECT_3]])   val |= GP2X_SELECT;
        if (joy_axes[2][JOYUD] == UP)           val |= GP2X_UP;
        if (joy_axes[2][JOYUD] == DOWN)         val |= GP2X_DOWN;
        if (joy_axes[2][JOYLR] == LEFT)         val |= GP2X_LEFT;
        if (joy_axes[2][JOYLR] == RIGHT)        val |= GP2X_RIGHT;
    }
    else if (which1 == 3) {
        if (joy_buttons[3][pi_joy[L_4]])        val |= GP2X_L;
        if (joy_buttons[3][pi_joy[R_4]])        val |= GP2X_R;
        if (joy_buttons[3][pi_joy[X_4]])        val |= GP2X_X;
        if (joy_buttons[3][pi_joy[Y_4]])        val |= GP2X_Y;
        if (joy_buttons[3][pi_joy[B_4]])        val |= GP2X_B;
        if (joy_buttons[3][pi_joy[A_4]])        val |= GP2X_A;
        if (joy_buttons[3][pi_joy[START_4]])    val |= GP2X_START;
        if (joy_buttons[3][pi_joy[SELECT_4]])   val |= GP2X_SELECT;
        if (joy_axes[3][JOYUD] == UP)           val |= GP2X_UP;
        if (joy_axes[3][JOYUD] == DOWN)         val |= GP2X_DOWN;
        if (joy_axes[3][JOYLR] == LEFT)         val |= GP2X_LEFT;
        if (joy_axes[3][JOYLR] == RIGHT)        val |= GP2X_RIGHT;
    }
    
    if(sdl_keys)
    {
        if(which1 == 0) {
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
            if (sdl_keys[pi_key[QUIT]] == SDL_PRESSED) GameLooping = 0;
        } else {
            if (sdl_keys[pi_key[L_2]] == SDL_PRESSED)       val |= GP2X_L;
            if (sdl_keys[pi_key[R_2]] == SDL_PRESSED)       val |= GP2X_R;
            if (sdl_keys[pi_key[X_2]] == SDL_PRESSED)       val |= GP2X_X;
            if (sdl_keys[pi_key[Y_2]] == SDL_PRESSED)       val |= GP2X_Y;
            if (sdl_keys[pi_key[B_2]] == SDL_PRESSED)       val |= GP2X_B;
            if (sdl_keys[pi_key[A_2]] == SDL_PRESSED)       val |= GP2X_A;
            if (sdl_keys[pi_key[START_2]] == SDL_PRESSED)   val |= GP2X_START;
            if (sdl_keys[pi_key[SELECT_2]] == SDL_PRESSED)  val |= GP2X_SELECT;
            if (sdl_keys[pi_key[UP_2]] == SDL_PRESSED)      val |= GP2X_UP;
            if (sdl_keys[pi_key[DOWN_2]] == SDL_PRESSED)    val |= GP2X_DOWN;
            if (sdl_keys[pi_key[LEFT_2]] == SDL_PRESSED)    val |= GP2X_LEFT;
            if (sdl_keys[pi_key[RIGHT_2]] == SDL_PRESSED)   val |= GP2X_RIGHT;
        }

    }
    
    return(val);
}

//sq Historical GP2X function to replace malloc,
// saves having to rename all the function calls
void *UpperMalloc(size_t size)
{
    return (void*)calloc(1, size);
}

//sq Historical GP2X function to replace malloc,
// saves having to rename all the function calls
void UpperFree(void* mem)
{
    free(mem);
}
