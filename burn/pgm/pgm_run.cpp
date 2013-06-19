
#include "pgm.h"
#include "cache.h"
#include "gp2xmemfuncs.h"
#include "gp2xsdk.h"

unsigned char PgmJoy1[8] = {0,0,0,0,0,0,0,0};
unsigned char PgmJoy2[8] = {0,0,0,0,0,0,0,0};
unsigned char PgmJoy3[8] = {0,0,0,0,0,0,0,0};
unsigned char PgmJoy4[8] = {0,0,0,0,0,0,0,0};
unsigned char PgmBtn1[8] = {0,0,0,0,0,0,0,0};
unsigned char PgmBtn2[8] = {0,0,0,0,0,0,0,0};
unsigned char PgmInput[8] = {0,0,0,0,0,0,0,0};
unsigned char PgmReset = 0;

int nPGM68KROMLen = 0;
int nPGMTileROMLen = 0;
int nPGMSPRColROMLen = 0;
int nPGMSPRMaskROMLen = 0;
int nPGMSNDROMLen = 0;

unsigned int *RamBg, *RamTx, *RamCurPal;
unsigned short *RamRs, *RamPal, *RamVReg, *RamSpr;
static unsigned char *RamZ80, *Ram68K;

static unsigned char *Mem = NULL, *MemEnd = NULL;
static unsigned char *RamStart, *RamEnd;

unsigned char *USER0, *USER1, *USER2; // User regions
unsigned char *PGM68KBIOS, *PGM68KROM, *PGMTileROM, *PGMTileROMExp, *PGMSPRColROM, *PGMSPRMaskROM, *PGMSNDROM, *PGMARMROM;

static unsigned char bGamePuzlstar = 0;
static unsigned char bGameDrgw2 = 0;
unsigned char nPgmPalRecalc = 0;
unsigned char nPgmZ80Work = 0;

void (*pPgmInitCallback)() = NULL;
int (*pPgmScanCallback)(int, int*) = NULL;

static int pgmMemIndex()
{
	unsigned char *Next; Next = Mem;
	PGM68KBIOS	= Next; Next += 0x0020000;			// 68000 BIOS
	PGM68KROM	= Next; Next += nPGM68KROMLen;		// 68000 PRG (max 0x400000)
	USER0		= Next; Next += 0x0200000;			// User0 ROM/RAM space (for protection roms, etc)

	RamStart	= Next;
	
	Ram68K		= Next; Next += 0x0020000;						// 128K Main RAM
	RamBg		= (unsigned int *) Next; Next += 0x0004000;
	RamTx		= (unsigned int *) Next; Next += 0x0002000;
	RamRs		= (unsigned short *) Next; Next += 0x0000800;	// Row Scroll
	RamPal		= (unsigned short *) Next; Next += 0x0001200;	// Palette R5G5B5
	RamVReg		= (unsigned short *) Next; Next += 0x0010000;	// Video Regs inc. Zoom Table
	RamZ80		= Next; Next += 0x0010000;
	
	RamEnd		= Next;
	
	RamSpr		= (unsigned short *) Ram68K;	// first 0xa00 of main ram = sprites, seems to be buffered, DMA? 
	RamCurPal	= (unsigned int *) Next; Next += 0x001200 * sizeof(unsigned int);
	
	MemEnd		= Next;
	return 0;
}

