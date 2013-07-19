// Irem M92 system games:
// 
// port from MAME by OopsWare
//

#include "burnint.h"
#include "vez.h"

static unsigned char *Mem = NULL, *MemEnd = NULL;
static unsigned char *RamStart, *RamEnd;

static unsigned char *RomV33;
static unsigned char *RomV30;
static unsigned char *RomGfx01;
static unsigned char *RomGfx02;

static unsigned char *RamVideo;
static unsigned char *RamV33;
static unsigned char *RamSpr;
static unsigned char *RamSprCpy;
static unsigned char *RamPal;

static unsigned short *RamCurPal;

static unsigned char DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy3[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy4[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvInput[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static unsigned char bRecalcPalette = 0;
static unsigned char DrvReset = 0;

static int m92_irq_vectorbase;
static unsigned int PalBank;

static unsigned char pf1_control[8],pf2_control[8],pf3_control[8],pf4_control[8];
static int pf1_enable,pf2_enable,pf3_enable;
static int pf1_shape,pf2_shape,pf3_shape;
static int pf1_rowscroll,pf2_rowscroll,pf3_rowscroll;
static int pf1_vram_ptr,pf2_vram_ptr,pf3_vram_ptr;
static int m92_sprite_list;

static unsigned char m92_sprite_buffer_busy;
static unsigned char m92_sprite_buffer_timer;

inline static unsigned int CalcCol(int offs)
{
	// xBBBBBGGGGGRRRRR
	int nColour = RamPal[offs + 0] | (RamPal[offs + 1] << 8);
	int r, g, b;

	r = (nColour & 0x001F) << 3;	// Red
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;	// Green
	g |= g >> 5;
	b = (nColour & 0x7C00) >> 7;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

static struct BurnInputInfo hookInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 2,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 0,	"p1 start"},
	
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 2"},	
	
	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 1,	"p2 start"},
	
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 2"},	

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy3 + 5,	"p3 coin"},
	{"P3 Start",	BIT_DIGITAL,	DrvJoy3 + 4,	"p3 start"},
	
	{"P3 Up",		BIT_DIGITAL,	DrvJoy3 + 3,	"p3 up"},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy3 + 2,	"p3 down"},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy3 + 1,	"p3 left"},
	{"P3 Right",	BIT_DIGITAL,	DrvJoy3 + 0,	"p3 right"},
	{"P3 Button 1",	BIT_DIGITAL,	DrvJoy3 + 7,	"p3 fire 1"},
	{"P3 Button 2",	BIT_DIGITAL,	DrvJoy3 + 6,	"p3 fire 2"},	
	
	{"P4 Coin",		BIT_DIGITAL,	DrvJoy4 + 5,	"p4 coin"},
	{"P4 Start",	BIT_DIGITAL,	DrvJoy4 + 4,	"p4 start"},
	
	{"P4 Up",		BIT_DIGITAL,	DrvJoy4 + 3,	"p4 up"},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy4 + 2,	"p4 down"},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy4 + 1,	"p4 left"},
	{"P4 Right",	BIT_DIGITAL,	DrvJoy4 + 0,	"p4 right"},
	{"P4 Button 1",	BIT_DIGITAL,	DrvJoy4 + 7,	"p4 fire 1"},
	{"P4 Button 2",	BIT_DIGITAL,	DrvJoy4 + 6,	"p4 fire 2"},	
	
	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
};

STDINPUTINFO(hook);

// Rom information
static struct BurnRomInfo hookRomDesc[] = {
	{ "h-h0-d.3h",		0x040000, 0x40189ff6, BRF_ESS | BRF_PRG },	// CPU 0, V33
	{ "h-l0-d.3h",		0x040000, 0x14567690, BRF_ESS | BRF_PRG }, 
	{ "h-h1.rom",		0x020000, 0x264ba1f0, BRF_ESS | BRF_PRG },
	{ "h-l1.rom",		0x020000, 0xf9913731, BRF_ESS | BRF_PRG }, 

	{ "h-sh0.rom",		0x010000, 0x86a4e56e, BRF_ESS | BRF_PRG },	// CPU 1, V30
	{ "h-sl0.rom",		0x010000, 0x10fd9676, BRF_ESS | BRF_PRG }, 
	
	{ "hook-c0.rom",	0x040000, 0xdec63dcf, BRF_GRA }, 			// Tiles
	{ "hook-c1.rom",	0x040000, 0xe4eb0b92, BRF_GRA },
	{ "hook-c2.rom",	0x040000, 0xa52b320b, BRF_GRA },
	{ "hook-c3.rom",	0x040000, 0x7ef67731, BRF_GRA },
	
	{ "hook-000.rom",	0x100000, 0xccceac30, BRF_GRA },        	// Sprites
	{ "hook-010.rom",	0x100000, 0x8ac8da67, BRF_GRA }, 
	{ "hook-020.rom",	0x100000, 0x8847af9a, BRF_GRA }, 
	{ "hook-030.rom",	0x100000, 0x239e877e, BRF_GRA }, 

