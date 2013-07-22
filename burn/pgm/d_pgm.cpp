/*
 * PGM System (c)1997 IGS 
 * Based on Information from ElSemi
 *
 * IGS driver for Finalburn Alpha ported from MAME v0.115
 *
 * Port by OopsWare 2007
 * http://oopsware.googlepages.com
 *
 * 
 * Changes:
 *
 * 07-09-2007 
 *   Sound available
 *
 * 07-06-2007 
 *  Change sceen scale 2:1 back to 4:3 ( sprites seems been stove )
 *  Add nPgmPalRecalc, support color depth change while gamming
 *  fix Save states
 *
 * 07-04-2007 ( IQ_132's release )
 *  Added all of BisonSAS's changes
 *  - Save State support
 *  - pgmPalUpdate()
 *  - Diagnostics Inputs
 *  - Dipswitches "Music and Voice"
 *  - Dipswitch "Disable Ba-Chieh" in sets orlegend, orlegnde and orlegndc
 *  - Removed chinese title in orlegend, orlegnde and orld105k
 *  - Save game configurations. (NVRAM)
 *  Drawing, protection, decrypt, sound, drivers, etc are all in seperate files
 *  All drivers for PGM games in MAME are added (though most, as in MAME, are not working)
 *  - Killing Blade, The (Chinese) is working (though glitchy)
 *  - Photoy2k & Real and Fake are working perfectly
 *  - Puzzle Star is working (though glitchy, freezes after second stage)
 *  - Dragon World 2 boots and runs, though the controls do not work (needs another irq?)
 *  - PGM Homebrew (PGM Frog Feast, PGM Demo) are supported (in the source) and work Perfectly
 *  All games have a shared loading routine (like CPS-2 and NeoGeo)
 *  All games have a shared memory map (like CPS-2 and NeoGeo)
 *  Improved Save states a little
 *  Removed code meant for low-ram environment
 *  Shortened and cleaned some code, removed a lot of 'printf's'
 *  Added all decrypt routines
 *
 * 06-09-2007
 *   rows scrolls not emulated
 *   sprite zoom not emulated
 *   no sound (z80 and ics2115 not emulated) 
 */

#include "pgm.h"

static struct BurnRomInfo emptyRomDesc[] = {
	{ "",                    0,          0, 0 },
};

static struct BurnInputInfo pgmInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	PgmBtn1 + 0,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	PgmJoy1 + 0,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	PgmJoy1 + 1,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	PgmJoy1 + 2,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	PgmJoy1 + 3,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	PgmJoy1 + 4,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	PgmJoy1 + 5,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	PgmJoy1 + 6,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	PgmJoy1 + 7,	"p1 fire 3"},
	{"P1 Button 4",	BIT_DIGITAL,	PgmBtn2 + 0,	"p1 fire 4"},

	{"P2 Coin",		BIT_DIGITAL,	PgmBtn1 + 1,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	PgmJoy2 + 0,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	PgmJoy2 + 1,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	PgmJoy2 + 2,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	PgmJoy2 + 3,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	PgmJoy2 + 4,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	PgmJoy2 + 5,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	PgmJoy2 + 6,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL,	PgmJoy2 + 7,	"p2 fire 3"},
	{"P2 Button 4",	BIT_DIGITAL,	PgmBtn2 + 1,	"p2 fire 4"},

	{"P3 Coin",		BIT_DIGITAL,	PgmBtn1 + 2,	"p3 coin"},
	{"P3 Start",	BIT_DIGITAL,	PgmJoy3 + 0,	"p3 start"},
	{"P3 Up",		BIT_DIGITAL,	PgmJoy3 + 1,	"p3 up"},
	{"P3 Down",		BIT_DIGITAL,	PgmJoy3 + 2,	"p3 down"},
	{"P3 Left",		BIT_DIGITAL,	PgmJoy3 + 3,	"p3 left"},
	{"P3 Right",	BIT_DIGITAL,	PgmJoy3 + 4,	"p3 right"},
	{"P3 Button 1",	BIT_DIGITAL,	PgmJoy3 + 5,	"p3 fire 1"},
	{"P3 Button 2",	BIT_DIGITAL,	PgmJoy3 + 6,	"p3 fire 2"},
	{"P3 Button 3",	BIT_DIGITAL,	PgmJoy3 + 7,	"p3 fire 3"},
	{"P3 Button 4",	BIT_DIGITAL,	PgmBtn2 + 2,	"p3 fire 4"},

	{"P4 Coin",		BIT_DIGITAL,	PgmBtn1 + 3,	"p4 coin"},
	{"P4 Start",	BIT_DIGITAL,	PgmJoy4 + 0,	"p4 start"},
	{"P4 Up",		BIT_DIGITAL,	PgmJoy4 + 1,	"p4 up"},
	{"P4 Down",		BIT_DIGITAL,	PgmJoy4 + 2,	"p4 down"},
	{"P4 Left",		BIT_DIGITAL,	PgmJoy4 + 3,	"p4 left"},
	{"P4 Right",	BIT_DIGITAL,	PgmJoy4 + 4,	"p4 right"},
	{"P4 Button 1",	BIT_DIGITAL,	PgmJoy4 + 5,	"p4 fire 1"},
	{"P4 Button 2",	BIT_DIGITAL,	PgmJoy4 + 6,	"p4 fire 2"},
	{"P4 Button 3",	BIT_DIGITAL,	PgmJoy4 + 7,	"p4 fire 3"},
	{"P4 Button 4",	BIT_DIGITAL,	PgmBtn2 + 3,	"p4 fire 4"},

	{"Reset",		BIT_DIGITAL,	&PgmReset,		"reset"},
	{"Diagnostics 1",	BIT_DIGITAL,	PgmBtn1 + 4,	"diag"},
	{"Diagnostics 2",	BIT_DIGITAL,	PgmBtn1 + 6,	""},
	{"Service 1",	BIT_DIGITAL,	PgmBtn1 + 5,	"service"},
	{"Service 2",	BIT_DIGITAL,	PgmBtn1 + 7,	""},

	{"Dip A",		BIT_DIPSWITCH,	PgmInput + 6,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	PgmInput + 7,	"dip"},
};

STDINPUTINFO(pgm);

static struct BurnDIPInfo pgmDIPList[] = {

	// Defaults
	{0x2D,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Test mode"},
	{0x2D,	0x01, 0x01,	0x00, "No"},
	{0x2D,	0x01, 0x01,	0x01, "Yes"},
	{0,		0xFE, 0,	2,	  "Music"},
	{0x2D,	0x01, 0x02,	0x02, "No"},
	{0x2D,	0x01, 0x02,	0x00, "Yes"},
	{0,		0xFE, 0,	2,	  "Voice"},
	{0x2D,	0x01, 0x04,	0x04, "No"},
	{0x2D,	0x01, 0x04,	0x00, "Yes"},
	{0,		0xFE, 0,	2,	  "Free play"},
	{0x2D,	0x01, 0x08,	0x00, "No"},
	{0x2D,	0x01, 0x08,	0x08, "Yes"},
	
	{0,		0xFE, 0,	2,	  "Stop mode"},
	{0x2D,	0x01, 0x10,	0x00, "No"},
	{0x2D,	0x01, 0x10,	0x10, "Yes"},
};

static struct BurnDIPInfo orlegendDIPList[] = {

	// Defaults
	{0x2E,	0xFF, 0xFF,	0x00, NULL},

	// DIP 2
	{0,		0xFE, 0,	2,	  "Disable Ba-Chieh"},
	{0x2E,	0x01, 0x01,	0x00, "No"},
	{0x2E,	0x01, 0x01,	0x01, "Yes"},
};

static struct BurnDIPInfo orlegndcDIPList[] = {

	// Defaults
	{0x2E,	0xFF, 0xFF,	0x03, NULL},

	// DIP 2
	{0,		0xFE, 0,	2,	  "Disable Ba-Chieh"},
	{0x2E,	0x02, 0x01,	0x00, "No"},
	{0x2E,	0x00, 0x02, 0x00, NULL},
	{0x2E,	0x02, 0x01,	0x01, "Yes"},
	{0x2E,	0x00, 0x02, 0x00, NULL},
	{0,		0xFE, 0,	3,	  "Region"},
	{0x2E,	0x01, 0x03,	0x00, "World"},
	{0x2E,	0x01, 0x03,	0x02, "Korea"},
	{0x2E,	0x01, 0x03,	0x03, "China"},
	{0x2E,	0x01, 0x03,	0x01, "World"},
};

static struct BurnDIPInfo orld111cDIPList[] = {

	// Defaults
	{0x2E,	0xFF, 0xFF,	0x02, NULL},

	// DIP 2
	{0,		0xFE, 0,	2,	  "Region"},
	{0x2E,	0x01, 0x03,	0x00, "Hong Kong"},
	{0x2E,	0x01, 0x03,	0x02, "China"},
	{0x2E,	0x01, 0x03,	0x01, "Hong Kong"},
	{0x2E,	0x01, 0x03,	0x03, "China"},
};

static struct BurnDIPInfo orld105kDIPList[] = {

	// Defaults
	{0x2E,	0xFF, 0xFF,	0x02, NULL},
};

static struct BurnDIPInfo sangoDIPList[] = {

	// Defaults
	{0x2E,	0xFF, 0xFF,	0x05, NULL},