static int pgmGetRoms(bool bLoad)
{
	char* pRomName;
	struct BurnRomInfo ri;
	struct BurnRomInfo pi;

	unsigned char *PGM68KROMLoad = PGM68KROM;
	unsigned char *PGMTileROMLoad = PGMTileROM + 0x400000;
	unsigned char *PGMSPRColROMLoad = PGMSPRColROM;
	unsigned char *PGMSPRMaskROMLoad = PGMSPRMaskROM;
	unsigned char *PGMSNDROMLoad = PGMSNDROM + 0x400000;

	for (int i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {

		BurnDrvGetRomInfo(&ri, i);

		if ((ri.nType & BRF_PRG) && (ri.nType & 7) == 1)
		{
			if (bLoad) {
				BurnDrvGetRomInfo(&pi, i+1);

				if (ri.nLen == 0x80000 && pi.nLen == 0x80000)
				{
					BurnLoadRom(PGM68KROMLoad + 0, i + 0, 2);
					BurnLoadRom(PGM68KROMLoad + 1, i + 1, 2);
					PGM68KROMLoad += pi.nLen;
					i += 1;
				}
				else
				{
					BurnLoadRom(PGM68KROMLoad, i, 1);
				}
				PGM68KROMLoad += ri.nLen;				
			} else {
				nPGM68KROMLen += ri.nLen;
			}
			continue;
		}

		if ((ri.nType & BRF_GRA) && (ri.nType & 7) == 2)
		{
			if (bLoad) {
				BurnLoadRom(PGMTileROMLoad, i, 1);
				PGMTileROMLoad += ri.nLen;
			} else {
				nPGMTileROMLen += ri.nLen;
			}
			continue;
		}

		if ((ri.nType & BRF_GRA) && (ri.nType & 7) == 3)
		{
			if (bLoad) {
				BurnLoadRom(PGMSPRColROMLoad, i, 1);
				PGMSPRColROMLoad += ri.nLen;
			} else {
				nPGMSPRColROMLen += ri.nLen;
			}
			continue;
		}

		if ((ri.nType & BRF_GRA) && (ri.nType & 7) == 4)
		{
			if (bLoad) {
				BurnLoadRom(PGMSPRMaskROMLoad, i, 1);
				PGMSPRMaskROMLoad += ri.nLen;
			} else {
				nPGMSPRMaskROMLen += ri.nLen;
			}
			continue;
		}

		if ((ri.nType & BRF_SND) && (ri.nType & 7) == 5)
		{
			if (bLoad) {
				BurnLoadRom(PGMSNDROMLoad, i, 1);
				PGMSNDROMLoad += ri.nLen;
			} else {
				nPGMSNDROMLen += ri.nLen;
			}
			continue;
		}
	}

	if (!bLoad) nPGMTileROMLen += 0x400000;
	if (!bLoad) nPGMSNDROMLen += 0x400000;

	return 0;
}

/* Calendar Emulation */

static unsigned char CalVal, CalMask, CalCom=0, CalCnt=0;

static unsigned char bcd(unsigned char data)
{
	return ((data / 10) << 4) | (data % 10);
}

static unsigned char pgm_calendar_r()
{
	unsigned char calr;
	calr = (CalVal & CalMask) ? 1 : 0;
	CalMask <<= 1;
	return calr;
}

static void pgm_calendar_w(unsigned short data)
{
	// initialize the time, otherwise it crashes
	time_t nLocalTime = time(NULL);
	tm* tmLocalTime = localtime(&nLocalTime);

	CalCom <<= 1;
	CalCom |= data & 1;
	++CalCnt;
	if(CalCnt==4)
	{
		CalMask = 1;
		CalVal = 1;
		CalCnt = 0;
		
		switch(CalCom & 0xf)
		{
			case 1: case 3: case 5: case 7: case 9: case 0xb: case 0xd:
				CalVal++;
				break;
			case 0:
				CalVal=bcd(tmLocalTime->tm_wday); //??
				break;
			case 2:  //Hours
				CalVal=bcd(tmLocalTime->tm_hour);
				break;
			case 4:  //Seconds
				CalVal=bcd(tmLocalTime->tm_sec);
				break;
			case 6:  //Month
				CalVal=bcd(tmLocalTime->tm_mon + 1); //?? not bcd in MVS
				break;
			case 8:
				CalVal=0; //Controls blinking speed, maybe milliseconds
				break;
			case 0xa: //Day
				CalVal=bcd(tmLocalTime->tm_mday);
				break;
			case 0xc: //Minute
				CalVal=bcd(tmLocalTime->tm_min);
				break;
			case 0xe:  //Year
				CalVal=bcd(tmLocalTime->tm_year % 100);
				break;
			case 0xf:  //Load Date
				tmLocalTime = localtime(&nLocalTime);
				break;
		}
	}
}


inline static unsigned int CalcCol(unsigned short nColour)
{
	int r, g, b;

	r = (nColour & 0x001F) << 3;	// Red
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;  	// Green
	g |= g >> 5;
	b = (nColour & 0x7C00) >> 7;	// Blue
	b |= b >> 5;

	return BurnHighCol(b, g, r, 0);
}

/* memory handler */

unsigned char __fastcall PgmReadByte(unsigned int sekAddress)
{
	if ((sekAddress & 0xFF0000) == 0x4F0000)
		return 0;
		
	switch (sekAddress) {

		//case 0xC00005:
		//	return ics2115_soundlatch_r(1);

		case 0xC00007:
			return pgm_calendar_r();

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	
	}
	return 0;
}

unsigned short __fastcall PgmReadWord(unsigned int sekAddress)
{
	if ((sekAddress & 0xFF0000) == 0x4F0000)
		if ( bGamePuzlstar )
			return PSTARS_protram_r(sekAddress & 0xffff);
		else
			return sango_protram_r(sekAddress & 0xffff);

	switch (sekAddress) {
		
		case 0x500000:
		case 0x500002:
			if ( bGamePuzlstar )
				return PSTARS_r16(sekAddress & 0xffff);
			else
				return pgm_asic28_r(sekAddress & 0xffff);
			
		case 0xC00004:
			return ics2115_soundlatch_r(1);
		case 0xC0400E:	// ASIC 3 protect
			return pgm_asic3_r(sekAddress);
		case 0xC08000:	// p1+p2 controls
			return ~(PgmInput[0] | (PgmInput[1] << 8));
		case 0xC08002:  // p3+p4 controls
			return ~(PgmInput[2] | (PgmInput[3] << 8));
		case 0xC08004:  // extra controls
			return ~(PgmInput[4] | (PgmInput[5] << 8));
		case 0xC08006: // dipswitches
			return ~(PgmInput[6]) | 0xffe0;

		case 0xd40000:
		case 0xd40002:
			return killbld_prot_r(sekAddress & 0xffff);

		case 0xd80000:
		case 0xd80002:
			return dw2_d80000_r(sekAddress);

		case 0xDCB400:
		case 0xDCB402:
			return olds_r16(sekAddress & 3);

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

/*void __fastcall PgmWriteByte(unsigned int sekAddress, unsigned char byteValue)
{
	switch (sekAddress) {
		
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}*/

void __fastcall PgmWriteWord(unsigned int sekAddress, unsigned short wordValue)
{
	switch (sekAddress) {
		case 0x500000:
		case 0x500002:
			if ( bGamePuzlstar )
				PSTARS_w16(sekAddress & 0xffff, wordValue);
			else
				pgm_asic28_w(sekAddress & 0xffff, wordValue);
			break;
		case 0x500004:
			break;	
			
		case 0x700006:	// Watchdog ???
			//bprintf(PRINT_NORMAL, _T("Watchdog write %04x\n"), wordValue);
			break;
			
		case 0xC00002:	// m68k_l1_w
			ics2115_soundlatch_w(0, wordValue);
			if(nPgmZ80Work) ZetNmi();
			break;
		case 0xC00004:	// soundlatch2_word_w
			ics2115_soundlatch_w(1, wordValue);
			break;
		case 0xC00006:
			pgm_calendar_w(wordValue);
			break;
		case 0xC00008:	// z80_reset_w
//			bprintf(PRINT_NORMAL, _T("z80_reset_w(%04x)  %4.1f%%\n"), wordValue, 6.0 * SekTotalCycles() / 20000.0);
			if (wordValue == 0x5050) {
				ics2115_reset();
				nPgmZ80Work = 1;
				
				ZetReset();
			} else {
				/* this might not be 100% correct, but several of the games (ddp2, puzzli2 etc. expect the z80 to be turned
           		   off during data uploads, they write here before the upload */
				nPgmZ80Work = 0;
			}
			break;
		case 0xC0000A:	// z80_ctrl_w
			break;
		case 0xC0000C:	// soundlatch3_word_w
			ics2115_soundlatch_w(2, wordValue);
			break;	
		
		case 0xC04000:
			pgm_asic3_reg_w(sekAddress, wordValue);
			break;
		case 0xC0400E:
			pgm_asic3_w(sekAddress, wordValue); // & 0xff
			break;
		
		case 0xC08006:
			// input_port_3_word_w (dipswitches)
			break;

		case 0xd40000:
		case 0xd40002:
			killbld_prot_w(sekAddress & 0xffff, wordValue);
			break;

		case 0xDCB400:
		case 0xDCB402:
			olds_w16(sekAddress & 3, wordValue);
			break;

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
	}
}

void __fastcall PgmPalWriteWord(unsigned int sekAddress, unsigned short wordValue)
{
	// 0xA00000 ~ 0xA011FF: 2304 color Palette (X1R5G5B5)
	sekAddress -= 0xA00000;
	sekAddress >>= 1;
	RamPal[sekAddress] = wordValue;
	RamCurPal[sekAddress] = CalcCol(wordValue);
}

unsigned char __fastcall PgmZ80ReadByte(unsigned int sekAddress)
{
	switch (sekAddress) {

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

unsigned short __fastcall PgmZ80ReadWord(unsigned int sekAddress)
{
	sekAddress -= 0xC10000;
	return (RamZ80[sekAddress] << 8) | RamZ80[sekAddress+1];
}

/*void __fastcall PgmZ80WriteByte(unsigned int sekAddress, unsigned char byteValue)
{
	switch (sekAddress) {
		
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}*/

void __fastcall PgmZ80WriteWord(unsigned int sekAddress, unsigned short wordValue)
{
	sekAddress -= 0xC10000;
	RamZ80[sekAddress] = wordValue >> 8;
	RamZ80[sekAddress+1] = wordValue & 0xFF;
}

unsigned char __fastcall PgmZ80PortRead(unsigned short p)
{
	switch (p >> 8) {
		case 0x80:
			return ics2115read(p & 0xff);
		case 0x81:
			return ics2115_soundlatch_r(2) & 0xff;
		case 0x82:
			return ics2115_soundlatch_r(0) & 0xff;
		case 0x84:
			return ics2115_soundlatch_r(1) & 0xff;
//		default:
//			bprintf(PRINT_NORMAL, _T("Z80 Attempt to read port %04x\n"), p);
	}
	return 0;
}

void __fastcall PgmZ80PortWrite(unsigned short p, unsigned char v)
{
	switch (p >> 8) {
		case 0x80:
			ics2115write(p&0xff, v);
			break;
		case 0x81:
			ics2115_soundlatch_w(2, v);
			break;
		case 0x82:
			ics2115_soundlatch_w(0, v);
			break;	
		case 0x84:
			ics2115_soundlatch_w(1, v);
			break;
//		default:
//			bprintf(PRINT_NORMAL, _T("Z80 Attempt to write %02x to port %04x\n"), v, p);
	}
}

int PgmDoReset()
{
	SekOpen(0);
	SekSetIRQLine(0, SEK_IRQSTATUS_NONE);
	SekReset();
	SekClose();
	
	nPgmZ80Work = 0;
	ZetReset();
	ics2115_reset();
	
	return 0;
}


static void expand_gfx_2()
{
	unsigned char *src = PGMTileROM;
	unsigned char *dst = PGMTileROMExp;

	for (int i = nPGMTileROMLen/5-1; i >= 0 ; i --) {
		dst[0+8*i] = ((src[0+5*i] >> 0) & 0x1f);
		dst[1+8*i] = ((src[0+5*i] >> 5) & 0x07) | ((src[1+5*i] << 3) & 0x18);
		dst[2+8*i] = ((src[1+5*i] >> 2) & 0x1f );
		dst[3+8*i] = ((src[1+5*i] >> 7) & 0x01) | ((src[2+5*i] << 1) & 0x1e);
		dst[4+8*i] = ((src[2+5*i] >> 4) & 0x0f) | ((src[3+5*i] << 4) & 0x10);
		dst[5+8*i] = ((src[3+5*i] >> 1) & 0x1f );
		dst[6+8*i] = ((src[3+5*i] >> 6) & 0x03) | ((src[4+5*i] << 2) & 0x1c);
		dst[7+8*i] = ((src[4+5*i] >> 3) & 0x1f );
	}
}

int pgmInit()
{
	Mem = NULL;
	bGamePuzlstar = (strcmp(BurnDrvGetTextA(DRV_NAME), "puzlstar") == 0) ? 1 : 0;
	bGameDrgw2 = (strcmp(BurnDrvGetTextA(DRV_NAME), "drgw2") == 0 || strcmp(BurnDrvGetTextA(DRV_NAME), "drgw2c") == 0
		|| strcmp(BurnDrvGetTextA(DRV_NAME), "drgw2j") == 0) ? 1 : 0;

	if ( bBurnUseRomCache == 1 )
	{
		nPGM68KROMLen = BurnCacheBlockSize(0)-0x0220000;
		pgmMemIndex();
		int nLen = MemEnd - (unsigned char *)0;
		if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
		gp2x_memset(Mem, 0, nLen);
		pgmMemIndex();

		BurnCacheRead(PGM68KBIOS, 0);
		nPGMTileROMLen = BurnCacheBlockSize(1);
		nPGMSPRColROMLen = BurnCacheBlockSize(2);
		nPGMSPRMaskROMLen = BurnCacheBlockSize(3);
		nPGMSNDROMLen = BurnCacheBlockSize(4);
		PGMTileROM      = (unsigned char *)malloc(nPGMTileROMLen);	// 8x8 Text Tiles + 32x32 BG Tiles
		PGMTileROMExp   = (unsigned char *)UpperMalloc((nPGMTileROMLen / 5) * 8);	// Expanded 8x8 Text Tiles and 32x32 BG Tiles
		PGMSPRColROM	= (unsigned char *)BurnCacheMap(2);
		PGMSPRMaskROM	= (unsigned char *)BurnCacheMap(3);
		
		PGMSNDROM		= (unsigned char *)UpperMalloc(nPGMSNDROMLen);
		BurnCacheRead(PGMTileROM, 1);
		BurnCacheRead(PGMSNDROM, 4);
		// expand gfx1 into gfx2
		expand_gfx_2();
	}
	else
	{
		pgmGetRoms(false);
	
		PGMTileROMExp   = (unsigned char*)malloc((nPGMTileROMLen / 5) * 8);	// Expanded 8x8 Text Tiles and 32x32 BG Tiles
		PGMTileROM      = (unsigned char*)malloc(nPGMTileROMLen);		// 8x8 Text Tiles + 32x32 BG Tiles
		PGMSPRColROM	= (unsigned char*)UpperMalloc(nPGMSPRColROMLen);
		PGMSPRMaskROM	= (unsigned char*)UpperMalloc(nPGMSPRMaskROMLen);
		PGMSNDROM		= (unsigned char*)malloc(nPGMSNDROMLen);
	
		pgmMemIndex();
		int nLen = MemEnd - (unsigned char *)0;
		if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
		gp2x_memset(Mem, 0, nLen);
		pgmMemIndex();

		pgmGetRoms(true);
	
		// load bios roms
		BurnLoadRom(PGM68KBIOS,		0x00080, 1);	// 68k bios
		BurnLoadRom(PGMTileROM,		0x00081, 1);	// Bios Text and Tiles
		BurnLoadRom(PGMSNDROM,		0x00082, 1);	// Bios Intro Sounds
	
		// expand gfx1 into gfx2
		expand_gfx_2();
		if (pPgmInitCallback) {
			pPgmInitCallback();
		}
	}

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(PGM68KBIOS,	0x000000, 0x01FFFF, SM_ROM);				// 68000 BIOS

		if (strcmp(BurnDrvGetTextA(DRV_NAME), "killbldt") == 0)
		{
			SekMapMemory(PGM68KROM,	0x100000, 0x2FFFFF, SM_ROM);				// 68000 ROM
		} else {
			SekMapMemory(PGM68KROM,	0x100000, 0x4EFFFF, SM_ROM);				// 68000 ROM
		}

		SekMapMemory(Ram68K,		0x800000, 0x81FFFF, SM_RAM);				// Main Ram
		SekMapMemory(Ram68K,		0x820000, 0x83FFFF, SM_RAM);				// Mirrors... 
		SekMapMemory(Ram68K,		0x840000, 0x85FFFF, SM_RAM);
		SekMapMemory(Ram68K,		0x860000, 0x87FFFF, SM_RAM);
		SekMapMemory(Ram68K,		0x880000, 0x89FFFF, SM_RAM);
		SekMapMemory(Ram68K,		0x8A0000, 0x8BFFFF, SM_RAM);
		SekMapMemory(Ram68K,		0x8C0000, 0x8DFFFF, SM_RAM);
		SekMapMemory(Ram68K,		0x8E0000, 0x8FFFFF, SM_RAM);
		

		SekMapMemory((unsigned char *)RamBg,	0x900000, 0x903FFF, SM_RAM);
		SekMapMemory((unsigned char *)RamTx,	0x904000, 0x905FFF, SM_RAM);
		SekMapMemory((unsigned char *)RamRs,	0x907000, 0x9077FF, SM_RAM);
		SekMapMemory((unsigned char *)RamPal,	0xA00000, 0xA011FF, SM_ROM);
		SekMapMemory((unsigned char *)RamVReg,	0xB00000, 0xB0FFFF, SM_RAM);
		
		SekMapHandler(1,						0xA00000, 0xA011FF, SM_WRITE);
		SekMapHandler(2,						0xC10000, 0xC1FFFF, SM_READ | SM_WRITE);
		
		SekSetReadWordHandler(0, PgmReadWord);
		SekSetReadByteHandler(0, PgmReadByte);
		SekSetWriteWordHandler(0, PgmWriteWord);
//		SekSetWriteByteHandler(0, PgmWriteByte);
		
		SekSetWriteWordHandler(1, PgmPalWriteWord);
		
		SekSetReadWordHandler(2, PgmZ80ReadWord);
//		SekSetReadByteHandler(2, PgmZ80ReadByte);
		SekSetWriteWordHandler(2, PgmZ80WriteWord);
//		SekSetWriteByteHandler(2, PgmZ80WriteByte);
		

		SekClose();
	}
	
	{
		ZetInit(1);
		ZetOpen(0);
		
		ZetMapArea(0x0000, 0xFFFF, 0, RamZ80);
		ZetMapArea(0x0000, 0xFFFF, 1, RamZ80);
		ZetMapArea(0x0000, 0xFFFF, 2, RamZ80);
		
		ZetMemEnd();
		
		ZetSetInHandler(PgmZ80PortRead);
		ZetSetOutHandler(PgmZ80PortWrite);
		
		ZetClose();
	}

	ics2115_init();

	PgmDoReset();
	return 0;
}

int pgmExit()
{
	SekExit();
	ZetExit();
	
	free(Mem);
	Mem = NULL;

	ics2115_exit();
	
	prot_reset();

	if ( !bBurnUseRomCache )
	{
		free (PGMTileROM);
		free (PGMTileROMExp);
		UpperFree (PGMSPRColROM);
		UpperFree (PGMSPRMaskROM);
	}

	free (PGMSNDROM);
	
	PGM68KBIOS = NULL;
	PGM68KROM = NULL;
	PGMTileROM = NULL;
	PGMTileROMExp = NULL;
	PGMSPRColROM = NULL;
	PGMSPRMaskROM = NULL;
	PGMSNDROM = NULL;

	nPGM68KROMLen = 0;
	nPGMTileROMLen = 0;
	nPGMSPRColROMLen = 0;
	nPGMSPRMaskROMLen = 0;
	nPGMSNDROMLen = 0;

	pPgmInitCallback = NULL;
	pPgmScanCallback = NULL;

	return 0;
}

#define M68K_CYCS_PER_FRAME	(16000000 / 60)
#define Z80_CYCS_PER_FRAME	( 8468000 / 60)

#define	PGM_INTER_LEAVE	2

#define M68K_CYCS_PER_INTER	(M68K_CYCS_PER_FRAME / PGM_INTER_LEAVE)
#define Z80_CYCS_PER_INTER	(Z80_CYCS_PER_FRAME  / PGM_INTER_LEAVE)

int pgmFrame()
{
	if (PgmReset) 
		PgmDoReset();
	
	if (nPgmPalRecalc) {
		for (int i=0;i<(0x1200/2);i++)
			RamCurPal[i] = CalcCol(RamPal[i]);
		nPgmPalRecalc = 0;
	}

	// Compile digital inputs
	PgmInput[0] = 0x0000;
	PgmInput[1] = 0x0000;
	PgmInput[2] = 0x0000;
	PgmInput[3] = 0x0000;
	PgmInput[4] = 0x0000;
	PgmInput[5] = 0x0000;
	for (int i = 0; i < 8; i++) {
		PgmInput[0] |= (PgmJoy1[i] & 1) << i;
		PgmInput[1] |= (PgmJoy2[i] & 1) << i;
		PgmInput[2] |= (PgmJoy3[i] & 1) << i;
		PgmInput[3] |= (PgmJoy4[i] & 1) << i;
		PgmInput[4] |= (PgmBtn1[i] & 1) << i;
		PgmInput[5] |= (PgmBtn2[i] & 1) << i;
	}	

	int nCyclesDone[2] = {0, 0};
	int nCyclesNext[2] = {0, 0};
	
	SekNewFrame();
	ZetNewFrame();
	SekOpen(0);
	//ZetOpen(0);

	for(int i=0; i<PGM_INTER_LEAVE; i++) {
		nCyclesNext[0] += M68K_CYCS_PER_INTER;
		nCyclesNext[1] += Z80_CYCS_PER_INTER;
		
		nCyclesDone[0] += SekRun( nCyclesNext[0] - nCyclesDone[0] );
		if ( nPgmZ80Work ) {
			nCyclesDone[1] += ZetRun( nCyclesNext[1] - nCyclesDone[1] );
			
		} else
			nCyclesDone[1] += nCyclesNext[1] - nCyclesDone[1];
	}

	if (bGameDrgw2)
	{
		SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
		SekRun(nCyclesNext[0] - nCyclesDone[0]);
		SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		SekRun(nCyclesNext[0] - nCyclesDone[0]);
	} else {
		SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
	}

	ics2115_frame();

	SekClose();
	//ZetClose();

	ics2115_update(nBurnSoundLen);

	if (pBurnDraw) pgmDraw();
	
	return 0;
}

int pgmScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin =  0x029671;
	}

	if (nAction & ACB_MEMORY_ROM) {						// Scan memory rom
		ba.Data		= PGM68KBIOS;
		ba.nLen		= 0x0020000;
		ba.nAddress = 0;
		ba.szName	= "BIOS ROM";
		BurnAcb(&ba);

		ba.Data		= PGM68KROM;
		ba.nLen		= nPGM68KROMLen;

		ba.nAddress = 0;
		ba.szName	= "68K ROM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_NVRAM) {								// Scan nvram
		ba.Data		= Ram68K;
		ba.nLen		= 0x0020000;
		ba.nAddress = 0;
		ba.szName	= "68K RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_MEMORY_RAM) {						// Scan memory, devices & variables
		ba.Data		= RamBg;
		ba.nLen		= 0x0004000;
		ba.nAddress = 0;
		ba.szName	= "Bg RAM";
		BurnAcb(&ba);

		ba.Data		= RamTx;
		ba.nLen		= 0x0002000;
		ba.nAddress = 0;
		ba.szName	= "Tx RAM";
		BurnAcb(&ba);

		ba.Data		= RamRs;
		ba.nLen		= 0x0000800;
		ba.nAddress = 0;
		ba.szName	= "Row Scroll";
		BurnAcb(&ba);

		ba.Data		= RamPal;
		ba.nLen		= 0x0001200;
		ba.nAddress = 0;
		ba.szName	= "Palette";
		BurnAcb(&ba);

		ba.Data		= RamVReg;
		ba.nLen		= 0x0010000;
		ba.nAddress = 0;
		ba.szName	= "Video Regs";
		BurnAcb(&ba);
		
		ba.Data		= RamZ80;
		ba.nLen		= 0x0010000;
		ba.nAddress = 0;
		ba.szName	= "Z80 RAM";
		BurnAcb(&ba);
		
	}

	if (nAction & ACB_DRIVER_DATA) {
	
		SekScan(nAction);										// Scan 68000 state
		ZetScan(nAction);										// Scan Z80 state

		// Scan critical driver variables
		SCAN_VAR(PgmInput);

		if (nAction & ACB_WRITE)
			nPgmPalRecalc = 1;
		
		SCAN_VAR(nPgmZ80Work);
		ics2115_scan(nAction, pnMin);
	}

	// save asic protections, even if they're not used
	asic3Scan(nAction, pnMin);
	asic28Scan(nAction, pnMin);

	if (pPgmScanCallback) {
		pPgmScanCallback(nAction, pnMin);
	}

 	return 0;
}