	{ "hook-da.rom",	0x080000, 0x88cd0212, BRF_SND }, 			// Sound
};

STD_ROM_PICK(hook);
STD_ROM_FN(hook);

static int MemIndex()
{
	unsigned char *Next; Next = Mem;
	RomV33 		= Next; Next += 0x0C0000;			// V33
	RomV30		= Next; Next += 0x100000;			// V30
	RomGfx01	= Next; Next += 0x200000;			// char
	RomGfx02	= Next; Next += 0x800000;			// spr
	//MSM6295ROM	= Next; Next += 0x080000;
	
	RamStart	= Next;
	RamVideo	= Next; Next += 0x010000;
	RamV33		= Next; Next += 0x010000;
	RamSpr		= Next; Next += 0x000800;
	RamSprCpy	= Next; Next += 0x000800;
	RamPal		= Next; Next += 0x001000;			// 2 bank of 0x0800
	//RamBg00		= (unsigned short *) Next; Next += 0x004000;
	//RamBg01		= (unsigned short *) Next; Next += 0x001000;
	//RamSpr		= (unsigned short *) Next; Next += 0x010000;
	
//	RamZ80		= Next; Next += 0x001800;

	RamEnd		= Next;
	
	RamCurPal	= (unsigned short *) Next; Next += 0x001000;
	
	MemEnd		= Next;
	return 0;
}

unsigned char __fastcall m92ReadByte(unsigned int vezAddress)
{
	// Palette Read
	if ((vezAddress & 0xFF800) == 0xF8800 )
		return RamPal[ vezAddress - 0xF8800 + PalBank ];
	
	// V33 Start vector
	if ((vezAddress & 0xFFFF0) == 0xFFFF0 )
		return RomV33[ vezAddress - 0xFFFF0 + 0x7FFF0 ];
	
	switch (vezAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), vezAddress);
	}
	return 0;
}

void __fastcall m92WriteByte(unsigned int vezAddress, unsigned char byteValue)
{
	static unsigned int sprite_extent;
	
	if ((vezAddress & 0xFF800) == 0xF8800 ) {
		RamPal[ vezAddress - 0xF8800 + PalBank ] = byteValue;
		if (vezAddress & 1) {
			int offs = (vezAddress - 0xF8800 + PalBank) >> 1;
			RamCurPal[ offs ] = CalcCol( offs << 1 );
		}
		return;
	}
		
	switch (vezAddress) {
		// 0xf9000 ~ 0xf900f : m92_spritecontrol_w
		case 0xF9000:
			sprite_extent = byteValue;
			break;
		case 0xF9004:
			if (byteValue==8)
				m92_sprite_list=(((0x100 - sprite_extent)&0xff)*8);
			else
				m92_sprite_list=0x800;
			break;
		case 0xF9008:
			/* Pixel clock is 26.6666 MHz, we have 0x800 bytes, or 0x400 words
	           to copy from spriteram to the buffer.  It seems safe to assume 1
	           word can be copied per clock.  So:
	
	           1 MHz clock would be 1 word every 0.000,001s = 1000ns
	           26.6666MHz clock would be 1 word every 0.000,000,037 = 37 ns
	           Buffer should copy in about 37888 ns. */

			
			//m92_sprite_buffer_busy = 0;
			//m92_sprite_buffer_timer = 1;
			memcpy(RamSprCpy, RamSpr, 0x800);
			
			break;

		case 0xF9800:
			PalBank = (byteValue & 0x2 /* && m92_game_kludge!=3 */) ? 0x0800 : 0x0000;
			break;

		case 0xF9001:
		case 0xF9002:
		case 0xF9003:
		case 0xF9005:
		case 0xF9006:
		case 0xF9007:
		case 0xF9009:
		case 0xF900a:
		case 0xF900b:
		case 0xF9801:
			//if (!byteValue) 
			break;
		default:
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, vezAddress);
	}
}

unsigned char __fastcall m92ReadPort(unsigned int vezPort)
{
	switch (vezPort) {
		case 0x00: return ~DrvInput[0];	// player 1
		case 0x01: return ~DrvInput[1];	// player 2
		case 0x03:
		case 0x04:
		case 0x05:
			return 0xFF;
		case 0x06: return ~DrvInput[2];	// player 3
		case 0x07: return ~DrvInput[3];	// player 4
		case 0x88:
			return 0xFF;
		case 0x02:
			return (~DrvInput[4] & 0x7F) | m92_sprite_buffer_busy;
		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of port %x\n"), vezPort);
	}
	return 0;
}