	// DIP 2
	{0,		0xFE, 0,	6,	  "Region"},
	{0x2E,	0x01, 0xFF,	0x00, "China"},
	{0x2E,	0x01, 0xFF,	0x01, "Taiwan"},
	{0x2E,	0x01, 0xFF,	0x02, "Japan (Alta license)"},
	{0x2E,	0x01, 0xFF,	0x03, "Korea"},
	{0x2E,	0x01, 0xFF,	0x04, "Hong Kong"},
	{0x2E,	0x01, 0xFF,	0x05, "World"},
};

static struct BurnDIPInfo kovjDIPList[] = {

	// Defaults
	{0x2E,	0xFF, 0xFF,	0x02, NULL},

	// DIP 2
	{0,		0xFE, 0,	6,	  "Region"},
	{0x2E,	0x01, 0xFF,	0x00, "China"},
	{0x2E,	0x01, 0xFF,	0x01, "Taiwan"},
	{0x2E,	0x01, 0xFF,	0x02, "Japan (Alta license)"},
	{0x2E,	0x01, 0xFF,	0x03, "Korea"},
	{0x2E,	0x01, 0xFF,	0x04, "Hong Kong"},
	{0x2E,	0x01, 0xFF,	0x05, "World"},
};

static struct BurnDIPInfo killbldDIPList[] = {

	// Defaults
	{0x2E,	0xFF, 0xFF,	0x05, NULL},

	// DIP 2
	{0,		0xFE, 0,	6,	  "Region"},
	{0x2E,	0x01, 0xFF,	0x00, "Taiwan"},
	{0x2E,	0x01, 0xFF,	0x01, "China"},
	{0x2E,	0x01, 0xFF,	0x02, "Hong Kong"},
	{0x2E,	0x01, 0xFF,	0x03, "Japan"},
	{0x2E,	0x01, 0xFF,	0x0a, "Korea"},
	{0x2E,	0x01, 0xFF,	0x0b, "World"},
};
STDDIPINFO(pgm);

STDDIPINFOEXT(orlegend, pgm, orlegend);
STDDIPINFOEXT(orlegndc, pgm, orlegndc);
STDDIPINFOEXT(orld111c, pgm, orld111c);
STDDIPINFOEXT(orld105k, pgm, orld105k);
STDDIPINFOEXT(sango,    pgm, sango);
STDDIPINFOEXT(kovj,     pgm, kovj);
STDDIPINFOEXT(killbld,  pgm, killbld);


// -----------------------------------------------------------------------------
// Bios

// PGM (Polygame Master) System Bios

static struct BurnRomInfo pgmRomDesc[] = {
	{ "pgm_p01s.rom", 0x020000, 0xe42b166e, BRF_ESS | BRF_PRG | BRF_BIOS | BRF_SELECT | BRF_OPT },	// 68000 Core
	{ "pgm_t01s.rom", 0x200000, 0x1a7123a0, BRF_GRA | BRF_BIOS }, 					// 8x8 Text Layer Tiles
	{ "pgm_m01s.rom", 0x200000, 0x45ae7159, BRF_SND | BRF_BIOS },					// Samples - (8 bit mono 11025Hz)
};

STD_ROM_PICK(pgm);
STD_ROM_FN(pgm);

struct BurnDriver BurnDrvpgm = {
	"pgm", NULL, NULL, "1997",
	"PGM (Polygame Master) System BIOS\0", "BIOS only", "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_BOARDROM, 0, HARDWARE_IGS_PGM,
	NULL, pgmRomInfo, pgmRomName, pgmInputInfo, pgmDIPInfo,
	pgmInit, pgmExit, pgmFrame, pgmDraw, pgmScan, &nPgmPalRecalc,
	448, 224, 4, 3
};


// -----------------------------------------------------------------------------
// Normal Games

// Oriental Legend (ver. 126)

static struct BurnRomInfo orlegendRomDesc[] = {
	{ "p0103.rom",     0x200000, 0xd5e93543, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "t0100.rom",     0x400000, 0x61425e1e, 2 | BRF_GRA },		  // 32x32 BG Tiles

