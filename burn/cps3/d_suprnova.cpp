#include <stdio.h>
#include <stdlib.h>

#include "burnint.h"
#include "sh2.h"

static unsigned char *Mem = NULL, *MemEnd = NULL;
static unsigned char *RamStart, *RamEnd;

static unsigned char *RomBios;
static unsigned char *RomGame;

static unsigned char *RamMain;
static unsigned char *RamCache;

static unsigned char *RamSpr;
static unsigned char *RamV3Reg;
static unsigned char *RamV3Tbl;
static unsigned char *RamV3Slc;
static unsigned char *RamV3t;

static unsigned char *RamPal;
//static unsigned char *CurPal;

static unsigned char DrvReset = 0;
static unsigned char bRecalcPalette = 0;

static struct BurnInputInfo galpani4InputList[] = {
	
	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},

};

STDINPUTINFO(galpani4);

static struct BurnRomInfo galpani4RomDesc[] = {
	{ "sknsj1.u10",	  0x080000, 0x7e2b836c, BRF_ESS | BRF_PRG },	// SH-2 Code
	 
	{ "gp4j1.u10",	  0x080000, 0x919a3893, BRF_ESS | BRF_PRG },	// SH-2 Code mapped at 0x04000000
	{ "gp4j1.u8",	  0x080000, 0x94cb1fb7, BRF_ESS | BRF_PRG }, 
	
//	{ "32j4-0.12", 	  0x200000, 0x1ebea5b6, BRF_GRA }, 				// gfx 2 sprite
//	{ "32j9-0.76",    0x080000, 0x16001e81, BRF_SND },				// PCM
};

STD_ROM_PICK(galpani4);
STD_ROM_FN(galpani4);



static int MemIndex()
{
	unsigned char *Next; Next = Mem;
	RomBios 	= Next; Next += 0x080000;
	RomGame 	= Next; Next += 0x200000;

//	RomGfx02	= Next; Next += 0xA00000 / 5 * 8;	// spr 16x16x5 sprite
//	MSM6295ROM	= Next; Next += 0x080000;
	
	RamStart	= Next;
	
	RamMain		= Next; Next += 0x100000;
	RamCache	= Next; Next += 0x001000;
	
	RamSpr		= Next; Next += 0x004000;
	RamV3t		= Next; Next += 0x040000;
	RamV3Reg	= Next; Next += 0x000080;
	RamV3Tbl	= Next; Next += 0x008000;
	RamV3Slc	= Next; Next += 0x008000;
	
	RamPal		= Next; Next += 0x020000;
//	RamBg00		= (unsigned short *) Next; Next += 0x002000;
//	RamZ80		= Next; Next += 0x001800;

	RamEnd		= Next;
	
//	RamCurPal	= (unsigned short *) Next; Next += 0x008000;
	
	MemEnd		= Next;
	return 0;
}

unsigned char __fastcall galpani4ReadByte(unsigned int addr)
{
	addr &= 0xc7ffffff;
	
	switch (addr) {

	case 0x00400007:
		return 0xff;
		
	default:
		bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %8x\n"), addr);
	}
	return 0;
}

unsigned short __fastcall galpani4ReadWord(unsigned int addr)
{
	switch (addr) {
		
		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %8x\n"), addr);
	}
	return 0;
}

unsigned int __fastcall galpani4ReadLong(unsigned int addr)
{
	switch (addr) {
		
		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read long value of location %8x\n"), addr);
	}
	return 0;
}

void __fastcall galpani4WriteByte(unsigned int addr, unsigned char data)
{
	switch (addr) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %2x to location %8x\n"), data, addr);
	}
}

void __fastcall galpani4WriteWord(unsigned int addr, unsigned short data)
{
	switch (addr) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to write word value %4x to location %8x\n"), data, addr);
	}
}

void __fastcall galpani4WriteLong(unsigned int addr, unsigned int data)
{
	switch (addr) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to write long value %8x to location %8x\n"), data, addr);
	}
}