void __fastcall m92WritePort(unsigned int vezPort, unsigned char byteValue)
{
	switch (vezPort) {
		case 0x00:
		case 0x01:
			//m92_soundlatch_w
			
			break;
		case 0x02:
		case 0x03:
			//m92_coincounter_w
			
			break;
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
			// Interrupt controller, only written to at bootup
			break;
		// 0x80 ~ 0x87: m92_pf1_control_w
		case 0x80: pf1_control[0] = byteValue; break;
		case 0x81: pf1_control[1] = byteValue; break;
		case 0x82: pf1_control[2] = byteValue; break;
		case 0x83: pf1_control[3] = byteValue; break;
		case 0x84: pf1_control[4] = byteValue; break;
		case 0x85: pf1_control[5] = byteValue; break;
		case 0x86: pf1_control[6] = byteValue; break;
		case 0x87: pf1_control[7] = byteValue; break;
		// 0x88 ~ 0x8f: m92_pf2_control_w
		case 0x88: pf2_control[0] = byteValue; break;
		case 0x89: pf2_control[1] = byteValue; break;
		case 0x8a: pf2_control[2] = byteValue; break;
		case 0x8b: pf2_control[3] = byteValue; break;
		case 0x8c: pf2_control[4] = byteValue; break;
		case 0x8d: pf2_control[5] = byteValue; break;
		case 0x8e: pf2_control[6] = byteValue; break;
		case 0x8f: pf2_control[7] = byteValue; break;
		// 0x90 ~ 0x97: m92_pf3_control_w
		case 0x90: pf3_control[0] = byteValue; break;
		case 0x91: pf3_control[1] = byteValue; break;
		case 0x92: pf3_control[2] = byteValue; break;
		case 0x93: pf3_control[3] = byteValue; break;
		case 0x94: pf3_control[4] = byteValue; break;
		case 0x95: pf3_control[5] = byteValue; break;
		case 0x96: pf3_control[6] = byteValue; break;
		case 0x97: pf3_control[7] = byteValue; break;
		// 0x98 ~ 0x9f: m92_master_control_w 
		case 0x98: 
			pf4_control[0] = byteValue; 
			pf1_enable = (byteValue & 0x10) ? 0 : 1;
			pf1_rowscroll = (byteValue & 0x40) >> 6;
			pf1_shape = (byteValue & 0x04) >> 2;
			pf1_vram_ptr = 0x4000 * (byteValue & 0x03);
			
			break;
		case 0x99: 
			pf4_control[1] = byteValue; 
			break;
		case 0x9a: 
			pf4_control[2] = byteValue; 
			pf2_enable = (byteValue & 0x10) ? 0 : 1;
			pf2_rowscroll = (byteValue & 0x40) >> 6;
			pf2_shape = (byteValue & 0x04) >> 2;
			pf2_vram_ptr = 0x4000 * (byteValue & 0x03);
			
			break;
		case 0x9b: 
			pf4_control[3] = byteValue; 
			pf3_enable = (byteValue & 0x10) ? 0 : 1;
			pf3_rowscroll = (byteValue & 0x40) >> 6;
			pf3_shape = (byteValue & 0x04) >> 2;
			pf3_vram_ptr = 0x4000 * (byteValue & 0x03);
			
			break;
		case 0x9c: pf4_control[4] = byteValue; break;
		case 0x9d: pf4_control[5] = byteValue; break;
		case 0x9e: pf4_control[6] = byteValue; break;
		case 0x9f: pf4_control[7] = byteValue; break;
		
		default:
			
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to port %x\n"), byteValue, vezPort);
	}
}

static int DrvDoReset()
{
	
	VezOpen(0);
	VezReset();
	VezClose();
	
	pf1_enable = pf2_enable = pf3_enable = 0;
	pf1_shape = pf2_shape = pf3_shape = 0;
	pf1_rowscroll = pf2_rowscroll = pf3_rowscroll = 0;
	pf1_vram_ptr = pf2_vram_ptr = pf3_vram_ptr = 0;
	
	m92_sprite_buffer_busy = 0x80;
	m92_sprite_buffer_timer = 0;
	
	return 0;
}

static int loadDecodeGfx01(unsigned char *tmp, int rid, int shift, int size)
{
	unsigned char * pgfx = RomGfx01;
	
	BurnLoadRom(tmp, rid, 1);
	
	for (int i=0; i<(size/8); i++) {
		for( int y=0;y<8;y++) {
			pgfx[0] |= ((tmp[0]>>7)&1)<<shift;
			pgfx[1] |= ((tmp[0]>>6)&1)<<shift;
			pgfx[2] |= ((tmp[0]>>5)&1)<<shift;
			pgfx[3] |= ((tmp[0]>>4)&1)<<shift;
			pgfx[4] |= ((tmp[0]>>3)&1)<<shift;
			pgfx[5] |= ((tmp[0]>>2)&1)<<shift;
			pgfx[6] |= ((tmp[0]>>1)&1)<<shift;
			pgfx[7] |= ((tmp[0]>>0)&1)<<shift;
			tmp ++;
			pgfx += 8;
		}
	}
	return 0;
}