	{ "a0100.rom",     0x400000, 0x8b3bd88a, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0101.rom",     0x400000, 0x3b9e9644, 3 | BRF_GRA },
	{ "a0102.rom",     0x400000, 0x069e2c38, 3 | BRF_GRA },
	{ "a0103.rom",     0x400000, 0x4460a3fd, 3 | BRF_GRA },
	{ "a0104.rom",     0x400000, 0x5f8abb56, 3 | BRF_GRA },
	{ "a0105.rom",     0x400000, 0xa17a7147, 3 | BRF_GRA },

	{ "b0100.rom",     0x400000, 0x69d2e48c, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0101.rom",     0x400000, 0x0d587bf3, 4 | BRF_GRA },
	{ "b0102.rom",     0x400000, 0x43823c1e, 4 | BRF_GRA },

	{ "m0100.rom",     0x200000, 0xe5c36c83, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(orlegend, orlegend, pgm);
STD_ROM_FN(orlegend);

struct BurnDriver BurnDrvOrlegend = {
	"orlegend", NULL, "pgm", "1997",
	"Oriental Legend (ver. 126)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, orlegendRomInfo, orlegendRomName, pgmInputInfo, orlegndcDIPInfo,
	pgmInit,pgmExit,pgmFrame,pgmDraw,pgmScan, &nPgmPalRecalc,
	448,224,4,3
};


// Oriental Legend (ver. 112)

static struct BurnRomInfo orlegndeRomDesc[] = {
	{ "p0102.rom",     0x200000, 0x4d0f6cc5, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "t0100.rom",     0x400000, 0x61425e1e, 2 | BRF_GRA },		  // 32x32 BG Tiles

	{ "a0100.rom",     0x400000, 0x8b3bd88a, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0101.rom",     0x400000, 0x3b9e9644, 3 | BRF_GRA },
	{ "a0102.rom",     0x400000, 0x069e2c38, 3 | BRF_GRA },
	{ "a0103.rom",     0x400000, 0x4460a3fd, 3 | BRF_GRA },
	{ "a0104.rom",     0x400000, 0x5f8abb56, 3 | BRF_GRA },
	{ "a0105.rom",     0x400000, 0xa17a7147, 3 | BRF_GRA },

	{ "b0100.rom",     0x400000, 0x69d2e48c, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0101.rom",     0x400000, 0x0d587bf3, 4 | BRF_GRA },
	{ "b0102.rom",     0x400000, 0x43823c1e, 4 | BRF_GRA },

	{ "m0100.rom",     0x200000, 0xe5c36c83, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)	
};

STDROMPICKEXT(orlegnde, orlegnde, pgm);
STD_ROM_FN(orlegnde);

struct BurnDriver BurnDrvOrlegnde = {
	"orlegnde", "orlegend", "pgm", "1997",
	"Oriental Legend (ver. 112)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, orlegndeRomInfo, orlegndeRomName, pgmInputInfo, orlegndcDIPInfo,
	pgmInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Oriental Legend \ Xi Yo Gi Shi Re Zuang (ver. 112, Chinese Board)

static struct BurnRomInfo orlegndcRomDesc[] = {
	{ "p0101.160",     0x200000, 0xb24f0c1e, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "t0100.rom",     0x400000, 0x61425e1e, 2 | BRF_GRA },		  // 32x32 BG Tiles

	{ "a0100.rom",     0x400000, 0x8b3bd88a, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0101.rom",     0x400000, 0x3b9e9644, 3 | BRF_GRA },
	{ "a0102.rom",     0x400000, 0x069e2c38, 3 | BRF_GRA },
	{ "a0103.rom",     0x400000, 0x4460a3fd, 3 | BRF_GRA },
	{ "a0104.rom",     0x400000, 0x5f8abb56, 3 | BRF_GRA },
	{ "a0105.rom",     0x400000, 0xa17a7147, 3 | BRF_GRA },

	{ "b0100.rom",     0x400000, 0x69d2e48c, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0101.rom",     0x400000, 0x0d587bf3, 4 | BRF_GRA },
	{ "b0102.rom",     0x400000, 0x43823c1e, 4 | BRF_GRA },

	{ "m0100.rom",     0x200000, 0xe5c36c83, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)	
};

STDROMPICKEXT(orlegndc, orlegndc, pgm);
STD_ROM_FN(orlegndc);

struct BurnDriver BurnDrvOrlegndc = {
	"orlegndc", "orlegend", "pgm", "1997",
	"Oriental Legend\0Xi Yo Gi Shi Re Zuang (ver. 112, Chinese Board)\0", NULL, "IGS", "PGM",
	L"Oriental Legend\0\u897F\u6E38\u91CA\u5384\u4F20 (ver. 112, Chinese Board)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, orlegndcRomInfo, orlegndcRomName, pgmInputInfo, orlegndcDIPInfo,
	pgmInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Oriental Legend \ Xi Yo Gi Shi Re Zuang (ver. 111, Chinese Board)

static struct BurnRomInfo orld111cRomDesc[] = {
	{ "olv111ch.u6",   0x080000, 0x5fb86373, 1 | BRF_ESS | BRF_PRG }, // 68000
	{ "olv111ch.u9",   0x080000, 0x83cf09c8, 1 | BRF_ESS | BRF_PRG },
	{ "olv111ch.u7",   0x080000, 0x6ee79faf, 1 | BRF_ESS | BRF_PRG },
	{ "olv111ch.u11",  0x080000, 0xb80ddd3c, 1 | BRF_ESS | BRF_PRG },

	{ "t0100.rom",     0x400000, 0x61425e1e, 2 | BRF_GRA },		  // 32x32 BG Tiles

	{ "a0100.rom",     0x400000, 0x8b3bd88a, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0101.rom",     0x400000, 0x3b9e9644, 3 | BRF_GRA },
	{ "a0102.rom",     0x400000, 0x069e2c38, 3 | BRF_GRA },
	{ "a0103.rom",     0x400000, 0x4460a3fd, 3 | BRF_GRA },
	{ "a0104.rom",     0x400000, 0x5f8abb56, 3 | BRF_GRA },
	{ "a0105.rom",     0x400000, 0xa17a7147, 3 | BRF_GRA },

	{ "b0100.rom",     0x400000, 0x69d2e48c, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0101.rom",     0x400000, 0x0d587bf3, 4 | BRF_GRA },
	{ "b0102.rom",     0x400000, 0x43823c1e, 4 | BRF_GRA },

	{ "m0100.rom",     0x200000, 0xe5c36c83, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)	
};

STDROMPICKEXT(orld111c, orld111c, pgm);
STD_ROM_FN(orld111c);

struct BurnDriver BurnDrvOrld111c = {
	"orld111c", "orlegend", "pgm", "1997",
	"Oriental Legend\0Xi Yo Gi Shi Re Zuang (ver. 111, Chinese Board)\0", NULL, "IGS", "PGM",
	L"Oriental Legend\0\u897F\u6E38\u91CA\u5384\u4F20 (ver. 111, Chinese Board)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, orld111cRomInfo, orld111cRomName, pgmInputInfo, orld111cDIPInfo,
	pgmInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Oriental Legend (ver. 105, Korean Board)

static struct BurnRomInfo orld105kRomDesc[] = {
	{ "olv105ko.u6",   0x080000, 0xb86703fe, 1 | BRF_ESS | BRF_PRG }, // 68000
	{ "olv105ko.u9",   0x080000, 0x5a108e39, 1 | BRF_ESS | BRF_PRG },
	{ "olv105ko.u7",   0x080000, 0x5712facc, 1 | BRF_ESS | BRF_PRG },
	{ "olv105ko.u11",  0x080000, 0x40ae4d9e, 1 | BRF_ESS | BRF_PRG },

	{ "t0100.rom",     0x400000, 0x61425e1e, 2 | BRF_GRA },		  // 32x32 BG Tiles

	{ "a0100.rom",     0x400000, 0x8b3bd88a, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0101.rom",     0x400000, 0x3b9e9644, 3 | BRF_GRA },
	{ "a0102.rom",     0x400000, 0x069e2c38, 3 | BRF_GRA },
	{ "a0103.rom",     0x400000, 0x4460a3fd, 3 | BRF_GRA },
	{ "a0104.rom",     0x400000, 0x5f8abb56, 3 | BRF_GRA },
	{ "a0105.rom",     0x400000, 0xa17a7147, 3 | BRF_GRA },

	{ "b0100.rom",     0x400000, 0x69d2e48c, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0101.rom",     0x400000, 0x0d587bf3, 4 | BRF_GRA },
	{ "b0102.rom",     0x400000, 0x43823c1e, 4 | BRF_GRA },

	{ "m0100.rom",     0x200000, 0xe5c36c83, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)	
};

STDROMPICKEXT(orld105k, orld105k, pgm);
STD_ROM_FN(orld105k);

struct BurnDriver BurnDrvOrld105k = {
	"orld105k", "orlegend", "pgm", "1997",
	"Oriental Legend (ver. 105, Korean Board)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, orld105kRomInfo, orld105kRomName, pgmInputInfo, orld105kDIPInfo,
	pgmInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Dragon World II (ver. 110X, Export)

static struct BurnRomInfo drgw2RomDesc[] = {
	{ "v-110x.u2",     0x080000, 0x1978106b, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "pgmt0200.u7",   0x400000, 0xb0f6534d, 2 | BRF_GRA },				// 32x32 BG Tiles

	{ "pgma0200.u5",   0x400000, 0x13b95069, 3 | BRF_GRA },				// Sprite Colour Data

	{ "pgmb0200.u9",   0x400000, 0x932d0f13, 4 | BRF_GRA },				// Sprite Masks + Colour Indexes

	// No samples
};

STDROMPICKEXT(drgw2, drgw2, pgm);
STD_ROM_FN(drgw2);

void drgw2_patch()
{
	pgm_dw2_decrypt();

	// These ROM patches are not hacks, the protection device
	// overlays the normal ROM code, this has been confirmed on a real PCB
	// although some addresses may be missing

	if (strcmp(BurnDrvGetTextA(DRV_NAME), "drgw2") == 0) {
		*((unsigned short*)(PGM68KROM + 0x031098)) = 0x4e93;
		*((unsigned short*)(PGM68KROM + 0x03113e)) = 0x4e93;
		*((unsigned short*)(PGM68KROM + 0x0311ce)) = 0x4e93;
	}

	if (strcmp(BurnDrvGetTextA(DRV_NAME), "drgw2c") == 0) {
		*((unsigned short*)(PGM68KROM + 0x0303bc)) = 0x4e93;
		*((unsigned short*)(PGM68KROM + 0x030462)) = 0x4e93;
		*((unsigned short*)(PGM68KROM + 0x0304F2)) = 0x4e93;
	}

	if (strcmp(BurnDrvGetTextA(DRV_NAME), "drgw2j") == 0) {
		*((unsigned short*)(PGM68KROM + 0x0302C0)) = 0x4e93;
		*((unsigned short*)(PGM68KROM + 0x030366)) = 0x4e93;
		*((unsigned short*)(PGM68KROM + 0x0303F6)) = 0x4e93;
	}
}

int drgw2Init()
{
	pPgmInitCallback = drgw2_patch;

	return pgmInit();
}

struct BurnDriver BurnDrvdrgw2 = {
	"drgw2", NULL, "pgm", "1997",
	"Dragon World II (ver. 110X, Export)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, drgw2RomInfo, drgw2RomName, pgmInputInfo, pgmDIPInfo,
	drgw2Init,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Zhong Guo Long II (ver. 100C, China)

static struct BurnRomInfo drgw2cRomDesc[] = {
	{ "v-100c.u2",     0x080000, 0x67467981, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "pgmt0200.u7",   0x400000, 0xb0f6534d, 2 | BRF_GRA },		  // 32x32 BG Tiles

	{ "pgma0200.u5",   0x400000, 0x13b95069, 3 | BRF_GRA },		  // Sprite Colour Data

	{ "pgmb0200.u9",   0x400000, 0x932d0f13, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes

	// No samples
};

STDROMPICKEXT(drgw2c, drgw2c, pgm);
STD_ROM_FN(drgw2c);

struct BurnDriver BurnDrvdrgw2c = {
	"drgw2c", "drgw2", "pgm", "1997",
	"Zhong Guo Long II (ver. 100C, China)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, drgw2cRomInfo, drgw2cRomName, pgmInputInfo, pgmDIPInfo,
	drgw2Init,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Chuugokuryuu II (ver. 100J, Japan)

static struct BurnRomInfo drgw2jRomDesc[] = {
	{ "v-100j.u2",     	  0x080000, 0xf8f8393e, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "pgmt0200.u7",   	  0x400000, 0xb0f6534d, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "pgma0200.u5",   	  0x400000, 0x13b95069, 3 | BRF_GRA },		 // Sprite Colour Data

	{ "pgmb0200.u9",   	  0x400000, 0x932d0f13, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes

	// No samples
};

STDROMPICKEXT(drgw2j, drgw2j, pgm);
STD_ROM_FN(drgw2j);

struct BurnDriverD BurnDrvdrgw2j = {
	"drgw2j", "drgw2", "pgm", "1997",
	"Chuugokuryuu II (ver. 100J, Japan)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, drgw2jRomInfo, drgw2jRomName, pgmInputInfo, pgmDIPInfo,
	drgw2Init,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// The Killing Blade

static struct BurnRomInfo killbldRomDesc[] = {
	{ "kb_u3_v104.u3", 0x080000, 0x6db1d719, 1 | BRF_ESS | BRF_PRG }, // 68000
	{ "kb_u6_v104.u6", 0x080000, 0x31ecc978, 1 | BRF_ESS | BRF_PRG },
	{ "kb_u4_v104.u4", 0x080000, 0x1ed8b2e7, 1 | BRF_ESS | BRF_PRG }, // order?
	{ "kb_u5_v104.u5", 0x080000, 0xa0bafc29, 1 | BRF_ESS | BRF_PRG }, // order?

	{ "kb_u2_v104.u2", 0x010000, 0xc970f6d5, 7 | BRF_ESS | BRF_PRG }, // Protection Data (not used atm)

	{ "t0300.u14",	   0x400000, 0x0922f7d9, 2 | BRF_GRA },		  // 32x32 BG Tiles

	{ "a0300.u9",      0x400000, 0x3f9455d3, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0301.u10",	   0x400000, 0x92776889, 3 | BRF_GRA },
	{ "a0303.u11",	   0x400000, 0x33f5cc69, 3 | BRF_GRA },
	{ "a0306.u12",	   0x400000, 0xcc018a8e, 3 | BRF_GRA },
	{ "a0307.u2",	   0x400000, 0xbc772e39, 3 | BRF_GRA },

	{ "b0300.u13",	   0x400000, 0x7f876981, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0302.u14",	   0x400000, 0xeea9c502, 4 | BRF_GRA },
	{ "b0303.u15",	   0x200000, 0x77a9652e, 4 | BRF_GRA },

	{ "m0300.u1",      0x400000, 0x93159695, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)

};

STDROMPICKEXT(killbld, killbld, pgm);
STD_ROM_FN(killbld);

struct BurnDriver BurnDrvkillbld = {
	"killbld", NULL, "pgm", "1998",
	"The Killing Blade\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, killbldRomInfo, killbldRomName, pgmInputInfo, pgmDIPInfo,
	pgmInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// The Killing Blade (Chinese Board)

static struct BurnRomInfo killbldtRomDesc[] = {
	{ "kb.u9",         0x200000, 0x43da77d7, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "kb_u2.rom",     0x010000, 0xde3eae63, 7 | BRF_ESS | BRF_PRG }, // Protection Data

	{ "t0300.u14",     0x400000, 0x0922f7d9, 2 | BRF_GRA },		  // 32x32 BG Tiles

	{ "a0300.u9",      0x400000, 0x3f9455d3, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0301.u10",     0x400000, 0x92776889, 3 | BRF_GRA },
	{ "a0303.u11",     0x400000, 0x33f5cc69, 3 | BRF_GRA },
	{ "a0306.u12",     0x400000, 0xcc018a8e, 3 | BRF_GRA },
	{ "a0307.u2",      0x400000, 0xbc772e39, 3 | BRF_GRA },

	{ "b0300.u13",     0x400000, 0x7f876981, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0302.u14",     0x400000, 0xeea9c502, 4 | BRF_GRA },
	{ "b0303.u15",     0x200000, 0x77a9652e, 4 | BRF_GRA },

	{ "m0300.u1",      0x400000, 0x93159695, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)

	{ "kb.ram",        0x004000, 0x6994c507, 0 | BRF_ESS }, 	  // dump of RAM shared with protection device, todo, emulate protection device instead!
};

STDROMPICKEXT(killbldt, killbldt, pgm);
STD_ROM_FN(killbldt);

void killbldt_patch()
{
	pgm_killbld_decrypt();

	// this isn't a hack.. doing a rom dump while the game is running shows the
	// rom space to look like this.. there may be more overlays / enables though
	// the game actually performs a CRC check of the rom during the 'Please Wait'
	// screen, the checksum expected is that of the patched rom.  if the checksum
	// fails the please wait screen doesn't last as long and the region supplied
	// by the protection device is ignored and the attract sequence appears out
	// of order

	*((unsigned short *)(PGM68KROM + 0x008a2c)) = 0xB6AA;
	*((unsigned short *)(PGM68KROM + 0x008a30)) = 0x6610;
	*((unsigned short *)(PGM68KROM + 0x008a32)) = 0x13c2;
	*((unsigned short *)(PGM68KROM + 0x008a34)) = 0x0080;
	*((unsigned short *)(PGM68KROM + 0x008a36)) = 0x9c76;
	*((unsigned short *)(PGM68KROM + 0x008a38)) = 0x23c3;
	*((unsigned short *)(PGM68KROM + 0x008a3a)) = 0x0080;
	*((unsigned short *)(PGM68KROM + 0x008a3c)) = 0x9c78;
	*((unsigned short *)(PGM68KROM + 0x008a3e)) = 0x1002;
	*((unsigned short *)(PGM68KROM + 0x008a40)) = 0x6054;
	*((unsigned short *)(PGM68KROM + 0x008a42)) = 0x5202;
	*((unsigned short *)(PGM68KROM + 0x008a44)) = 0x0c02;

	USER1 = USER0 + 0x10000;
	USER2 = USER1 + 0x10000;

	killbld_sharedprotram = (unsigned short*)USER0;

	BurnLoadRom(USER1,  1, 1); // load protection data
	BurnLoadRom(USER2, 12, 1); // load ram dump
}

static int killbldtInit()
{
	int nRet;

	pPgmInitCallback = killbldt_patch;
	pPgmScanCallback = killbldtScan;

	nRet = pgmInit();

	/* fill the protection ram with a5 */
	memset(USER0, 0xa5, 0x4000);

	SekOpen(0);
	SekMapMemory(USER0,	0x300000, 0x303fff, SM_RAM);
	SekClose();

	return nRet;
}

struct BurnDriver BurnDrvkillbldt = {
	"killbldt", "killbld", "pgm", "1998",
	"The Killing Blade (Chinese Board)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, killbldtRomInfo, killbldtRomName, pgmInputInfo, killbldDIPInfo,
	killbldtInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Dragon World 3

static struct BurnRomInfo drgw3RomDesc[] = {
	{ "dw3_v100.u12",	  0x080000, 0x47243906, 1 | BRF_ESS | BRF_PRG }, // 68000
	{ "dw3_v100.u13",	  0x080000, 0xb7cded21, 1 | BRF_ESS | BRF_PRG },

	{ "dw3_v100.u15",	  0x010000, 0x03dc4fdf, 7 | BRF_ESS | BRF_PRG }, // Protection data (not used)

	{ "dw3t0400.u18",	  0x400000, 0xb70f3357, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "dw3a0400.u9",	  0x400000, 0xdd7bfd40, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "dw3a0401.u10",	  0x400000, 0xcab6557f, 3 | BRF_GRA },

	{ "dw3b0400.u13",	  0x400000, 0x4bb87cc0, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes

	{ "dw3m0400.u1",	  0x400000, 0x031eb9ce, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(drgw3, drgw3, pgm);
STD_ROM_FN(drgw3);

int drgw3Init()
{
	pPgmInitCallback = pgm_dw3_decrypt;

	return pgmInit();
}

struct BurnDriverD BurnDrvdrgw3 = {
	"drgw3", NULL, "pgm", "1998",
	"Dragon World 3\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, drgw3RomInfo, drgw3RomName, pgmInputInfo, pgmDIPInfo,
	drgw3Init,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Dragon World 3 (Korean Board)

static struct BurnRomInfo drgw3kRomDesc[] = {
	{ "dw3_v106.u12",	  0x080000, 0xc3f6838b, 1 | BRF_ESS | BRF_PRG }, // 68000
	{ "dw3_v106.u13",	  0x080000, 0x28284e22, 1 | BRF_ESS | BRF_PRG },

	{ "dw3_v100.u15",	  0x010000, 0x03dc4fdf, 7 | BRF_ESS | BRF_PRG }, // Protection data (not used)

	{ "dw3t0400.u18",	  0x400000, 0xb70f3357, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "dw3a0400.u9",	  0x400000, 0xdd7bfd40, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "dw3a0401.u10",	  0x400000, 0xcab6557f, 3 | BRF_GRA },

	{ "dw3b0400.u13",	  0x400000, 0x4bb87cc0, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes

	{ "dw3m0400.u1",	  0x400000, 0x031eb9ce, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(drgw3k, drgw3k, pgm);
STD_ROM_FN(drgw3k);

struct BurnDriverD BurnDrvdrgw3k = {
	"drgw3k", "drgw3", "pgm", "1998",
	"Dragon World 3 (Korean Board)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, drgw3kRomInfo, drgw3kRomName, pgmInputInfo, pgmDIPInfo,
	drgw3Init,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Oriental Legend Super / Special (Korea 101)

static struct BurnRomInfo oldsRomDesc[] = {
	{ "sp_v101.u2",		  0x080000, 0x08eb9661, 1 | BRF_ESS | BRF_PRG }, // 68000
	{ "sp_v101.u3",		  0x080000, 0x0a358c1e, 1 | BRF_ESS | BRF_PRG },
	{ "sp_v101.u4",		  0x080000, 0x766570e0, 1 | BRF_ESS | BRF_PRG },
	{ "sp_v101.u5",		  0x080000, 0x58662e12, 1 | BRF_ESS | BRF_PRG },
	{ "sp_v101.u1",		  0x080000, 0x2b2f4f1e, 1 | BRF_ESS | BRF_PRG },

	{ "sp_v101.u6",		  0x010000, 0x097046bc, 7 | BRF_ESS | BRF_PRG }, // protection rom (not used atm)

	{ "t0500.rom",		  0x400000, 0xd881726c, 2 | BRF_GRA },		 // 32x32 BG Tiles
	{ "t0501.rom",		  0x200000, 0xd2106864, 2 | BRF_GRA },

	{ "a0500.rom",		  0x400000, 0x80a59197, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "a0501.rom",		  0x400000, 0x98c931b1, 3 | BRF_GRA },
	{ "a0502.rom",		  0x400000, 0xc3fcdf1d, 3 | BRF_GRA },
	{ "a0503.rom",		  0x400000, 0x066dffec, 3 | BRF_GRA },
	{ "a0504.rom",		  0x400000, 0x45337583, 3 | BRF_GRA },
	{ "a0505.rom",		  0x400000, 0x5b8cf3a5, 3 | BRF_GRA },
	{ "a0506.rom",		  0x400000, 0x087ac60c, 3 | BRF_GRA },

	{ "b0500.rom",		  0x400000, 0xcde07f74, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes
	{ "b0501.rom",		  0x400000, 0x1546c2e9, 4 | BRF_GRA },
	{ "b0502.rom",		  0x400000, 0xe97b31c3, 4 | BRF_GRA },
	{ "b0503.u16",		  0x400000, 0xe41d98e4, 4 | BRF_GRA },

	{ "m0500.rom",		  0x200000, 0x37928cdd, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(olds, olds, pgm);
STD_ROM_FN(olds);

void olds_protram()
{
	BurnLoadRom(USER0, 16, 1);
	BurnByteswap(USER0, 0x200000);
}

int oldsInit()
{
	// only olds100a supported
	if (strcmp(BurnDrvGetTextA(DRV_NAME), "olds100a") != 0)
		return 1;

	int nRet;

	pPgmInitCallback = olds_protram;
	pPgmScanCallback = oldsScan;

	nRet = pgmInit();

	SekOpen(0);
	SekMapMemory(USER0 + 0x100000,	0x400000, 0x4FFFFF, SM_RAM);
	SekClose();

	return nRet;
}

struct BurnDriverD BurnDrvolds = {
	"olds", NULL, "pgm", "1999",
	"Oriental Legend Super / Special (Korea 101)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, oldsRomInfo, oldsRomName, pgmInputInfo, sangoDIPInfo,
	oldsInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Oriental Legend Super / Special (100)

static struct BurnRomInfo olds100RomDesc[] = {
	{ "v100-u2.040",	  0x080000, 0x517c2a06, 1 | BRF_ESS | BRF_PRG }, // 68000
	{ "v100-u3.040",	  0x080000, 0xd0e2b741, 1 | BRF_ESS | BRF_PRG },
	{ "v100-u4.040",	  0x080000, 0x32a6bdbd, 1 | BRF_ESS | BRF_PRG },
	{ "v100-u5.040",	  0x080000, 0xb4a1cafb, 1 | BRF_ESS | BRF_PRG },

	{ "kd-u6.512",		  0x010000, 0xe7613dda, 7 | BRF_ESS | BRF_PRG }, // protection rom (not used atm)

	{ "t0500.rom",		  0x400000, 0xd881726c, 2 | BRF_GRA },		 // 32x32 BG Tiles
	{ "t0501.rom",		  0x200000, 0xd2106864, 2 | BRF_GRA },

	{ "a0500.rom",		  0x400000, 0x80a59197, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "a0501.rom",		  0x400000, 0x98c931b1, 3 | BRF_GRA },
	{ "a0502.rom",		  0x400000, 0xc3fcdf1d, 3 | BRF_GRA },
	{ "a0503.rom",		  0x400000, 0x066dffec, 3 | BRF_GRA },
	{ "a0504.rom",		  0x400000, 0x45337583, 3 | BRF_GRA },
	{ "a0505.rom",		  0x400000, 0x5b8cf3a5, 3 | BRF_GRA },
	{ "a0506.rom",		  0x400000, 0x087ac60c, 3 | BRF_GRA },

	{ "b0500.rom",		  0x400000, 0xcde07f74, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes
	{ "b0501.rom",		  0x400000, 0x1546c2e9, 4 | BRF_GRA },
	{ "b0502.rom",		  0x400000, 0xe97b31c3, 4 | BRF_GRA },
	{ "b0503.u16",		  0x400000, 0xe41d98e4, 4 | BRF_GRA },

	{ "m0500.rom",		  0x200000, 0x37928cdd, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(olds100, olds100, pgm);
STD_ROM_FN(olds100);

struct BurnDriverD BurnDrvolds100 = {
	"olds100", "olds", "pgm", "1999",
	"Oriental Legend Super / Special (100)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, olds100RomInfo, olds100RomName, pgmInputInfo, sangoDIPInfo,
	oldsInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Oriental Legend Super / Special (100 alt)

static struct BurnRomInfo olds100aRomDesc[] = {
	{ "p0500.v10",	  	  0x400000, 0x8981fc87, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "protection_data.u6",	  0x010000, 0, 		7 | BRF_NODUMP | BRF_PRG }, // protection rom (not used atm)

	{ "t0500.rom",		  0x400000, 0xd881726c, 2 | BRF_GRA },		 // 32x32 BG Tiles
	{ "t0501.rom",		  0x200000, 0xd2106864, 2 | BRF_GRA },

	{ "a0500.rom",		  0x400000, 0x80a59197, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "a0501.rom",		  0x400000, 0x98c931b1, 3 | BRF_GRA },
	{ "a0502.rom",		  0x400000, 0xc3fcdf1d, 3 | BRF_GRA },
	{ "a0503.rom",		  0x400000, 0x066dffec, 3 | BRF_GRA },
	{ "a0504.rom",		  0x400000, 0x45337583, 3 | BRF_GRA },
	{ "a0505.rom",		  0x400000, 0x5b8cf3a5, 3 | BRF_GRA },
	{ "a0506.rom",		  0x400000, 0x087ac60c, 3 | BRF_GRA },

	{ "b0500.rom",		  0x400000, 0xcde07f74, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes
	{ "b0501.rom",		  0x400000, 0x1546c2e9, 4 | BRF_GRA },
	{ "b0502.rom",		  0x400000, 0xe97b31c3, 4 | BRF_GRA },
	{ "b0503.u16",		  0x400000, 0xe41d98e4, 4 | BRF_GRA },

	{ "m0500.rom",		  0x200000, 0x37928cdd, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)

	{ "ram_dump", 		  0x200000, 0xe7b26aea, 0 | BRF_ESS },		 // ram dump
};

STDROMPICKEXT(olds100a, olds100a, pgm);
STD_ROM_FN(olds100a);

struct BurnDriverD BurnDrvolds100a = {
	"olds100a", "olds", "pgm", "1999",
	"Oriental Legend Super / Special (100 alt)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, olds100aRomInfo, olds100aRomName, pgmInputInfo, sangoDIPInfo,
	oldsInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Photo Y2K

static struct BurnRomInfo photoy2kRomDesc[] = {
	{ "v104.16m",      0x200000, 0xe051070f, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "t0700.rom",     0x080000, 0x93943b4d, 2 | BRF_GRA },		  // 32x32 BG Tiles

	{ "a0700.l",       0x400000, 0x26a9ae9c, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0700.h",       0x400000, 0x79bc1fc1, 3 | BRF_GRA },
	{ "a0701.l",       0x400000, 0x23607f81, 3 | BRF_GRA },
	{ "a0701.h",       0x400000, 0x5f2efd37, 3 | BRF_GRA },
	{ "a0702.rom",     0x080000, 0x42239e1b, 3 | BRF_GRA },

	{ "b0700.l",       0x400000, 0xaf096904, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0700.h",       0x400000, 0x6d53de26, 4 | BRF_GRA },
	{ "cgv101.rom",    0x020000, 0xda02ec3e, 4 | BRF_GRA },

	{ "m0700.rom",     0x080000, 0xacc7afce, 5 | BRF_SND },		   // Samples - (8 bit mono 11025Hz)
	
	{ "photoy2k_v100_china.asic", 0x004000,  0x6dd7f257, 7 | BRF_OPT | BRF_PRG  }, // ARM protection ASIC (Not used ATM)
};

STDROMPICKEXT(photoy2k, photoy2k, pgm);
STD_ROM_FN(photoy2k);

static int photoy2kInit()
{
	pPgmInitCallback = pgm_djlzz_decrypt;

	return pgmInit();
}

struct BurnDriver BurnDrvphotoy2k = {
	"photoy2k", NULL, "pgm", "1999",
	"Photo Y2K\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, photoy2kRomInfo, photoy2kRomName, pgmInputInfo, sangoDIPInfo,
	photoy2kInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Real and Fake / Photo Y2K (ver. 102, Japan Board)

static struct BurnRomInfo raf102jRomDesc[] = {
	{ "v102.u4",       0x080000, 0xa65eda9f, 1 | BRF_ESS | BRF_PRG }, // 68000
	{ "v102.u6",       0x080000, 0xb9ca5504, 1 | BRF_ESS | BRF_PRG },
	{ "v102.u5",       0x080000, 0x9201621b, 1 | BRF_ESS | BRF_PRG },
	{ "v102.u8",       0x080000, 0x3be22b8f, 1 | BRF_ESS | BRF_PRG },

	{ "t0700.rom",     0x080000, 0x93943b4d, 2 | BRF_GRA },		  // 32x32 BG Tiles

	{ "a0700.l",       0x400000, 0x26a9ae9c, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0700.h",       0x400000, 0x79bc1fc1, 3 | BRF_GRA },
	{ "a0701.l",       0x400000, 0x23607f81, 3 | BRF_GRA },
	{ "a0701.h",       0x400000, 0x5f2efd37, 3 | BRF_GRA },
	{ "a0702.rom",     0x080000, 0x42239e1b, 3 | BRF_GRA },

	{ "b0700.l",       0x400000, 0xaf096904, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0700.h",       0x400000, 0x6d53de26, 4 | BRF_GRA },
	{ "cgv101.rom",    0x020000, 0xda02ec3e, 4 | BRF_GRA },

	{ "m0700.rom",     0x080000, 0xacc7afce, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(raf102j, raf102j, pgm);
STD_ROM_FN(raf102j);

struct BurnDriver BurnDrvraf102j = {
	"raf102j", "photoy2k", "pgm", "1999",
	"Real and Fake / Photo Y2K (ver. 102, Japan Board)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE, 4, HARDWARE_IGS_PGM,
	NULL, raf102jRomInfo, raf102jRomName, pgmInputInfo, sangoDIPInfo,
	photoy2kInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Knights of Valour \ Sangoku Senki (ver 117)

static struct BurnRomInfo kovRomDesc[] = {
	{ "p0600.117",     0x400000, 0xc4d19fe6, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  // 32x32 BG Tiles
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },
	{ "a0603.rom",     0x400000, 0xec31abda, 3 | BRF_GRA },
	
	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0601.rom",     0x400000, 0xa0bb1c2f, 4 | BRF_GRA },
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(kov, kov, pgm);
STD_ROM_FN(kov);

static int kovInit()
{
	pPgmInitCallback = pgm_kov_decrypt;

	return pgmInit();
}

struct BurnDriver BurnDrvKov = {
	"kov", NULL, "pgm", "1999",
	"Knights of Valour\0Sangoku Senki (ver. 117)\0", NULL, "IGS", "PGM",
	L"Knights of Valour\0\u4E09\u56FD\u6218\u7EAA (ver. 117)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, kovRomInfo, kovRomName, pgmInputInfo, sangoDIPInfo,
	kovInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Knights of Valour \ Sangoku Senki (ver. 115)

static struct BurnRomInfo kov115RomDesc[] = {
	{ "p0600.115",     0x400000, 0x527a2924, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  // 32x32 BG Tiles
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },
	{ "a0603.rom",     0x400000, 0xec31abda, 3 | BRF_GRA },
	
	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0601.rom",     0x400000, 0xa0bb1c2f, 4 | BRF_GRA },
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(kov115, kov115, pgm);
STD_ROM_FN(kov115);

struct BurnDriver BurnDrvKov115 = {
	"kov115", "kov", "pgm", "1999",
	"Knights of Valour\0Sangoku Senki (ver. 115)\0", NULL, "IGS", "PGM",
	L"Knights of Valour\0\u4E09\u56FD\u6218\u7EAA (ver. 115)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, kov115RomInfo, kov115RomName, pgmInputInfo, sangoDIPInfo,
	kovInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Knights of Valour Plus \ Sangoku Senki (ver. 100, Japanese Board)

static struct BurnRomInfo kovjRomDesc[] = {
	{ "sav111.u4",     0x080000, 0xae2f1b4e, 1 | BRF_ESS | BRF_PRG }, // 68000
	{ "sav111.u7",     0x080000, 0x95eedf0e, 1 | BRF_ESS | BRF_PRG },
	{ "sav111.u5",     0x080000, 0x5fdd4aa8, 1 | BRF_ESS | BRF_PRG },
	{ "sav111.u8",     0x080000, 0x003cbf49, 1 | BRF_ESS | BRF_PRG },
	{ "sav111.u10",    0x080000, 0xd5536107, 1 | BRF_ESS | BRF_PRG },

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  // 32x32 BG Tiles
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },
	{ "a0603.rom",     0x400000, 0xec31abda, 3 | BRF_GRA },
	
	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes
	{ "b0601.rom",     0x400000, 0xa0bb1c2f, 4 | BRF_GRA },
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)	
};

STDROMPICKEXT(kovj, kovj, pgm);
STD_ROM_FN(kovj);

struct BurnDriver BurnDrvKovj = {
	"kovj", "kov", "pgm", "1999",
	"Knights of Valour\0Sangoku Senki (ver. 100, Japanese Board)\0", NULL, "IGS", "PGM",
	L"Knights of Valour\0\u4E09\u56FD\u6226\u7D00 (ver. 100, Japanese Board)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, kovjRomInfo, kovjRomName, pgmInputInfo, kovjDIPInfo,
	kovInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Knights of Valour Plus \ Sangoku Senki Plus (ver. 119)

static struct BurnRomInfo kovplusRomDesc[] = {
	{ "p0600.119",     0x400000, 0xe4b0875d, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  // 32x32 BG Tiles
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },
	{ "a0603.rom",     0x400000, 0xec31abda, 3 | BRF_GRA },
	
	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0601.rom",     0x400000, 0xa0bb1c2f, 4 | BRF_GRA },
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)	
};

STDROMPICKEXT(kovplus, kovplus, pgm);
STD_ROM_FN(kovplus);

struct BurnDriver BurnDrvKovplus = {
	"kovplus", "kov", "pgm", "1999",
	"Knights of Valour Plus\0Sangoku Senki Plus (ver. 119)\0", NULL, "IGS", "PGM",
	L"Knights of Valour Plus\0\u4E09\u56FD\u6218\u7EAA Plus (ver. 119)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, kovplusRomInfo, kovplusRomName, pgmInputInfo, sangoDIPInfo,
	kovInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Knights of Valour Plus \ Sangoku Senki Plus (alt ver. 119)

static struct BurnRomInfo kovplusaRomDesc[] = {
	{ "v119.u3",       0x080000, 0x6750388f, 1 | BRF_ESS | BRF_PRG }, // 68000
	{ "v119.u5",       0x080000, 0xd4101ffd, 1 | BRF_ESS | BRF_PRG },
	{ "v119.u4",       0x080000, 0x8200ece6, 1 | BRF_ESS | BRF_PRG },
	{ "v119.u6",       0x080000, 0x71e28f27, 1 | BRF_ESS | BRF_PRG },
	{ "v119.u2",	   0x080000, 0x29588ef2, 1 | BRF_ESS | BRF_PRG },

	{ "t0600.rom",     0x800000, 0x4acc1ad6, 2 | BRF_GRA },		  // 32x32 BG Tiles
	
	{ "a0600.rom",     0x800000, 0xd8167834, 3 | BRF_GRA },		  // Sprite Colour Data
	{ "a0601.rom",     0x800000, 0xff7a4373, 3 | BRF_GRA },
	{ "a0602.rom",     0x800000, 0xe7a32959, 3 | BRF_GRA },
	{ "a0603.rom",     0x400000, 0xec31abda, 3 | BRF_GRA },
	
	{ "b0600.rom",     0x800000, 0x7d3cd059, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes
	{ "b0601.rom",     0x400000, 0xa0bb1c2f, 4 | BRF_GRA },
	
	{ "m0600.rom",     0x400000, 0x3ada4fd6, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)	
};

STDROMPICKEXT(kovplusa, kovplusa, pgm);
STD_ROM_FN(kovplusa);

struct BurnDriver BurnDrvKovplusa = {
	"kovplusa", "kov", "pgm", "1999",
	"Knights of Valour Plus\0Sangoku Senki Plus (alt ver. 119)\0", NULL, "IGS", "PGM",
	L"Knights of Valour Plus\0\u4E09\u56FD\u6218\u7EAA Plus (alt ver. 119)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, kovplusaRomInfo, kovplusaRomName, pgmInputInfo, sangoDIPInfo,
	kovInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Knights of Valour Superheroes / Sangoku Senki Superheroes (ver. 322)

static struct BurnRomInfo kovshRomDesc[] = {
	{ "p0600.322",		  0x400000, 0x7c78e5f3, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "kovsh_v100_china.asic", 0x04000, 0x0f09a5c1, 7 | BRF_ESS | BRF_PRG }, // protection rom (not used atm)

	{ "t0600.320",		  0x400000, 0x164b3c94, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "a0600.rom",		  0x800000, 0xd8167834, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "a0601.rom",		  0x800000, 0xff7a4373, 3 | BRF_GRA },
	{ "a0602.rom",		  0x800000, 0xe7a32959, 3 | BRF_GRA },
	{ "a0603.rom",		  0x400000, 0xec31abda, 3 | BRF_GRA },

	{ "b0600.rom",		  0x800000, 0x7d3cd059, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes
	{ "b0601.rom",		  0x400000, 0xa0bb1c2f, 4 | BRF_GRA },

	{ "m0600.rom",		  0x400000, 0x3ada4fd6, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(kovsh, kovsh, pgm);
STD_ROM_FN(kovsh);

int kovshInit()
{
	pPgmInitCallback = pgm_kovsh_decrypt;

	return pgmInit();
}

struct BurnDriverD BurnDrvkovsh = {
	"kovsh", "kov", "pgm", "1999",
	"Knights of Valour Superheroes / Sangoku Senki Superheroes (ver. 322)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU,
	NULL, kovshRomInfo, kovshRomName, pgmInputInfo, pgmDIPInfo,
	kovshInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Puzzle Star

static struct BurnRomInfo puzlstarRomDesc[] = {
	{ "v100mg.u1",	          0x080000, 0x5788b77d, 1 | BRF_ESS | BRF_PRG }, // 68000
	{ "v100mg.u2",	          0x080000, 0x4c79d979, 1 | BRF_ESS | BRF_PRG },

	{ "t0800.u5",	          0x200000, 0xf9d84e59, 2 | BRF_GRA }, 		 // 32x32 BG Tiles

	{ "a0800.u1",	          0x400000, 0xe1e6ec40, 3 | BRF_GRA }, 		 // Sprite Colour Data

	{ "b0800.u3",	          0x200000, 0x52e7bef5, 4 | BRF_GRA }, 		 // Sprite Masks + Colour Indexes

	{ "m0800.u2",	          0x400000, 0xe1a46541, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(puzlstar, puzlstar, pgm);
STD_ROM_FN(puzlstar);

int puzlstarInit()
{
	pPgmInitCallback = pgm_pstar_decrypt;
	pPgmScanCallback = pstarsScan;

	return pgmInit();
}

struct BurnDriver BurnDrvpuzlstar = {
	"puzlstar", NULL, "pgm", "1999",
	"Puzzle Star\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, puzlstarRomInfo, puzlstarRomName, pgmInputInfo, sangoDIPInfo,
	puzlstarInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Puzzli 2 Super

static struct BurnRomInfo puzzli2RomDesc[] = {
	{ "2sp_v200.u3",	  0x080000, 0x2a5ba8a6, 1 | BRF_ESS | BRF_PRG }, // 68000
	{ "2sp_v200.u4",	  0x080000, 0xfa5c86c1, 1 | BRF_ESS | BRF_PRG }, 

	{ "t0900.u9",	  	  0x200000, 0x70615611, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "a0900.u3",	  	  0x400000, 0x14911251, 3 | BRF_GRA },		 // Sprite Colour Data

	{ "b0900.u4",	  	  0x200000, 0x6f0638b6, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes

	{ "m0900.u2",	  	  0x400000, 0x9ea7af2e, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(puzzli2, puzzli2, pgm);
STD_ROM_FN(puzzli2);

void puzzli2_decrypt()
{
 	pgm_puzzli2_decrypt();

	// protection related?
	*((unsigned short*)(PGM68KROM + 0x0268c0)) = 0x4e71;
	*((unsigned short*)(PGM68KROM + 0x0268c2)) = 0x4e71;
	*((unsigned short*)(PGM68KROM + 0x0268c4)) = 0x4e71;
	*((unsigned short*)(PGM68KROM + 0x03877a)) = 0x4e71;
	*((unsigned short*)(PGM68KROM + 0x04cee0)) = 0x4e71;
	*((unsigned short*)(PGM68KROM + 0x0548ec)) = 0x4e71;
	*((unsigned short*)(PGM68KROM + 0x0548fc)) = 0x4e71;
	*((unsigned short*)(PGM68KROM + 0x054948)) = 0x4e71;
	*((unsigned short*)(PGM68KROM + 0x05496A)) = 0x4e71;
	*((unsigned short*)(PGM68KROM + 0x0549FA)) = 0x4e71;
	*((unsigned short*)(PGM68KROM + 0x054A0A)) = 0x4e71;

	// patch irq4 vector (irq4 should be disabled on this game? how?)
//	*((unsigned short*)(PGM68KROM + 0x000070)) = 0x0012;
//	*((unsigned short*)(PGM68KROM + 0x000072)) = 0x5D78;
}

int puzzli2Init()
{
	pPgmInitCallback = puzzli2_decrypt;

	return pgmInit();
}

struct BurnDriverD BurnDrvpuzzli2 = {
	"puzzli2", NULL, "pgm", "2001",
	"Puzzli 2 Super\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, puzzli2RomInfo, puzzli2RomName, pgmInputInfo, pgmDIPInfo,
	puzzli2Init,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Martial Masters

static struct BurnRomInfo martmastRomDesc[] = {
	{ "v104_32m.u9",  	  0x400000, 0xcfd9dff4, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "martial_masters_v102_usa.asic", 0x4000,  0xa6c0828c, 7 | BRF_ESS | BRF_PRG }, // protection rom (not used atm)
//	{ "martial_masters_v101_china.asic",0x4000, 0xb3e25b7d, 7 | BRF_ESS | BRF_PRG }, // protection rom (not used atm)

	{ "v102_16m.u10",  	  0x200000, 0x18b745e6, 8 | BRF_ESS | BRF_PRG }, // External ARM rom

	{ "t1000.u3",	  	  0x800000, 0xbbf879b5, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "a1000.u3",    	  0x800000, 0x43577ac8, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "a1001.u4",    	  0x800000, 0xfe7a476f, 3 | BRF_GRA },
	{ "a1002.u6",    	  0x800000, 0x62e33d38, 3 | BRF_GRA },
	{ "a1003.u8",    	  0x800000, 0xb2c4945a, 3 | BRF_GRA },
	{ "a1004.u10",   	  0x400000, 0x9fd3f5fd, 3 | BRF_GRA },

	{ "b1000.u9",	  	  0x800000, 0xc5961f6f, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes
	{ "b1001.u11",	  	  0x800000, 0x0b7e1c06, 4 | BRF_GRA },

	{ "m1000.u5",    	  0x800000, 0xed407ae8, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
	{ "m1001.u7",    	  0x400000, 0x662d2d48, 5 | BRF_SND },
};

STDROMPICKEXT(martmast, martmast, pgm);
STD_ROM_FN(martmast);

int martmastInit()
{
	pPgmInitCallback = pgm_mm_decrypt;

	return pgmInit();
}

struct BurnDriverD BurnDrvmartmast = {
	"martmast", NULL, "pgm", "2001",
	"Martial Masters\0", "Needs second CPU emulated (ARM7)", "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU,
	NULL, martmastRomInfo, martmastRomName, pgmInputInfo, pgmDIPInfo,
	martmastInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Knights of Valour 2

static struct BurnRomInfo kov2RomDesc[] = {
	{ "igs_u18.rom",  	  0x400000, 0x86205879, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "kov2_v100_hongkong.asic",0x4000, 0xe0d7679f, 7 | BRF_ESS | BRF_PRG }, // protection rom (not used atm)

	{ "igs_u19.rom",  	  0x200000, 0xedd59922, 8 | BRF_ESS | BRF_PRG }, // External ARM rom

	{ "t1200.rom",	  	  0x800000, 0xd7e26609, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "a1200.rom",	  	  0x800000, 0xceeb81d8, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "a1201.rom",	  	  0x800000, 0x82f0a878, 3 | BRF_GRA },
	{ "a1202.rom",	  	  0x800000, 0x4bb92fae, 3 | BRF_GRA },
	{ "a1203.rom",	  	  0x800000, 0xe73cb627, 3 | BRF_GRA },
	{ "a1204.rom",	  	  0x800000, 0x27527099, 3 | BRF_GRA },

	{ "b1200.rom",	  	  0x800000, 0xbed7d994, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes
	{ "b1201.rom",	  	  0x800000, 0xf251eb57, 4 | BRF_GRA },

	{ "m1200.rom",	  	  0x800000, 0xb0d88720, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(kov2, kov2, pgm);
STD_ROM_FN(kov2);

int kov2Init()
{
	pPgmInitCallback = pgm_kov2_decrypt;

	return pgmInit();
}

struct BurnDriverD BurnDrvkov2 = {
	"kov2", NULL, "pgm", "2000",
	"Knights of Valour 2\0", "Needs second CPU emulated (ARM7)", "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU,
	NULL, kov2RomInfo, kov2RomName, pgmInputInfo, pgmDIPInfo,
	kov2Init,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Knights of Valour 2 (106)

static struct BurnRomInfo kov2106RomDesc[] = {
	{ "u18.106",	  	  0x400000, 0x40051ad9, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "kov2_v100_hongkong.asic",0x4000, 0xe0d7679f, 7 | BRF_ESS | BRF_PRG }, // protection rom (not used atm)

	{ "u19.102",	  	  0x200000, 0x462e2980, 8 | BRF_ESS | BRF_PRG }, // External ARM rom

	{ "t1200.rom",	  	  0x800000, 0xd7e26609, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "a1200.rom",	  	  0x800000, 0xceeb81d8, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "a1201.rom",	  	  0x800000, 0x82f0a878, 3 | BRF_GRA },
	{ "a1202.rom",	  	  0x800000, 0x4bb92fae, 3 | BRF_GRA },
	{ "a1203.rom",	  	  0x800000, 0xe73cb627, 3 | BRF_GRA },
	{ "a1204.rom",	  	  0x800000, 0x27527099, 3 | BRF_GRA },

	{ "b1200.rom",	  	  0x800000, 0xbed7d994, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes
	{ "b1201.rom",	  	  0x800000, 0xf251eb57, 4 | BRF_GRA },

	{ "m1200.rom",	  	  0x800000, 0xb0d88720, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(kov2106, kov2106, pgm);
STD_ROM_FN(kov2106);

struct BurnDriverD BurnDrvkov2106 = {
	"kov2106", "kov2", "pgm", "2000",
	"Knights of Valour 2 (106)\0", "Needs second CPU emulated (ARM7)", "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU,
	NULL, kov2106RomInfo, kov2106RomName, pgmInputInfo, pgmDIPInfo,
	kov2Init,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Knights of Valour 2 Plus - Nine Dragons

static struct BurnRomInfo kov2pRomDesc[] = {
	{ "v204-32m.rom",  	  0x400000, 0x583e0650, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "kov2p.asic",		  0x004000, 0xe0d7679f, 7 | BRF_ESS | BRF_PRG }, // protection rom (not used atm)

	{ "v200-16.rom",  	  0x200000, 0x16a0c11f, 8 | BRF_ESS | BRF_PRG }, // External ARM rom

	{ "t1200.rom",	  	  0x800000, 0xd7e26609, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "a1200.rom",	  	  0x800000, 0xceeb81d8, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "a1201.rom_p",	  0x800000, 0x21063ca7, 3 | BRF_GRA },
	{ "a1202.rom",	  	  0x800000, 0x4bb92fae, 3 | BRF_GRA },
	{ "a1203.rom",	  	  0x800000, 0xe73cb627, 3 | BRF_GRA },
	{ "a1204.rom_p",  	  0x200000, 0x14b4b5bb , 3 | BRF_GRA },

	{ "b1200.rom",	  	  0x800000, 0xbed7d994, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes
	{ "b1201.rom",	  	  0x800000, 0xf251eb57, 4 | BRF_GRA },

	{ "m1200.rom",	  	  0x800000, 0xb0d88720, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(kov2p, kov2p, pgm);
STD_ROM_FN(kov2p);

struct BurnDriverD BurnDrvkov2p = {
	"kov2p", "kov2", "pgm", "2000",
	"Knights of Valour 2 Plus - Nine Dragons\0", "Needs second CPU emulated (ARM7)", "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU,
	NULL, kov2pRomInfo, kov2pRomName, pgmInputInfo, pgmDIPInfo,
	kov2Init,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Bee Storm - DoDonPachi II

static struct BurnRomInfo ddp2RomDesc[] = {
	{ "v100.u8",	  	  0x200000, 0x0c8aa8ea, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "ddp2_igs027a.bin",     0x004000, 0,		7 | BRF_NODUMP | BRF_PRG }, // protection rom (not used atm)

	{ "v100.u23", 	 	  0x020000, 0x06c3dd29, 8 | BRF_ESS | BRF_PRG }, // External ARM rom

	{ "t1300.u21",	  	  0x800000, 0xe748f0cb, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "a1300.u1",	  	  0x800000, 0xfc87a405, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "a1301.u2",	  	  0x800000, 0x0c8520da, 3 | BRF_GRA },

	{ "b1300.u7",	  	  0x800000, 0xef646604, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes

	{ "m1300.u5",	  	  0x400000, 0x82d4015d, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(ddp2, ddp2, pgm);
STD_ROM_FN(ddp2);

int ddp2Init()
{
	int nRet;

	pPgmInitCallback = pgm_ddp2_decrypt;

	nRet = pgmInit();


	return nRet;
}

struct BurnDriverD BurnDrvddp2 = {
	"ddp2", NULL, "pgm", "2001",
	"Bee Storm - DoDonPachi II\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU,
	NULL, ddp2RomInfo, ddp2RomName, pgmInputInfo, pgmDIPInfo,
	ddp2Init,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Demon Front (v102)

static struct BurnRomInfo dmnfrntRomDesc[] = {
	{ "v102_16m.u5",  	  0x200000, 0x3d4d481a, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "dmnfrnt_igs027a.bin",  0x004000, 0,  	7 | BRF_NODUMP | BRF_PRG }, // protection rom (not used atm)

	{ "v101_32m.u26",  	  0x400000, 0x93965281, 8 | BRF_ESS | BRF_PRG }, // External ARM rom

	{ "t04501.u29",	  	  0x800000, 0x900eaaac, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "a04501.u3",    	  0x800000, 0x9741bea6, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "a04502.u4",    	  0x800000, 0xe104f405, 3 | BRF_GRA },
	{ "a04503.u6",    	  0x800000, 0xbfd5cfe3, 3 | BRF_GRA },

	{ "b04501.u9",	  	  0x800000, 0x29320b7d, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes
	{ "b04502.u11",	  	  0x200000, 0x578c00e9, 4 | BRF_GRA },

	{ "w04501.u5",    	  0x800000, 0x3ab58137, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(dmnfrnt, dmnfrnt, pgm);
STD_ROM_FN(dmnfrnt);

int dmnfrntInit()
{
//	pPgmInitCallback = pgm_dmnfrnt_decrypt;

	return pgmInit();
}

struct BurnDriverD BurnDrvdmnfrnt = {
	"dmnfrnt", NULL, "pgm", "2002",
	"Demon Front (V102)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU,
	NULL, dmnfrntRomInfo, dmnfrntRomName, pgmInputInfo, pgmDIPInfo,
	dmnfrntInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Demon Front (v105)

static struct BurnRomInfo dmnfrntaRomDesc[] = {
	{ "v105_16m.u5",  	  0x200000, 0xbda083bd, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "dmnfrnt_igs027a.bin",  0x004000, 0,  	7 | BRF_NODUMP | BRF_PRG }, // protection rom (not used atm)

	{ "v105_32m.u26",  	  0x400000, 0xd200ee63, 8 | BRF_ESS | BRF_PRG }, // External ARM rom

	{ "t04501.u29",	  	  0x800000, 0x900eaaac, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "a04501.u3",    	  0x800000, 0x9741bea6, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "a04502.u4",    	  0x800000, 0xe104f405, 3 | BRF_GRA },
	{ "a04503.u6",    	  0x800000, 0xbfd5cfe3, 3 | BRF_GRA },

	{ "b04501.u9",	  	  0x800000, 0x29320b7d, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes
	{ "b04502.u11",	  	  0x200000, 0x578c00e9, 4 | BRF_GRA },

	{ "w04501.u5",    	  0x800000, 0x3ab58137, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(dmnfrnta, dmnfrnta, pgm);
STD_ROM_FN(dmnfrnta);

struct BurnDriverD BurnDrvdmnfrnta = {
	"dmnfrnta", "dmnfrnt", "pgm", "2002",
	"Demon Front (V105)\0", NULL, "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU,
	NULL, dmnfrntaRomInfo, dmnfrntaRomName, pgmInputInfo, pgmDIPInfo,
	dmnfrntInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// The Gladiator

static struct BurnRomInfo thegladRomDesc[] = {
	{ "u6.rom",	  	  0x080000, 0x14c85212, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "u2.rom",  	  	  0x200000, 0xc7bcf2ae, 8 | BRF_ESS | BRF_PRG},  // External ARM rom

	{ "t04601.u33",	  	  0x800000, 0x2da3be8e, 2 | BRF_GRA },		 // 32x32 BG Tiles

	{ "a04601.u2",    	  0x800000, 0xd9b2e004, 3 | BRF_GRA },		 // Sprite Colour Data
	{ "a04602.u4",  	  0x800000, 0x14f22308, 3 | BRF_GRA },
	{ "a04603.u6",  	  0x800000, 0x8f621e17, 3 | BRF_GRA },

	{ "b04601.u11",	  	  0x800000, 0xee72bccf, 4 | BRF_GRA },		 // Sprite Masks + Colour Indexes
	{ "b04602.u12",	  	  0x400000, 0x7dba9c38, 4 | BRF_GRA },

	{ "w04601.u1",    	  0x800000, 0x5f15ddb3, 5 | BRF_SND },		 // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(theglad, theglad, pgm);
STD_ROM_FN(theglad);

struct BurnDriverD BurnDrvtheglad = {
	"theglad", NULL, "pgm", "2001",
	"The Gladiator\0", "Needs second CPU emulated (ARM7)", "IGS", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM | HARDWARE_IGS_USE_ARM_CPU,
	NULL, thegladRomInfo, thegladRomName, pgmInputInfo, pgmDIPInfo,
	pgmInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// -----------------------------------------------------------------------------
// Homebrew

// PGM Demo (Homebrew)

static struct BurnRomInfo pgmdemoRomDesc[] = {
	{ "p0103.rom",     0x200000, 0xd3f6ec45, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "t0100.rom",     0x400000, 0x0596a59a, 2 | BRF_GRA },		  // 32x32 BG Tiles

	{ "a0100.rom",     0x400000, 0x5d0e8fa1, 3 | BRF_GRA },		  // Sprite Colour Data

	{ "b0100.rom",     0x400000, 0x15dd191f, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes

	{ "m0100.rom",     0x200000, 0x8d89877e, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(pgmdemo, pgmdemo, pgm);
STD_ROM_FN(pgmdemo);

struct BurnDriverD BurnDrvpgmdemo = {
	"pgmdemo", NULL, "pgm", "2005",
	"PGM Demo (Homebrew)\0", NULL, "Rastersoft / CDoty / Homebrew", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, pgmdemoRomInfo, pgmdemoRomName, pgmInputInfo, pgmDIPInfo,
	pgmInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};


// Frog Feast (Homebrew)

static struct BurnRomInfo pgmfrogRomDesc[] = {
	{ "p0103.rom",     0x200000, 0xCDEC9E8D, 1 | BRF_ESS | BRF_PRG }, // 68000

	{ "t0100.rom",     0x400000, 0x8F58B6D8, 2 | BRF_GRA },		  // 32x32 BG Tiles

	{ "a0100.rom",     0x400000, 0xDC1EAFE6, 3 | BRF_GRA },		  // Sprite Colour Data

	{ "b0100.rom",     0x400000, 0x3D44B66F, 4 | BRF_GRA },		  // Sprite Masks + Colour Indexes

	{ "m0100.rom",     0x200000, 0x05E2F761, 5 | BRF_SND },		  // Samples - (8 bit mono 11025Hz)
};

STDROMPICKEXT(pgmfrog, pgmfrog, pgm);
STD_ROM_FN(pgmfrog);

struct BurnDriverD BurnDrvpgmfrog = {
	"pgmfrog", NULL, "pgm", "2006",
	"Frog Feast (Homebrew)\0", NULL, "Rastersoft / CDoty / Homebrew", "PGM",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 4, HARDWARE_IGS_PGM,
	NULL, pgmfrogRomInfo, pgmfrogRomName, pgmInputInfo, pgmDIPInfo,
	pgmInit,pgmExit,pgmFrame,pgmDraw,pgmScan,&nPgmPalRecalc,
	448,224,4,3
};
