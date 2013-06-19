#include "gp2xsdk.h"
#include "gp2xmemfuncs.h"

#define BLOCKSIZE 1024
#define SetTaken(Start, Size) TakenSize[(Start - 0x2000000) / BLOCKSIZE] = (Size - 1) / BLOCKSIZE + 1

static int mem_fd = -1;
void *UpperMem;
int TakenSize[0x2000000 / BLOCKSIZE];
unsigned short *VideoBuffer = NULL;
static int screen_mode = 0;
volatile static unsigned short *gp2xregs = NULL;
unsigned long gp2x_physvram[4]={0,0,0,0};
unsigned short *framebuffer[4]={0,0,0,0};
static int currentframebuffer = 0;
struct usbjoy *joys[4];
char joyCount = 0;

void Disable_940(void)
{
	gp2xregs[0x3B42>>1];
	gp2xregs[0x3B42>>1]=0;
	gp2xregs[0x3B46>>1]=0xffff;	
	gp2xregs[0x3B48>>1]|= (1 << 7);
	gp2xregs[0x904>>1]&=0xfffe;
}

void InitMemPool() {
  //Try to apply MMU hack.
  UpperMem = mmap(0, 0x2000000, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0x2000000);
  gp2x_memset(TakenSize, 0, sizeof(TakenSize));

  //SetTaken(0x3000000, 0x80000); // Video decoder (you could overwrite this, but if you
                                // don't need the memory then be nice and don't)
  //SetTaken(0x3101000, 153600);  // Primary frame buffer
  //SetTaken(0x3381000, 153600);  // Secondary frame buffer (if you don't use it, uncomment)
  SetTaken(0x3600000, 0x80000);  // Sound buffer
  SetTaken(0x3F2E000, 0xD4000);  // Video Buffers
}

void DestroyMemPool() {
  munmap (UpperMem,0x2000000);
  UpperMem = NULL;
}

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

	mem_fd = open("/dev/mem", O_RDWR | O_SYNC);

	framebuffer[0]=(unsigned short *)mmap(0, 0x35000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, (gp2x_physvram[0]=0x04000000-(0x35000*4) )); 
	framebuffer[1]=(unsigned short *)mmap(0, 0x35000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, (gp2x_physvram[1]=0x04000000-(0x35000*3) )); 
	framebuffer[2]=(unsigned short *)mmap(0, 0x35000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, (gp2x_physvram[2]=0x04000000-(0x35000*2) )); 
	framebuffer[3]=(unsigned short *)mmap(0, 0x35000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, (gp2x_physvram[3]=0x04000000-(0x35000) )); 
	InitMemPool();
	gp2xregs = (unsigned short *)mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0xC0000000);
	if(gp2xregs)
	{
		Disable_940();
		int tMRD = 0, tRFC = 0, tRP = 1, tRCD = 1;
		int tRC = 6, tRAS = 3, tWR = 0;
		gp2xregs[0x28e2>>1] = 0;
		gp2xregs[0x28e6>>1] = 0;
		if(gp2xregs[0x2816>>1] == 319) { // LCD
			screen_mode = 0;
			gp2xregs[0x28e4>>1] = 319;
			gp2xregs[0x28e8>>1] = 239;
		} else { // TV
			gp2xregs[0x28e4>>1] = 719;
			if (gp2xregs[0x2818>>1]  == 287) { // PAL
				screen_mode = 1;
				gp2xregs[0x28e8>>1] = 287;
			} else if (gp2xregs[0x2818>>1]  == 239) { // NTSC
				screen_mode = 2;
				gp2xregs[0x28e8>>1] = 239;
			}
		}

		gp2xregs[0x3802>>1] = ((tMRD & 0xF) << 12) | ((tRFC & 0xF) << 8) | ((tRP & 0xF) << 4) | (tRCD & 0xF);
		gp2xregs[0x3804>>1] = /*0x9000 |*/ ((tRC & 0xF) << 8) | ((tRAS & 0xF) << 4) | (tWR & 0xF);

		if(mem_fd < 0) {
			fprintf (stderr, "Couldn't open /dev/mem device.\n");
			exit (1);
		} else {
			int mmu_fd;
			mmu_fd = open("/dev/mmuhack", O_RDWR);
			if(mmu_fd < 0) {
				system("/sbin/insmod -f mmuhack.o");
				mmu_fd = open("/dev/mmuhack", O_RDWR);
			}
			if(mmu_fd >= 0) close(mmu_fd);
		}
		gp2x_setvideo_mode(320,240);
		gp2x_clear_framebuffers();
		gp2x_video_flip();
	}
}