static int loadDecodeGfx02(unsigned char *tmp, int rid, int shift, int size)
{
	unsigned char * pgfx = RomGfx02;
	
	BurnLoadRom(tmp, rid, 1);
	
	for (int i=0; i<(size/32); i++) {
		for( int y=0;y<16;y++) {
			pgfx[ 0] |= ((tmp[ 0]>>7)&1)<<shift;
			pgfx[ 1] |= ((tmp[ 0]>>6)&1)<<shift;
			pgfx[ 2] |= ((tmp[ 0]>>5)&1)<<shift;
			pgfx[ 3] |= ((tmp[ 0]>>4)&1)<<shift;
			pgfx[ 4] |= ((tmp[ 0]>>3)&1)<<shift;
			pgfx[ 5] |= ((tmp[ 0]>>2)&1)<<shift;
			pgfx[ 6] |= ((tmp[ 0]>>1)&1)<<shift;
			pgfx[ 7] |= ((tmp[ 0]>>0)&1)<<shift;
			
			pgfx[ 8] |= ((tmp[16]>>7)&1)<<shift;
			pgfx[ 9] |= ((tmp[16]>>6)&1)<<shift;
			pgfx[10] |= ((tmp[16]>>5)&1)<<shift;
			pgfx[11] |= ((tmp[16]>>4)&1)<<shift;
			pgfx[12] |= ((tmp[16]>>3)&1)<<shift;
			pgfx[13] |= ((tmp[16]>>2)&1)<<shift;
			pgfx[14] |= ((tmp[16]>>1)&1)<<shift;
			pgfx[15] |= ((tmp[16]>>0)&1)<<shift;
			
			tmp ++;
			pgfx += 16;
		}
		tmp += 16;
	}
	return 0;
}

static int hookInit()
{
	int nRet;
	
	Mem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();	
	
	nRet = BurnLoadRom(RomV33 + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV33 + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV33 + 0x080001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV33 + 0x080000, 3, 2); if (nRet != 0) return 1;

	nRet = BurnLoadRom(RomV30 + 0x000001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV30 + 0x000000, 5, 2); if (nRet != 0) return 1;
	
	// load and decode tile
	unsigned char *tmp = (unsigned char *) malloc (0x100000);
	if ( tmp == 0 ) return 1;
	
	loadDecodeGfx01(tmp,  6, 0, 0x040000);
	loadDecodeGfx01(tmp,  7, 1, 0x040000);
	loadDecodeGfx01(tmp,  8, 2, 0x040000);
	loadDecodeGfx01(tmp,  9, 3, 0x040000);
	
	loadDecodeGfx02(tmp, 10, 0, 0x100000);
	loadDecodeGfx02(tmp, 11, 1, 0x100000);
	loadDecodeGfx02(tmp, 12, 2, 0x100000);
	loadDecodeGfx02(tmp, 13, 3, 0x100000);
	
	free(tmp);



	{
		unsigned int cpu_types[] = { 0, 8 };
		VezInit(2, &cpu_types[0]);
	    
	    VezOpen(0);

		VezMapArea(0x00000, 0x9ffff, 0, RomV33 + 0x00000);	// CPU 0 ROM
//		memcpy(RomV33+0xffff0, RomV33+0x7fff0, 0x10);		// V33 Start vector
		
		VezMapArea(0xa0000, 0xbffff, 0, RomV33 + 0xa0000);	// rom bank
		
		VezMapArea(0xd0000, 0xdffff, 0, RamVideo);
		VezMapArea(0xd0000, 0xdffff, 1, RamVideo);
		
		VezMapArea(0xe0000, 0xeffff, 0, RamV33);			// system ram
		VezMapArea(0xe0000, 0xeffff, 1, RamV33);

		VezMapArea(0xf8000, 0xf87ff, 0, RamSpr);			// sprites ram
		VezMapArea(0xf8000, 0xf87ff, 1, RamSpr);

		//VezMapArea(0xf8800, 0xf8fff, 0, RamPal);			// palette
		//VezMapArea(0xf8800, 0xf8fff, 1, RamPal);
		
		VezSetReadHandler(m92ReadByte);
		VezSetWriteHandler(m92WriteByte);
		VezSetReadPort(m92ReadPort);
		VezSetWritePort(m92WritePort);
		
		VezClose();

	}
	
	m92_irq_vectorbase = 0x80;
	PalBank	= 0;

	DrvDoReset();
	return 0;
}

static int hookExit()
{
	VezExit();
	
	free(Mem);
	Mem = NULL;
	
	return 0;
}

#define	TILE_LAYER_1(xx)		\
	if (d[xx]) p[xx] = pal[ d[xx] | c ];