static int DrvDoReset()
{

	Sh2Reset( *(unsigned int *)(RomBios + 0), 
			  *(unsigned int *)(RomBios + 4)/*, 
			  0x000000f0*/ );

	return 0;
}

static void be_to_le(unsigned char * p, int size)
{
	unsigned char c;
	for(int i=0; i<size; i+=4, p+=4) {
		c = *(p+0);	*(p+0) = *(p+3);	*(p+3) = c;
		c = *(p+1);	*(p+1) = *(p+2);	*(p+2) = c;
	}
}

static int galpani4Init()
{
	int nRet;
	
	Mem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();	
	
	nRet = BurnLoadRom(RomBios + 0x000000, 0, 1); if (nRet != 0) return 1;
	be_to_le( RomBios, 0x080000 );
	
	nRet = BurnLoadRom(RomGame + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomGame + 0x000001, 2, 2); if (nRet != 0) return 1;
	be_to_le( RomGame, 0x200000 );

	{
		Sh2Init(1);
	    Sh2Open(0);

		// Map 68000 memory:
		Sh2MapMemory(RomBios,		0x00000000, 0x0007ffff, SM_ROM);	// BIOS

		Sh2MapMemory(RamSpr,		0x02000000, 0x02003fff, SM_RAM);

		Sh2MapMemory(RamV3Reg,		0x02400000, 0x0240007f, SM_RAM);
		Sh2MapMemory(RamV3Tbl,		0x02500000, 0x02507fff, SM_RAM);
		Sh2MapMemory(RamV3Slc,		0x02600000, 0x02607fff, SM_RAM);
		
		Sh2MapMemory(RamPal,		0x02a40000, 0x02a5ffff, SM_RAM);	// Palette
	
		Sh2MapMemory(RomGame,		0x04000000, 0x041fffff, SM_ROM);	// Game ROM
		Sh2MapMemory(RamV3t,		0x04800000, 0x0483ffff, SM_RAM);	// tilemap b ram based tiles
		Sh2MapMemory(RamMain,		0x06000000, 0x060fffff, SM_RAM);	// Main RAM
		Sh2MapMemory(RamCache,		0xc0000000, 0xc0000fff, SM_RAM);	// Cache RAM
		

//		SekMapHandler(1,			0x180000, 0x187FFF, SM_WRITE);
		
		Sh2SetReadByteHandler (0, galpani4ReadByte);
		Sh2SetReadWordHandler (0, galpani4ReadWord);
		Sh2SetReadLongHandler (0, galpani4ReadLong);
		
		Sh2SetWriteByteHandler(0, galpani4WriteByte);
		Sh2SetWriteWordHandler(0, galpani4WriteWord);
		Sh2SetWriteLongHandler(0, galpani4WriteLong);
		
		//SekSetWriteByteHandler(1, shadfrceWriteBytePalette);
		//SekSetWriteWordHandler(1, shadfrceWriteWordPalette);

	}


	DrvDoReset();
	
	return 0;
}

static int galpani4Exit()
{
	
	Sh2Exit();
	
	free(Mem);
	Mem = NULL;
	
	return 0;
}

static void DrvDraw()
{
}

static int galpani4Frame()
{
	if (DrvReset)														// Reset machine
		DrvDoReset();
		
	if (bRecalcPalette) {
		// for(int i=0;i<0x4000;i++) CalcCol(i);
		bRecalcPalette = 0;
	}
	
	Sh2Run(28638000 / 60);
	
	if (pBurnDraw) DrvDraw();

	return 0;
}

static int galpani4Scan(int nAction,int *pnMin)
{
	if (pnMin) *pnMin =  0x029672;
//	struct BurnArea ba;


	return 0;
}
		
struct BurnDriver BurnDrvGalpani4 = {
	"galpani4", NULL, NULL, "1996",
	"Gals Panic 4 (Japan)\0", NULL, "Kaneko", "Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 2, HARDWARE_MISC_MISC,
	NULL, galpani4RomInfo, galpani4RomName, galpani4InputInfo, NULL,
	galpani4Init, galpani4Exit, galpani4Frame, NULL, galpani4Scan, &bRecalcPalette, 
	320, 240, 4, 3
};