void gp2x_terminate(char *frontend)
{
struct stat info;

	for (int i=0; i<joyCount; i++)
	{
		joy_close(joys[i]);
	}
	DestroyMemPool();
	gp2x_setvideo_mode(320,240);
	munmap(framebuffer[0],0x40000);
	munmap(framebuffer[1],0x40000);
	if(gp2xregs) munmap((void *)gp2xregs, 0x10000);
	if(mem_fd >= 0) close(mem_fd);
	system("/sbin/rmmod mmuhack");
	
	if( (lstat(frontend, &info) == 0) && S_ISREG(info.st_mode) )
	{
	char path[256];
	char *p;
		strcpy(path, frontend);
		p = strrchr(path, '/');
		if(p == NULL) p = strrchr(path, '\\');
		if(p != NULL)
		{
			*p = '\0';
			chdir(path);
		}
		execl(frontend, frontend, NULL);
	}
	else
	{
		chdir("/usr/gp2x");
		execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
	}

}

void gp2x_setvideo_mode(int hw, int vh)
{
int hsc, vsc;

	hw *= 2;
	switch(screen_mode) {
		case 1: // TV PAL
			hsc = (float)(489.0 * hw) / 640;
			vsc = (float)(vh * (hw * 274.0)/320) / 240;
			break;
		case 2: // TV NTSC
			hsc = (float)(489.0 * hw) / 640;
			vsc = (float)(vh * (hw * 331.0)/320) / 240;
			break;
		default: // LCD
			hsc = (1024 * hw) / 640;
			vsc = (vh * hw) / 240;
	}
	gp2xregs[0x2906>>1] = hsc & 0xFFFF;
	gp2xregs[0x2908>>1] = vsc & 0xFFFF;
	gp2xregs[0x290A>>1] = vsc >> 16;
	gp2xregs[0x290C>>1] = hw;
}

unsigned long gp2x_joystick_read(void)
{
  unsigned long value=(gp2xregs[0x1198>>1] & 0x00FF);

  if(value==0xFD) value=0xFA;
  if(value==0xF7) value=0xEB;
  if(value==0xDF) value=0xAF;
  if(value==0x7F) value=0xBE;

  return ~((gp2xregs[0x1184>>1] & 0xFF00) | value | (gp2xregs[0x1186>>1] << 16));
}

void gp2x_video_flip()
{
unsigned int address;

	address=(unsigned int)gp2x_physvram[currentframebuffer];
	gp2xregs[0x290E>>1]=(unsigned short)(address & 0xffff);
	gp2xregs[0x2910>>1]=(unsigned short)(address >> 16);
	gp2xregs[0x2912>>1]=(unsigned short)(address & 0xffff);
	gp2xregs[0x2914>>1]=(unsigned short)(address >> 16);
	currentframebuffer = ++currentframebuffer % 4;
	VideoBuffer = framebuffer[currentframebuffer];
}

void gp2x_clear_framebuffers()
{
	gp2x_memset(framebuffer[0],0,0x35000);
	gp2x_memset(framebuffer[1],0,0x35000);
	gp2x_memset(framebuffer[2],0,0x35000);
	gp2x_memset(framebuffer[3],0,0x35000);
}

void * UpperMalloc(size_t size)
{
  int i = 0;
ReDo:
  for (; TakenSize[i]; i += TakenSize[i]);
  if (i >= 0x2000000 / BLOCKSIZE) {
    printf("UpperMalloc out of mem!");
    return NULL;
  }
  int BSize = (size - 1) / BLOCKSIZE + 1;
  for(int j = 1; j < BSize; j++) {
    if (TakenSize[i + j]) {
      i += j;
      goto ReDo; //OMG Goto, kill me.
    }
  }
  
  TakenSize[i] = BSize;
  void* mem = ((char*)UpperMem) + i * BLOCKSIZE;
  gp2x_memset(mem, 0, size);
  return mem;
}

//Releases UpperMalloced memory
void UpperFree(void* mem)
{
  int i = (((int)mem) - ((int)UpperMem));
  if (i < 0 || i >= 0x2000000) {
    fprintf(stderr, "UpperFree of not UpperMalloced mem: %p\n", mem);
  } else {
    if (i % BLOCKSIZE)
      fprintf(stderr, "delete error: %p\n", mem);
    TakenSize[i / BLOCKSIZE] = 0;
  }
}

//Returns the size of a UpperMalloced block.
int GetUpperSize(void* mem)
{
  int i = (((int)mem) - ((int)UpperMem));
  if (i < 0 || i >= 0x2000000) {
    fprintf(stderr, "GetUpperSize of not UpperMalloced mem: %p\n", mem);
    return -1;
  }
  return TakenSize[i / BLOCKSIZE] * BLOCKSIZE;
}