#define	TILE_LAYER_1_E(xx)		\
	if (d[xx] && (x + xx)>=0 && (x + xx)<320 ) p[xx] = pal[ d[xx] | c ];

#define	TILE_LAYER_1_LINE		\
	TILE_LAYER_1(0)				\
	TILE_LAYER_1(1)				\
	TILE_LAYER_1(2)				\
	TILE_LAYER_1(3)				\
	TILE_LAYER_1(4)				\
	TILE_LAYER_1(5)				\
	TILE_LAYER_1(6)				\
	TILE_LAYER_1(7)
	
#define	TILE_LAYER_1_LINE_E		\
	if ( (y+k)>=0 && (y+k)<240 ) {	\
		TILE_LAYER_1_E(0)		\
		TILE_LAYER_1_E(1)		\
		TILE_LAYER_1_E(2)		\
		TILE_LAYER_1_E(3)		\
		TILE_LAYER_1_E(4)		\
		TILE_LAYER_1_E(5)		\
		TILE_LAYER_1_E(6)		\
		TILE_LAYER_1_E(7)		\
	}

static void tileLayer_1()
{
	/* T--- -YX-  Pccc cccc  tttt tttt tttt tttt */
	unsigned short * pal = RamCurPal;
	int offs, mx, my, x, y;
	
	mx = -1;
	my = 0;
	for (offs = 0; offs < 64*64*4; offs+=4) {
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}

		x = mx * 8 - ((80 + (short)((pf1_control[5]<<8) | pf1_control[4])) & 0x1ff);
		if (x < -16) x += 512;
		
		y = my * 8 - ((136 + (short)((pf1_control[1]<<8) | pf1_control[0])) & 0x1ff);
		if (y < -16) y += 512;
		
		if ( x<=-8 || x>=320 || y<=-8 || y>= 240 ) 
			continue;

		int tile_index = offs + pf1_vram_ptr;
		unsigned int tileno = RamVideo[tile_index] | (RamVideo[tile_index+1] << 8) | ((RamVideo[tile_index+3] & 0x80) << 9);
		if (tileno == 0) continue;
			
		unsigned int c = (RamVideo[tile_index+2] & 0x7F) << 4;
		unsigned short * p = (unsigned short *) pBurnDraw + y * 320 + x;
		unsigned char * d = RomGfx01 + (tileno * 64);

		if ( x >=0 && x <= (320-8) && y >= 0 && y <= (240-8)) {
			
			//unsigned char * pp = RamPri + y * 320 + x;
					
			for (int k=0;k<8;k++) {
				
 				TILE_LAYER_1_LINE

 				d += 8;
 				p += 320;
 			}
		
		} else {

			for (int k=0;k<8;k++) {
				
 				TILE_LAYER_1_LINE_E

 				d += 8;
 				p += 320;
 			}

		
		}
	}
}


static void tileLayer_2()
{
	/* T--- -YX-  Pccc cccc  tttt tttt tttt tttt */
	unsigned short * pal = RamCurPal;
	int offs, mx, my, x, y;
	
	mx = -1;
	my = 0;
	for (offs = 0; offs < 64*64*4; offs+=4) {
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}

		x = mx * 8 - ((80 - 2 + (short)((pf2_control[5]<<8) | pf2_control[4])) & 0x1ff);
		if (x < -16) x += 512;
		
		y = my * 8 - ((136 + (short)((pf2_control[1]<<8) | pf2_control[0])) & 0x1ff);
		if (y < -16) y += 512;
		
		if ( x<=-8 || x>=320 || y<=-8 || y>= 240 ) 
			continue;
		
		int tile_index = offs + pf2_vram_ptr;

		unsigned int tileno = RamVideo[tile_index] | (RamVideo[tile_index+1] << 8) | ((RamVideo[tile_index+3] & 0x80) << 9);
		if (tileno == 0) continue;
			
		unsigned int c = (RamVideo[tile_index+2] & 0x7F) << 4;
		unsigned short * p = (unsigned short *) pBurnDraw + y * 320 + x;
		unsigned char * d = RomGfx01 + (tileno * 64);
		//unsigned char * pp = RamPri + y * 320 + x;

		if ( x >=0 && x <= (320-8) && y >= 0 && y <= (240-8)) {
			
			for (int k=0;k<8;k++) {
				
 				TILE_LAYER_1_LINE

 				d += 8;
 				p += 320;
 			}
			
		} else {
			
			for (int k=0;k<8;k++) {
				
 				TILE_LAYER_1_LINE_E

 				d += 8;
 				p += 320;
 			}
			
		}
	}
}

#define	TILE_LAYER_3(xx)		\
	p[xx] = pal[ d[xx] | c ];

#define	TILE_LAYER_3_E(xx)		\
	if ((x + xx)>=0 && (x + xx)<320 ) p[xx] = pal[ d[xx] | c ];

