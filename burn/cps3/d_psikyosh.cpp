#include "burnint.h"
#include "sh2.h"

static unsigned char *Mem = NULL, *MemEnd = NULL;
static unsigned char *RamStart, *RamEnd;

static unsigned char *RomSH2;
//static unsigned char *RomGame;

static unsigned char *RamSpr;
static unsigned char *RamBg;
static unsigned short *RamPal;
static unsigned char *RamVReg;

static unsigned char *RamSH2;

static unsigned short *CurPal;

static unsigned char DrvReset = 0;
static unsigned char bRecalcPalette = 0;

static struct BurnInputInfo s1945iiInputList[] = {
	
	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},

};

STDINPUTINFO(s1945ii);

static struct BurnRomInfo s1945iiRomDesc[] = {
	{ "2_prog_l.u18", 0x080000, 0x20a911b8, BRF_ESS | BRF_PRG },	// SH-2 Code
	{ "1_prog_h.u17", 0x080000, 0x4c0fe85e, BRF_ESS | BRF_PRG }, 
	
	{ "0l.u4",		  0x400000, 0xbfacf98d, BRF_ESS | BRF_PRG },	// cd content region
	{ "0h.u13",		  0x400000, 0x1266f67c, BRF_ESS | BRF_PRG }, 
	{ "1l.u3",		  0x400000, 0x2d3332c9, BRF_ESS | BRF_PRG },
	{ "1h.u12",		  0x400000, 0x27b32c3e, BRF_ESS | BRF_PRG }, 
	{ "2l.u2",		  0x400000, 0x91ba6d23, BRF_ESS | BRF_PRG },
	{ "2h.u20",		  0x400000, 0xfabf4334, BRF_ESS | BRF_PRG }, 
	{ "3l.u1",		  0x400000, 0xa6c3704e, BRF_ESS | BRF_PRG },
	{ "3h.u19",		  0x400000, 0x4cd3ca70, BRF_ESS | BRF_PRG }, 

};

STD_ROM_PICK(s1945ii);
STD_ROM_FN(s1945ii);

static int MemIndex()
{
	unsigned char *Next; Next = Mem;
	RomSH2	 	= Next; Next += 0x0100000;

	
	RamStart	= Next;
	
	RamSH2		= Next; Next += 0x0100000;
	RamSpr		= Next; Next += 0x0004000;
	RamBg		= Next; Next += 0x000c000;
	
	RamPal		= (unsigned short *) Next; Next += 0x0005000;
	RamVReg		= Next; Next += 0x0010000;

	RamEnd		= Next;
	
	CurPal		= (unsigned short *) Next; Next += 0x0005000;
	
	MemEnd		= Next;
	return 0;
}

unsigned char __fastcall ps3v1ReadByte(unsigned int addr)
{
	addr &= 0xc7ffffff;
	
	switch (addr) {
	
	case 0x05000000:
		// sound 
		return 0x0;
		
	case 0x05800000:
		
		return 0xff;

	default:

		bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %8x\n"), addr);
	}
	return 0;
}

unsigned short __fastcall ps3v1ReadWord(unsigned int addr)
{
	addr &= 0xc7ffffff;
	
	switch (addr) {

	default:

		bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %8x\n"), addr);
	}
	return 0;
}

unsigned int __fastcall ps3v1ReadLong(unsigned int addr)
{
	addr &= 0xc7ffffff;
		
	switch (addr) {
		
	default:

		bprintf(PRINT_NORMAL, _T("Attempt to read long value of location %8x\n"), addr);
	}
	return 0;
}

void __fastcall ps3v1WriteByte(unsigned int addr, unsigned char data)
{
	addr &= 0xc7ffffff;
	
	switch (addr) {
		
	case 0x05000000:
		// sound 
		break;

	default:
		
		bprintf(PRINT_NORMAL, _T("Attempt to write byte value   %02x to location %8x\n"), data, addr);
	}
}

void __fastcall ps3v1WriteWord(unsigned int addr, unsigned short data)
{
	addr &= 0xc7ffffff;
	
	switch (addr) {
		
	default:
			
		bprintf(PRINT_NORMAL, _T("Attempt to write word value %04x to location %8x\n"), data, addr);
	}
}

