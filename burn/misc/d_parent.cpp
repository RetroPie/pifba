// A module for keeping parents who have no driver in FBA

#include "burnint.h"

static unsigned char ParentReset         = 0;

static struct BurnInputInfo ParentInputList[] = {
	{"Reset"             , BIT_DIGITAL  , &ParentReset        , "reset"     },
};

STDINPUTINFO(Parent);

static int ParentInit()
{
	return 1;
}

static int ParentExit()
{
	return 0;
}

static struct BurnRomInfo EightballactRomDesc[] = {
	{ "8b-dk.5e",           0x01000, 0x166c1c9b, BRF_ESS | BRF_PRG },
	{ "8b-dk.5c",           0x01000, 0x9ec87baa, BRF_ESS | BRF_PRG },
	{ "8b-dk.5b",           0x01000, 0xf836a962, BRF_ESS | BRF_PRG },
	{ "8b-dk.5a",           0x01000, 0xd45866d4, BRF_ESS | BRF_PRG },
	
	{ "8b-dk.3h",           0x00800, 0xa8752c60, BRF_ESS | BRF_PRG },

	{ "8b-dk.3n",           0x00800, 0x44830867, BRF_GRA },
	{ "8b-dk.3p",           0x00800, 0x6148c6f2, BRF_GRA },
	
	{ "8b-dk.7c",           0x00800, 0xe34409f5, BRF_GRA },
	{ "8b-dk.7d",           0x00800, 0xb4dc37ca, BRF_GRA },
	{ "8b-dk.7e",           0x00800, 0x655af8a8, BRF_GRA },
	{ "8b-dk.7f",           0x00800, 0xa29b2763, BRF_GRA },
	
	{ "8b.2e",              0x00100, 0xc7379a12, BRF_GRA },
	{ "8b.2f",              0x00100, 0x116612b4, BRF_GRA },
	{ "8b.2n",              0x00100, 0x30586988, BRF_GRA },
	
	{ "82s147.prm",         0x00100, 0x46e5bc92, BRF_GRA },
	
	{ "pls153h.bin",        0x000eb, 0x00000000, BRF_NODUMP },
};

STD_ROM_PICK(Eightballact);
STD_ROM_FN(Eightballact);

struct BurnDriverD BurnDrvEightballact = {
	"8ballact", NULL, NULL, "1984",
	"Eight Ball Action (DK conversion)\0", NULL, "Seatongrove Ltd (Magic Eletronics USA licence)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_MISC_MISC,
	NULL, EightballactRomInfo, EightballactRomName, ParentInputInfo, NULL,
	ParentInit, ParentExit, NULL, NULL, NULL,
	NULL, 256, 224, 4, 3
};

static struct BurnRomInfo ManiacsqRomDesc[] = {
	{ "d8-d15.1m",          0x20000, 0x9121d1b6, BRF_ESS | BRF_PRG },
	{ "d0-d7.1m",           0x20000, 0xa95cfd2a, BRF_ESS | BRF_PRG },
	
	{ "d0-d7.4m",           0x80000, 0xd8551b2f, BRF_GRA },
	{ "d8-d15.4m",          0x80000, 0xb269c427, BRF_GRA },
	{ "d16-d23.1m",         0x20000, 0xaf4ea5e7, BRF_GRA },
	{ "d24-d31.1m",         0x20000, 0x578c3588, BRF_GRA },
};

STD_ROM_PICK(Maniacsq);
STD_ROM_FN(Maniacsq);

struct BurnDriverD BurnDrvManiacsq = {
	"maniacsq", NULL, NULL, "1996",
	"Maniac Square (unprotected)\0", NULL, "Gaelco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_MISC_MISC,
	NULL, ManiacsqRomInfo, ManiacsqRomName, ParentInputInfo, NULL,
	ParentInit, ParentExit, NULL, NULL, NULL,
	NULL, 256, 224, 4, 3
};

static struct BurnRomInfo TumblepRomDesc[] = {
	{ "hl00-1.f12",         0x40000, 0xfd697c1b, BRF_ESS | BRF_PRG },
	{ "hl01-1.f13",         0x40000, 0xd5a62a3f, BRF_ESS | BRF_PRG },
	
	{ "hl02-.f16",          0x10000, 0xa5cab888, BRF_ESS | BRF_PRG },

	{ "map-02.rom",         0x80000, 0xdfceaa26, BRF_GRA },
	
	{ "map-01.rom",         0x80000, 0xe81ffa09, BRF_GRA },
	{ "map-00.rom",         0x80000, 0x8c879cfe, BRF_GRA },
	
	{ "hl03-.j15",          0x20000, 0x01b81da0, BRF_SND },
};

STD_ROM_PICK(Tumblep);
STD_ROM_FN(Tumblep);

struct BurnDriver BurnDrvTumblep = {
	"tumblep", NULL, NULL, "1991",
	"Tumble Pop (World)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_MISC_MISC,
	NULL, TumblepRomInfo, TumblepRomName, ParentInputInfo, NULL,
	ParentInit, ParentExit, NULL, NULL, NULL,
	NULL, 256, 224, 4, 3
};