#define	TILE_LAYER_3_LINE		\
	TILE_LAYER_3(0)				\
	TILE_LAYER_3(1)				\
	TILE_LAYER_3(2)				\
	TILE_LAYER_3(3)				\
	TILE_LAYER_3(4)				\
	TILE_LAYER_3(5)				\
	TILE_LAYER_3(6)				\
	TILE_LAYER_3(7)
	
#define	TILE_LAYER_3_LINE_E		\
	if ( (y+k)>=0 && (y+k)<240 ) {	\
		TILE_LAYER_3_E(0)		\
		TILE_LAYER_3_E(1)		\
		TILE_LAYER_3_E(2)		\
		TILE_LAYER_3_E(3)		\
		TILE_LAYER_3_E(4)		\
		TILE_LAYER_3_E(5)		\
		TILE_LAYER_3_E(6)		\
		TILE_LAYER_3_E(7)		\
	}

static void tileLayer_3()
{
	/* T--- -YX-  Pccc cccc  tttt tttt tttt tttt */
	unsigned short * pal = RamCurPal;
	int offs, mx, my, x, y;
	
	mx = -1;
	my = 0;
	for (offs = 0; offs < 64*64*4; offs+=4) {
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}

		x = mx * 8 - ((80 - 4 + (short)((pf3_control[5]<<8) | pf3_control[4])) & 0x1ff);
		if (x < -16) x += 512;
		
		y = my * 8 - ((136 + (short)((pf3_control[1]<<8) | pf3_control[0])) & 0x1ff);
		if (y < -16) y += 512;
		
		if ( x<=-8 || x>=320 || y<=-8 || y>= 240 ) 
			continue;
		
		/* pf3_vram_ptr NOT been set, why ????!!!!	*/
		//int tile_index = offs + pf3_vram_ptr;
		int tile_index = offs + 0x8000;

		unsigned int tileno = RamVideo[tile_index] | (RamVideo[tile_index+1] << 8) | ((RamVideo[tile_index+3] & 0x80) << 9);
		if (tileno == 0) continue;
			
		unsigned int c = (RamVideo[tile_index+2] & 0x7F) << 4;
		unsigned short * p = (unsigned short *) pBurnDraw + y * 320 + x;
		unsigned char * d = RomGfx01 + (tileno * 64);
		//unsigned char * pp = RamPri + y * 320 + x;

		if ( x >=0 && x <= (320-8) && y >= 0 && y <= (240-8)) {
			
			for (int k=0;k<8;k++) {
				
 				TILE_LAYER_3_LINE

 				d += 8;
 				p += 320;
 			}
			
		} else {
			
			for (int k=0;k<8;k++) {
				
 				TILE_LAYER_3_LINE_E

 				d += 8;
 				p += 320;
 			}
			
		}
	}
}

#define TILE_SPR_NORMAL(x)				\
	if (q[x]) p[x]=pal[q[x]|color];

#define TILE_SPR_FLIP_X(x)				\
	if (q[x]) p[15-x]=pal[q[x]|color];

#define TILE_SPR_NORMAL_E(x)			\
	if (q[x]&&((sx+x)>=0)&&((sx+x)<320)) p[x]=pal[q[x]|color];

#define TILE_SPR_FLIP_X_E(x)			\
	if (q[x]&&((sx+15-x)>=0)&&((sx+15-x)<320)) p[15-x]=pal[q[x]|color];

#define TILE_SPR_NORMAL_LINE			\
	TILE_SPR_NORMAL( 0)					\
	TILE_SPR_NORMAL( 1)					\
	TILE_SPR_NORMAL( 2)					\
	TILE_SPR_NORMAL( 3)					\
	TILE_SPR_NORMAL( 4)					\
	TILE_SPR_NORMAL( 5)					\
	TILE_SPR_NORMAL( 6)					\
	TILE_SPR_NORMAL( 7)					\
	TILE_SPR_NORMAL( 8)					\
	TILE_SPR_NORMAL( 9)					\
	TILE_SPR_NORMAL(10)					\
	TILE_SPR_NORMAL(11)					\
	TILE_SPR_NORMAL(12)					\
	TILE_SPR_NORMAL(13)					\
	TILE_SPR_NORMAL(14)					\
	TILE_SPR_NORMAL(15)

#define TILE_SPR_FLIP_X_LINE			\
	TILE_SPR_FLIP_X( 0)					\
	TILE_SPR_FLIP_X( 1)					\
	TILE_SPR_FLIP_X( 2)					\
	TILE_SPR_FLIP_X( 3)					\
	TILE_SPR_FLIP_X( 4)					\
	TILE_SPR_FLIP_X( 5)					\
	TILE_SPR_FLIP_X( 6)					\
	TILE_SPR_FLIP_X( 7)					\
	TILE_SPR_FLIP_X( 8)					\
	TILE_SPR_FLIP_X( 9)					\
	TILE_SPR_FLIP_X(10)					\
	TILE_SPR_FLIP_X(11)					\
	TILE_SPR_FLIP_X(12)					\
	TILE_SPR_FLIP_X(13)					\
	TILE_SPR_FLIP_X(14)					\
	TILE_SPR_FLIP_X(15)