void __fastcall ps3v1WriteLong(unsigned int addr, unsigned int data)
{
	addr &= 0xc7ffffff;
	
	switch (addr) {

	default:
		bprintf(PRINT_NORMAL, _T("Attempt to write long value %8x to location %8x\n"), data, addr);
	}
}

static int DrvDoReset()
{

	Sh2Reset( *(unsigned int *)(RomSH2 + 0), 
			  *(unsigned int *)(RomSH2 + 4)/*, 
			  0x000000f0*/ );

	return 0;
}

static void be_to_le(unsigned char * p, int size)
{
	unsigned char c;
	for(int i=0; i<size; i+=4, p+=4) {
//		c = *(p+0);	*(p+0) = *(p+3);	*(p+3) = c;
		c = *(p+1);	*(p+1) = *(p+2);	*(p+2) = c;
	}
}

static int s1945iiInit()
{
	int nRet;
	
	Mem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();	
	
	nRet = BurnLoadRom(RomSH2 + 0x000000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomSH2 + 0x000001, 1, 2); if (nRet != 0) return 1;
	be_to_le( RomSH2, 0x100000 );

#if 0
FILE * f = fopen("d.bin", "wb+");	
fwrite(RomSH2, 1, 0x100000, f);
fclose(f);
#endif
	

	{
		Sh2Init(1);
	    Sh2Open(0);

		// Map 68000 memory:
		Sh2MapMemory(RomSH2,		0x00000000, 0x000fffff, SM_ROM);
		Sh2MapMemory(RamSpr,		0x03000000, 0x0300ffff, SM_RAM);
		Sh2MapMemory((unsigned char *) RamPal,		
									0x03040000, 0x03044fff, SM_RAM);
		Sh2MapMemory(RamVReg,		0x03050000, 0x0305ffff, SM_WRITE);
		Sh2MapMemory(RamSH2,		0x06000000, 0x060fffff, SM_RAM);
	

		Sh2SetReadByteHandler (0, ps3v1ReadByte);
		Sh2SetReadWordHandler (0, ps3v1ReadWord);
		Sh2SetReadLongHandler (0, ps3v1ReadLong);
		Sh2SetWriteByteHandler(0, ps3v1WriteByte);
		Sh2SetWriteWordHandler(0, ps3v1WriteWord);
		Sh2SetWriteLongHandler(0, ps3v1WriteLong);

	}


	DrvDoReset();
	
	return 0;
}

static int s1945iiExit()
{
	
	Sh2Exit();
	
	free(Mem);
	Mem = NULL;
	
	return 0;
}

static void DrvDraw()
{
	memcpy(pBurnDraw, RamStart, 384 * 224 * 2);
	
}


static int s1945iiFrame()
{
	if (DrvReset)														// Reset machine
		DrvDoReset();
		
	if (bRecalcPalette) {
		// for(int i=0;i<0x4000;i++) CalcCol(i);
		bRecalcPalette = 0;
	}
	
	Sh2Run(28636350 / 60);

//	Sh2SetIRQLine(4, SH2_IRQSTATUS_AUTO);
	
//	Sh2SetIRQLine(12, SH2_IRQSTATUS_AUTO);
		
//	bprintf(0, _T("PC: %08x\n"), Sh2GetPC(0));
	
	//Sh2Run(1000);
	
	if (pBurnDraw) DrvDraw();

	return 0;
}

static int s1945iiScan(int nAction,int *pnMin)
{
	if (pnMin) *pnMin =  0x029672;
//	struct BurnArea ba;


	return 0;
}

		
struct BurnDriver BurnDrvS1945ii = {
	"s1945ii", NULL, NULL, "1997",
	"Strikers 1945 II\0", NULL, "Psikyo", "Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 2, HARDWARE_MISC_MISC,
	NULL, s1945iiRomInfo, s1945iiRomName, s1945iiInputInfo, NULL,
	s1945iiInit, s1945iiExit, s1945iiFrame, NULL, s1945iiScan, &bRecalcPalette, 
	384, 224, 4, 3
};