#define TILE_SPR_NORMAL_LINE_E			\
	TILE_SPR_NORMAL_E( 0)				\
	TILE_SPR_NORMAL_E( 1)				\
	TILE_SPR_NORMAL_E( 2)				\
	TILE_SPR_NORMAL_E( 3)				\
	TILE_SPR_NORMAL_E( 4)				\
	TILE_SPR_NORMAL_E( 5)				\
	TILE_SPR_NORMAL_E( 6)				\
	TILE_SPR_NORMAL_E( 7)				\
	TILE_SPR_NORMAL_E( 8)				\
	TILE_SPR_NORMAL_E( 9)				\
	TILE_SPR_NORMAL_E(10)				\
	TILE_SPR_NORMAL_E(11)				\
	TILE_SPR_NORMAL_E(12)				\
	TILE_SPR_NORMAL_E(13)				\
	TILE_SPR_NORMAL_E(14)				\
	TILE_SPR_NORMAL_E(15)

#define TILE_SPR_FLIP_X_LINE_E			\
	TILE_SPR_FLIP_X_E( 0)				\
	TILE_SPR_FLIP_X_E( 1)				\
	TILE_SPR_FLIP_X_E( 2)				\
	TILE_SPR_FLIP_X_E( 3)				\
	TILE_SPR_FLIP_X_E( 4)				\
	TILE_SPR_FLIP_X_E( 5)				\
	TILE_SPR_FLIP_X_E( 6)				\
	TILE_SPR_FLIP_X_E( 7)				\
	TILE_SPR_FLIP_X_E( 8)				\
	TILE_SPR_FLIP_X_E( 9)				\
	TILE_SPR_FLIP_X_E(10)				\
	TILE_SPR_FLIP_X_E(11)				\
	TILE_SPR_FLIP_X_E(12)				\
	TILE_SPR_FLIP_X_E(13)				\
	TILE_SPR_FLIP_X_E(14)				\
	TILE_SPR_FLIP_X_E(15)

static void pdrawgfx(unsigned int code,unsigned int color,int flipx,int flipy,int sx,int sy,int pri)
{
	unsigned short * p	= (unsigned short *) pBurnDraw;
//	unsigned char * pp = RamPri;
	unsigned char * q	= RomGfx02 + (code << 8);
	unsigned short *pal	= RamCurPal;
	
	sx -= 80;
	sy -= 136;
	
	p += sy * 320 + sx;
//	pp += sy * 320 + sx;
		
	if (sx < 0 || sx >= (320-16) || sy < 0 || sy >= (240-16) ) {
		
		if ((sx <= -16) || (sx >= 320) || (sy <= -16) || (sy >= 240))
			return;
			
		if (flipy) {
		
			p += 320 * 15;
			//pp += 320 * 15;
			
			if (flipx) {
			
				for (int i=15;i>=0;i--) {
					if (((sy+i)>=0) && ((sy+i)<240)) {
						
						TILE_SPR_FLIP_X_LINE_E
						
					}
					p -= 320;
					//pp -= 320;
					q += 16;
				}	
			
			} else {
	
				for (int i=15;i>=0;i--) {
					if (((sy+i)>=0) && ((sy+i)<240)) {
						
						TILE_SPR_NORMAL_LINE_E

					}
					p -= 320;
					//pp -= 320;
					q += 16;
				}		
			}
			
		} else {
			
			if (flipx) {
			
				for (int i=0;i<16;i++) {
					if (((sy+i)>=0) && ((sy+i)<240)) {
						
						TILE_SPR_FLIP_X_LINE_E
						
					}
					p += 320;
					//pp += 320;
					q += 16;
				}		
			
			} else {
	
				for (int i=0;i<16;i++) {
					if (((sy+i)>=0) && ((sy+i)<240)) {

						TILE_SPR_NORMAL_LINE_E

					}
					p += 320;
					//pp += 320;
					q += 16;
				}	
	
			}
			
		}
		
		return;
	}

	if (flipy) {
		
		p += 320 * 15;
		//pp += 320 * 15;
		
		if (flipx) {
		
			for (int i=0;i<16;i++) {
				
				TILE_SPR_FLIP_X_LINE
	
				p -= 320;
				//pp -= 320;
				q += 16;
			}	
		
		} else {

			for (int i=0;i<16;i++) {
				
				TILE_SPR_NORMAL_LINE
	
				p -= 320;
				//pp -= 320;
				q += 16;
			}		
		}
		
	} else {
		
		if (flipx) {
		
			for (int i=0;i<16;i++) {
				
				TILE_SPR_FLIP_X_LINE
	
				p += 320;
				//pp += 320;
				q += 16;
			}		
		
		} else {

			for (int i=0;i<16;i++) {
				
				TILE_SPR_NORMAL_LINE
	
				p += 320;
				//pp += 320;
				q += 16;
			}	

		}
		
	}

}

static void drawSprites()
{
	int offs = m92_sprite_list - 8;
	while (offs >= 0) {
		int x,y,sprite,colour,fx,fy,x_multi,y_multi,i,j,s_ptr,pri;

		y=(RamSprCpy[offs+0] | (RamSprCpy[offs+1]<<8))&0x1ff;
		x=(RamSprCpy[offs+6] | (RamSprCpy[offs+7]<<8))&0x1ff;

		if ((RamSprCpy[offs+4]&0x80)==0x80) pri=0; else pri=2;

		x = x - 16;
		y = 512 - 16 - y;

	    sprite=(RamSprCpy[offs+2] | (RamSprCpy[offs+3]<<8));
		colour= (RamSprCpy[offs+4]&0x7f) << 4;

		fx=RamSprCpy[offs+5]&1;
		fy=(RamSprCpy[offs+5]&2)>>1;
		y_multi=(RamSprCpy[offs+1]>>1)&0x3;
		x_multi=(RamSprCpy[offs+1]>>3)&0x3;

		y_multi=1 << y_multi; /* 1, 2, 4 or 8 */
		x_multi=1 << x_multi; /* 1, 2, 4 or 8 */

		if (fx) x+=16 * (x_multi - 1);

		for (j=0; j<x_multi; j++) {
			s_ptr=8 * j;
			if (!fy) s_ptr+=y_multi-1;

			for (i=0; i<y_multi; i++) {

				pdrawgfx(sprite + s_ptr, colour, fx, fy, x, y-i*16 ,pri);

				if (fy) s_ptr++; else s_ptr--;
			}
			if (fx) x-=16; else x+=16;
			
			offs-=8;
		}
	}
}

static void DrvDraw()
{
	memset(pBurnDraw, 0, 320 * 240 * 2);

	if (pf3_enable)
		tileLayer_3();

	if (pf2_enable)
		tileLayer_2();
	
	drawSprites();
	
	if (pf1_enable)
		tileLayer_1();

/*
	bprintf(PRINT_NORMAL, _T("E:%d%d%d S:%d%d%d P:%d%d%d %04x %04x %04x R: %04x %04x, %04x %04x, %04x %04x\n"), 
		pf1_enable, pf2_enable, pf3_enable, 
		pf1_rowscroll, pf2_rowscroll, pf3_rowscroll,
		pf1_shape, pf2_shape, pf3_shape,
		pf1_vram_ptr, pf2_vram_ptr, pf3_vram_ptr,
		(pf1_control[5]<<8)+pf1_control[4], (pf1_control[1]<<8) + pf1_control[0],
		(pf2_control[5]<<8)+pf2_control[4], (pf2_control[1]<<8) + pf2_control[0],
		(pf3_control[5]<<8)+pf3_control[4], (pf3_control[1]<<8) + pf3_control[0]  );

*/
}

static int hookFrame()
{
	if (DrvReset) DrvDoReset();
	
	DrvInput[0] = 0x00;
	DrvInput[1] = 0x00;
	DrvInput[2] = 0x00;
	DrvInput[3] = 0x00;
	DrvInput[4] = 0x00;
	
	for (int i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvJoy3[i] & 1) << i;
		DrvInput[3] |= (DrvJoy4[i] & 1) << i;
		DrvInput[4] |= (DrvButton[i] & 1) << i;
	}
	
	VezOpen(0);

	VezRun(9000000 / 60);
	
	if ( m92_sprite_buffer_timer ) {
		m92_sprite_buffer_timer --;
		
		if (m92_sprite_buffer_timer == 0) {
			m92_sprite_buffer_busy = 0x80;
			
			VezSetIRQLine(m92_irq_vectorbase + 4, VEZ_IRQSTATUS_ACK);
		}
	}
	
	VezSetIRQLine(m92_irq_vectorbase + 0, VEZ_IRQSTATUS_ACK);
	
	VezClose();

	if (pBurnDraw) DrvDraw();
	return 0;
}

static int hookScan(int nAction,int *pnMin)
{
	//struct BurnArea ba;

	if (pnMin)						// Return minimum compatible version
		*pnMin =  0x029671;
	
	return 0;
}


struct BurnDriver BurnDrvHook = {
	"hook", NULL, NULL, "1992",
	"Hook (World)\0", NULL, "Irem", "misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 2, HARDWARE_MISC_MISC,
	NULL, hookRomInfo, hookRomName, hookInputInfo, NULL, // hookDIPInfo,
	hookInit, hookExit, hookFrame, NULL, hookScan, &bRecalcPalette, 
	320, 240, 4, 3
};
