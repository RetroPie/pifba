
#include "pgm.h"
#include "pgmy2ks.h"
#include "bitswap.h"

/* ASIC3 (oriental legends protection */

static unsigned char asic3_reg, asic3_latch[3], asic3_x, asic3_y, asic3_z, asic3_h1, asic3_h2;
static unsigned short asic3_hold;

static UINT32 bt(UINT32 v, int bit)
{
	return (v & (1<<bit)) != 0;
}

static void asic3_compute_hold(void)
{
	// The mode is dependant on the region
	static int modes[4] = { 1, 1, 3, 2 };
	int mode = modes[PgmInput[7] & 3];

	switch(mode) {
	case 1:
		asic3_hold =
			(asic3_hold << 1)
			^0x2bad
			^bt(asic3_hold, 15)^bt(asic3_hold, 10)^bt(asic3_hold, 8)^bt(asic3_hold, 5)
			^bt(asic3_z, asic3_y)
			^(bt(asic3_x, 0) << 1)^(bt(asic3_x, 1) << 6)^(bt(asic3_x, 2) << 10)^(bt(asic3_x, 3) << 14);
		break;
	case 2:
		asic3_hold =
			(asic3_hold << 1)
			^0x2bad
			^bt(asic3_hold, 15)^bt(asic3_hold, 7)^bt(asic3_hold, 6)^bt(asic3_hold, 5)
			^bt(asic3_z, asic3_y)
			^(bt(asic3_x, 0) << 4)^(bt(asic3_x, 1) << 6)^(bt(asic3_x, 2) << 10)^(bt(asic3_x, 3) << 12);
		break;
	case 3:
		asic3_hold =
			(asic3_hold << 1)
			^0x2bad
			^bt(asic3_hold, 15)^bt(asic3_hold, 10)^bt(asic3_hold, 8)^bt(asic3_hold, 5)
			^bt(asic3_z, asic3_y)
			^(bt(asic3_x, 0) << 4)^(bt(asic3_x, 1) << 6)^(bt(asic3_x, 2) << 10)^(bt(asic3_x, 3) << 12);
		break;
	}
}

unsigned char pgm_asic3_r(unsigned short /*offset*/)
{
	unsigned char res = 0;
	/* region is supplied by the protection device */
	switch(asic3_reg) {
	case 0x00: res = (asic3_latch[0] & 0xf7) | ((PgmInput[7] << 3) & 0x08); break;
	case 0x01: res = asic3_latch[1]; break;
	case 0x02: res = (asic3_latch[2] & 0x7f) | ((PgmInput[7] << 6) & 0x80); break;
	case 0x03:
		res = (bt(asic3_hold, 15) << 0)
			| (bt(asic3_hold, 12) << 1)
			| (bt(asic3_hold, 13) << 2)
			| (bt(asic3_hold, 10) << 3)
			| (bt(asic3_hold,  7) << 4)
			| (bt(asic3_hold,  9) << 5)
			| (bt(asic3_hold,  2) << 6)
			| (bt(asic3_hold,  5) << 7);
		break;
	case 0x20: res = 0x49; break;
	case 0x21: res = 0x47; break;
	case 0x22: res = 0x53; break;
	case 0x24: res = 0x41; break;
	case 0x25: res = 0x41; break;
	case 0x26: res = 0x7f; break;
	case 0x27: res = 0x41; break;
	case 0x28: res = 0x41; break;
	case 0x2a: res = 0x3e; break;
	case 0x2b: res = 0x41; break;
	case 0x2c: res = 0x49; break;
	case 0x2d: res = 0xf9; break;
	case 0x2e: res = 0x0a; break;
	case 0x30: res = 0x26; break;
	case 0x31: res = 0x49; break;
	case 0x32: res = 0x49; break;
	case 0x33: res = 0x49; break;
	case 0x34: res = 0x32; break;
	}
	return res;
}

void pgm_asic3_w(unsigned short /*offset*/, unsigned short data)
{
	{
		if(asic3_reg < 3)
			asic3_latch[asic3_reg] = data << 1;
		else if(asic3_reg == 0xa0) {
			asic3_hold = 0;
		} else if(asic3_reg == 0x40) {
			asic3_h2 = asic3_h1;
			asic3_h1 = data;
		} else if(asic3_reg == 0x48) {
			asic3_x = 0;
			if(!(asic3_h2 & 0x0a)) asic3_x |= 8;
			if(!(asic3_h2 & 0x90)) asic3_x |= 4;
			if(!(asic3_h1 & 0x06)) asic3_x |= 2;
			if(!(asic3_h1 & 0x90)) asic3_x |= 1;
		} else if(asic3_reg >= 0x80 && asic3_reg <= 0x87) {
			asic3_y = asic3_reg & 7;
			asic3_z = data;
			asic3_compute_hold();
		}
	}
}

void pgm_asic3_reg_w(unsigned short /*offset*/, unsigned short data)
{
	asic3_reg = data & 0xff;
}

// reset asic3
void asic3_reset()
{
	asic3_hold = 0;
	asic3_latch[0] = asic3_latch[1] = asic3_latch[2] = 0;
	asic3_reg = asic3_x = asic3_y, asic3_z = asic3_h1 = asic3_h2 = 0;
}

/* ASIC28 Knights of Valour / Sango / PhotoY2k Protection (from ElSemi) */

// photo2yk bonus stage
static const unsigned int AETABLE[16]={0x00,0x0a,0x14,  0x01,0x0b,0x15,  0x02,0x0c,0x16};

//Not sure if BATABLE is complete
static const unsigned int BATABLE[0x40]= {
		0x00,0x29,0x2c,0x35,0x3a,0x41,0x4a,0x4e,	//0x00
		0x57,0x5e,0x77,0x79,0x7a,0x7b,0x7c,0x7d,	//0x08
		0x7e,0x7f,0x80,0x81,0x82,0x85,0x86,0x87,	//0x10
		0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x90,	//0x18
		0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,
		0x9e,0xa3,0xd4,0xa9,0xaf,0xb5,0xbb,0xc1 };

static const unsigned int B0TABLE[16]={2,0,1,4,3}; //maps char portraits to tables

unsigned short ASIC28KEY;
unsigned short ASIC28REGS[10];
unsigned short ASICPARAMS[256];
unsigned short ASIC28RCNT=0;
unsigned int E0REGS[16];

static unsigned int photoy2k_seqpos;
static unsigned int photoy2k_trf[3], photoy2k_soff;

unsigned short sango_protram_r(unsigned short offset)
{
	if (offset == 8) return PgmInput[7]; //readinputport(4);
	// otherwise it doesn't seem to use the ram for anything important, we return 0 to avoid test mode corruption
	// kovplus reads from offset 000e a lot ... why?
	
	return 0x0000;
}

static unsigned int photoy2k_spritenum()
{
	UINT32 base = photoy2k_seqpos & 0xffc00;
	UINT32 low  = photoy2k_seqpos & 0x003ff;

	switch((photoy2k_seqpos >> 10) & 0xf) {
	case 0x0:
	case 0xa:
		return base | (BITSWAP16(low, 15,14,13,12,11,10, 0,8,3,1,5,9,4,2,6,7) ^ 0x124);
	case 0x1:
	case 0xb:
		return base | (BITSWAP16(low, 15,14,13,12,11,10, 5,1,7,4,0,8,3,6,9,2) ^ 0x088);
	case 0x2:
	case 0x8:
		return base | (BITSWAP16(low, 15,14,13,12,11,10, 3,5,9,7,6,4,1,8,2,0) ^ 0x011);
	case 0x3:
	case 0x9:
		return base | (BITSWAP16(low, 15,14,13,12,11,10, 1,8,3,6,0,4,5,2,9,7) ^ 0x154);
	case 0x4:
	case 0xe:
		return base | (BITSWAP16(low, 15,14,13,12,11,10, 2,1,7,4,5,8,3,6,9,0) ^ 0x0a9);
	case 0x5:
	case 0xf:
		return base | (BITSWAP16(low, 15,14,13,12,11,10, 9,4,6,8,2,1,7,5,3,0) ^ 0x201);
	case 0x6:
	case 0xd:
		return base | (BITSWAP16(low, 15,14,13,12,11,10, 4,6,0,8,9,7,3,5,1,2) ^ 0x008);
	case 0x7:
	case 0xc:
		return base | (BITSWAP16(low, 15,14,13,12,11,10, 8,9,3,2,0,1,6,7,5,4) ^ 0x000);
	}
	return 0;
}


unsigned short pgm_asic28_r(unsigned short offset)
{
	unsigned int val=(ASIC28REGS[1]<<16)|(ASIC28REGS[0]);
	
	switch(ASIC28REGS[1]&0xff) {
		case 0x20: // PhotoY2k spritenum conversion 4/4
			val = photoy2k_soff >> 16;
			break;
		case 0x21: // PhotoY2k spritenum conversion 3/4
			if(!ASIC28RCNT) {
				photoy2k_trf[2] = val & 0xffff;
				if(photoy2k_trf[0] < 0x3c00)
					photoy2k_soff = pgmy2ks[photoy2k_trf[0]];
				else
					photoy2k_soff = 0;
			}
			val = photoy2k_soff & 0xffff;
			break;
		case 0x22: // PhotoY2k spritenum conversion 2/4
			if(!ASIC28RCNT)	photoy2k_trf[1] = val & 0xffff;
			val = photoy2k_trf[0] | 0x880000;
			break;
		case 0x23: // PhotoY2k spritenum conversion 1/4
			if(!ASIC28RCNT) photoy2k_trf[0] = val & 0xffff;
			val = 0x880000;
			break;
		case 0x30: // PhotoY2k next element
			if(!ASIC28RCNT)	photoy2k_seqpos++;
			val = photoy2k_spritenum();
			break;
		case 0x32: // PhotoY2k start of sequence
			if(!ASIC28RCNT) photoy2k_seqpos = (val & 0xffff) << 4;
			val = photoy2k_spritenum();
			break;
		case 0x99:
			val=0x880000;
			break;
		case 0x9d:	// spr palette
			val=0xa00000+((ASIC28REGS[0]&0x1f)<<6);
			break;
		case 0xae:  //Photo Y2k Bonus stage
			val=AETABLE[ASIC28REGS[0]&0xf];
			break;
		case 0xb0:
			val=B0TABLE[ASIC28REGS[0]&0xf];
			break;
		case 0xb4:
			{
				int v2=ASIC28REGS[0]&0x0f;
				int v1=(ASIC28REGS[0]&0x0f00)>>8;
				if(ASIC28REGS[0]==0x102)
					E0REGS[1]=E0REGS[0];
				else
					E0REGS[v1]=E0REGS[v2];
				val=0x880000;
			}
			break;
		case 0xba:
			val=BATABLE[ASIC28REGS[0]&0x3f];
			if(ASIC28REGS[0]>0x2f) {

			}
			break;
		case 0xc0:
			val=0x880000;
			break;
		case 0xc3:	//TXT tile position Uses C0 to select column
			val=0x904000+(ASICPARAMS[0xc0]+ASICPARAMS[0xc3]*64)*4;
			break;
		case 0xcb:
			val=0x880000;
			break;
		case 0xcc:	//BG
			{
	   	 		int y=ASICPARAMS[0xcc];
	    		if(y&0x400)    //y is signed (probably x too and it also applies to TXT, but I've never seen it used)
	     			y=-(0x400-(y&0x3ff));
	    		val=0x900000+(((ASICPARAMS[0xcb]+(y)*64)*4)/*&0x1fff*/);
   			}
   			break;
		case 0xd0:	//txt palette
			val=0xa01000+(ASIC28REGS[0]<<5);
			break;
		case 0xd6:	//???? check it
			{
				int v2=ASIC28REGS[0]&0xf;
				E0REGS[0]=E0REGS[v2];
				//E0REGS[v2]=0;
				val=0x880000;
			}
			break;
		case 0xdc:	//bg palette
			val=0xa00800+(ASIC28REGS[0]<<6);
			break;
		case 0xe0:	//spr palette
			val=0xa00000+((ASIC28REGS[0]&0x1f)<<6);
			break;
		case 0xe5:
			val=0x880000;
			break;
		case 0xe7:
			val=0x880000;
			break;
		case 0xf0:
			val=0x00C000;
			break;
		case 0xf8:
			val=E0REGS[ASIC28REGS[0]&0xf]&0xffffff;
			break;
		case 0xfc:	//Adjust damage level to char experience level
			val=(ASICPARAMS[0xfc]*ASICPARAMS[0xfe])>>6;
			break;
		case 0xfe:	//todo
			val=0x880000;
			break;
		default:
			val=0x880000;
	}

	if(offset==0) {
		unsigned short d=val&0xffff;
		unsigned short realkey;
		realkey=ASIC28KEY>>8;
		realkey|=ASIC28KEY;
		d^=realkey;
		return d;
	}
	else if(offset==2) {
		unsigned short d=val>>16;
		unsigned short realkey;
		realkey=ASIC28KEY>>8;
		realkey|=ASIC28KEY;
		d^=realkey;
		ASIC28RCNT++;
		if(!(ASIC28RCNT&0xf)) {
			ASIC28KEY+=0x100;
			ASIC28KEY&=0xFF00;
		}
		return d;
	}
	return 0xff;
}

void pgm_asic28_w(unsigned short offset, unsigned short data)
{
	if(offset==0) {
		unsigned short realkey;
		realkey=ASIC28KEY>>8;
		realkey|=ASIC28KEY;
		data^=realkey;
		ASIC28REGS[0]=data;
		return;
	}
	if(offset==2) {
		unsigned short realkey;

		ASIC28KEY=data&0xff00;

		realkey=ASIC28KEY>>8;
		realkey|=ASIC28KEY;
		data^=realkey;
		ASIC28REGS[1]=data;

		ASICPARAMS[ASIC28REGS[1]&0xff]=ASIC28REGS[0];
		if(ASIC28REGS[1]==0xE7) {
			unsigned int E0R=(ASICPARAMS[0xE7]>>12)&0xf;
			E0REGS[E0R]&=0xffff;
			E0REGS[E0R]|=ASIC28REGS[0]<<16;
		}
		if(ASIC28REGS[1]==0xE5) {
			unsigned int E0R=(ASICPARAMS[0xE7]>>12)&0xf;
			E0REGS[E0R]&=0xff0000;
			E0REGS[E0R]|=ASIC28REGS[0];
		}
		ASIC28RCNT=0;
	}
}


void asic28_reset()
{
	ASIC28KEY=ASIC28RCNT=0;
	memset(ASIC28REGS, 0, 10);
	memset(ASICPARAMS, 0, 256);
	memset(E0REGS, 0, 16);

	// photoy2k
	photoy2k_seqpos = photoy2k_soff = 0;
	memset(photoy2k_trf, 0, 3);
}

// Dragon World 2


#define DW2BITSWAP(s,d,bs,bd)  d=((d&(~(1<<bd)))|(((s>>bs)&1)<<bd))

// Use this handler for reading from 0xd80000-0xd80002
unsigned short dw2_d80000_r(unsigned int /*sekAddress*/)
{
	// The value at 0x80EECE is computed in the routine at 0x107c18

	unsigned short d = SekReadWord(0x80eece);
	unsigned short d2 = 0;

	d=(d>>8)|(d<<8);
	DW2BITSWAP(d,d2,7 ,0);
	DW2BITSWAP(d,d2,4 ,1);
	DW2BITSWAP(d,d2,5 ,2);
	DW2BITSWAP(d,d2,2 ,3);
	DW2BITSWAP(d,d2,15,4);
	DW2BITSWAP(d,d2,1 ,5);
	DW2BITSWAP(d,d2,10,6);
	DW2BITSWAP(d,d2,13,7);

	// ... missing bitswaps here (8-15) there is not enough data to know them
	// the code only checks the lowest 8 bytes

	return d2;
}


// Killing Blade uses some kind of DMA protection device which can copy data from a data rom.  The
//   MCU appears to have an internal ROM as if you remove the data ROM then the shared ram is filled
 //  with a constant value.

 //  The device can perform various decryption operations on the data it copies.  for now we're just
 //  using a dump of the shared RAM instead.  This will be improved later.


static int kb_cmd;
static int reg;
static int ptr;
unsigned short *killbld_sharedprotram;


void killbld_prot_w(unsigned int offset, unsigned short data)
{
	if(offset==0)
		kb_cmd=data;
	else
	{
		if(kb_cmd==0)
			reg=data;
		else if(kb_cmd==2)
		{

			if(data==1)	// Execute cmd
			{
				unsigned short cmd=killbld_sharedprotram[0x200/2];

				if(cmd==0x6d)	// Store values to asic ram
				{
					unsigned int p1=(killbld_sharedprotram[0x298/2] << 16) | killbld_sharedprotram[0x29a/2];
					unsigned int p2=(killbld_sharedprotram[0x29c/2] << 16) | killbld_sharedprotram[0x29e/2];
					static unsigned int Regs[0x10];
					if((p2&0xFFFF)==0x9)	// Set value
					{
						int rega=(p2>>16)&0xFFFF;
						if(rega&0x200)
							Regs[rega&0xFF]=p1;
					}
					if((p2&0xFFFF)==0x6)	// Add value
					{
						int src1=(p1>>16)&0xFF;
						int src2=(p1>>0)&0xFF;
						int dst=(p2>>16)&0xFF;
						Regs[dst]=Regs[src2]-Regs[src1];
					}
					if((p2&0xFFFF)==0x1)	// Add Imm?
					{
						int rega=(p2>>16)&0xFF;
						int imm=(p1>>0)&0xFFFF;
						Regs[rega]+=imm;
					}
					if((p2&0xFFFF)==0xa)	// Get value
					{
						int rega=(p1>>16)&0xFF;
						killbld_sharedprotram[0x29c/2] = (Regs[rega]>>16)&0xffff;
						killbld_sharedprotram[0x29e/2] = Regs[rega]&0xffff;
					}
				}
				if(cmd==0x4f)	// memcpy with encryption / scrambling
				{
					unsigned short src=killbld_sharedprotram[0x290/2]>>1; // ?
					unsigned int dst=killbld_sharedprotram[0x292/2];
					unsigned short size=killbld_sharedprotram[0x294/2];
					unsigned short mode=killbld_sharedprotram[0x296/2];

					mode &=0xf;  // what are the other bits?

					if (mode == 1 || mode == 2 || mode == 3)
					{
FILE *fz;
fz = fopen("ram.0", "wb");
fwrite (USER0, 1, 0x2000, fz);
fclose (fz);
						// for now, cheat -- the scramble isn't understood, it might be state based
						int x;
						for (x=0;x<size;x++)
						{

							unsigned short *RAMDUMP = (unsigned short*)USER2;

							unsigned short dat;

							dat = RAMDUMP[dst+x];
							killbld_sharedprotram[dst+x] = dat;
						}
fz = fopen("ram.1", "wb");
fwrite (USER0, 1, 0x2000, fz);
fclose (fz);
					}
					else if (mode == 5)
					{
						// mode 5 seems to be a straight copy
						int x;
						for (x=0;x<size;x++)
						{
							unsigned short *PROTROM = (unsigned short*)USER1;
							unsigned short dat;
							dat = PROTROM[src+x];

							killbld_sharedprotram[dst+x] = dat;
						}
					}
					else if (mode == 6)
					{
						// mode 6 seems to swap bytes and nibbles
						int x;
						for (x=0;x<size;x++)
						{
							unsigned short *PROTROM = (unsigned short*)USER1;
							unsigned short dat;
							dat = PROTROM[src+x];

							dat = ((dat & 0xf000) >> 12)|
								  ((dat & 0x0f00) >> 4)|
								  ((dat & 0x00f0) << 4)|
								  ((dat & 0x000f) << 12);

							killbld_sharedprotram[dst+x] = dat;
						}
					}
					else
					{

					}

					killbld_sharedprotram[0x2600/2]=0x4e75;
				}
				reg++;
			}
		}
		else if(kb_cmd==4)
			ptr=data;
		else if(kb_cmd==0x20)
			ptr++;
	}
}

unsigned short killbld_prot_r(unsigned int offset)
{
	if(offset==2)
	{
		if(kb_cmd == 1)
			return reg & 0x7f;

		if(kb_cmd == 5)
			return ((0x89911400 | (/*PgmInput[7] + */0x16)) >> (8 * (ptr - 1))) & 0xff; // region
	}

	return 0;
}

void killbldt_reset()
{
	kb_cmd = reg = ptr = 0;
	killbld_sharedprotram = NULL;
}



// (PSTARS)
unsigned short PSTARSKEY;
static unsigned short PSTARSINT[2];
static unsigned int PSTARS_REGS[16];
static unsigned int PSTARS_VAL;

static unsigned short pstar_e7,pstar_b1,pstar_ce;
static unsigned short pstar_ram[3];

static int Pstar_ba[0x1E]={
	0x02,0x00,0x00,0x01,0x00,0x03,0x00,0x00, //0
	0x02,0x00,0x06,0x00,0x22,0x04,0x00,0x03, //8
	0x00,0x00,0x06,0x00,0x20,0x07,0x00,0x03, //10
	0x00,0x21,0x01,0x00,0x00,0x63
};

static int Pstar_b0[0x10]={
	0x09,0x0A,0x0B,0x00,0x01,0x02,0x03,0x04,
	0x05,0x06,0x07,0x08,0x00,0x00,0x00,0x00
};

static int Pstar_ae[0x10]={
	0x5D,0x86,0x8C ,0x8B,0xE0,0x8B,0x62,0xAF,
	0xB6,0xAF,0x10A,0xAF,0x00,0x00,0x00,0x00
};

static int Pstar_a0[0x10]={
	0x02,0x03,0x04,0x05,0x06,0x01,0x0A,0x0B,
	0x0C,0x0D,0x0E,0x09,0x00,0x00,0x00,0x00,
};

static int Pstar_9d[0x10]={
	0x05,0x03,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

static int Pstar_90[0x10]={
	0x0C,0x10,0x0E,0x0C,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
static int Pstar_8c[0x23]={
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,
	0x02,0x02,0x03,0x03,0x03,0x04,0x04,0x04,
	0x03,0x03,0x03
};

static int Pstar_80[0x1a3]={
	0x03,0x03,0x04,0x04,0x04,0x04,0x05,0x05,
	0x05,0x05,0x06,0x06,0x03,0x03,0x04,0x04,
	0x05,0x05,0x05,0x05,0x06,0x06,0x07,0x07,
	0x03,0x03,0x04,0x04,0x05,0x05,0x05,0x05,
	0x06,0x06,0x07,0x07,0x06,0x06,0x06,0x06,
	0x06,0x06,0x06,0x07,0x07,0x07,0x07,0x07,
	0x06,0x06,0x06,0x06,0x06,0x06,0x07,0x07,
	0x07,0x07,0x08,0x08,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x06,0x06,0x06,0x07,0x07,
	0x06,0x06,0x06,0x07,0x07,0x07,0x08,0x08,
	0x09,0x09,0x09,0x09,0x07,0x07,0x07,0x07,
	0x07,0x08,0x08,0x08,0x08,0x09,0x09,0x09,
	0x06,0x06,0x07,0x07,0x07,0x08,0x08,0x08,
	0x08,0x08,0x09,0x09,0x05,0x05,0x06,0x06,
	0x06,0x07,0x07,0x08,0x08,0x08,0x08,0x09,
	0x07,0x07,0x07,0x07,0x07,0x08,0x08,0x08,
	0x08,0x09,0x09,0x09,0x06,0x06,0x07,0x03,
	0x07,0x06,0x07,0x07,0x08,0x07,0x05,0x04,
	0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,
	0x06,0x06,0x06,0x06,0x03,0x04,0x04,0x04,
	0x04,0x05,0x05,0x06,0x06,0x06,0x06,0x07,
	0x04,0x04,0x05,0x05,0x06,0x06,0x06,0x06,
	0x06,0x07,0x07,0x08,0x05,0x05,0x06,0x07,
	0x07,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x05,0x05,0x05,0x07,0x07,0x07,0x07,0x07,
	0x07,0x08,0x08,0x08,0x08,0x08,0x09,0x09,
	0x09,0x09,0x03,0x04,0x04,0x05,0x05,0x05,
	0x06,0x06,0x07,0x07,0x07,0x07,0x08,0x08,
	0x08,0x09,0x09,0x09,0x03,0x04,0x05,0x05,
	0x04,0x03,0x04,0x04,0x04,0x05,0x05,0x04,
	0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
	0x03,0x03,0x03,0x04,0x04,0x04,0x04,0x04,
	0x04,0x04,0x04,0x04,0x04,0x03,0x03,0x03,
	0x03,0x03,0x03,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,
	0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00
};




unsigned short PSTARS_protram_r(unsigned int offset)
{
	offset >>= 1;

	if (offset == 4)		//region
		return PgmInput[7];
	else if (offset >= 0x10)  //timer
	{
		return pstar_ram[offset-0x10]--;
	}
	return 0x0000;
}

unsigned short PSTARS_r16(unsigned int offset)
{
	offset >>= 1;

	if(offset==0)
	{
		UINT16 d=PSTARS_VAL&0xffff;
		UINT16 realkey;
		realkey=PSTARSKEY>>8;
		realkey|=PSTARSKEY;
		d^=realkey;
		return d;
	}
	else if(offset==1)
	{
		UINT16 d=PSTARS_VAL>>16;
		UINT16 realkey;
		realkey=PSTARSKEY>>8;
		realkey|=PSTARSKEY;
		d^=realkey;
		return d;

	}
	return 0xff;
}

void PSTARS_w16(unsigned int offset, unsigned short data)
{
	offset >>= 1;

	if(offset==0)
	{
		PSTARSINT[0]=data;
		return;
	}

	if(offset==1)
	{
		UINT16 realkey;
		if((data>>8)==0xff)
			PSTARSKEY=0xff00;
		realkey=PSTARSKEY>>8;
		realkey|=PSTARSKEY;
		{
			PSTARSKEY+=0x100;
			PSTARSKEY&=0xff00;
		 if(PSTARSKEY==0xff00)PSTARSKEY=0x100;
		 }
		data^=realkey;
		PSTARSINT[1]=data;
		PSTARSINT[0]^=realkey;

		switch(PSTARSINT[1]&0xff)
			{
				case 0x99:
				{
					PSTARSKEY=0x100;
					PSTARS_VAL=0x880000;

				}
				break;

				case 0xE0:
					{
						PSTARS_VAL=0xa00000+(PSTARSINT[0]<<6);
					}
					break;
				case 0xDC:
					{
						PSTARS_VAL=0xa00800+(PSTARSINT[0]<<6);
					}
					break;
				case 0xD0:
					{
						PSTARS_VAL=0xa01000+(PSTARSINT[0]<<5);
					}
					break;

				case 0xb1:
					{
						pstar_b1=PSTARSINT[0];
						PSTARS_VAL=0x890000;
					}
					break;
				case 0xbf:
					{
						PSTARS_VAL=pstar_b1*PSTARSINT[0];
					}
					break;

				case 0xc1: //TODO:TIMER  0,1,2,FIX TO 0 should be OK?
					{
						PSTARS_VAL=0;
					}
					break;
				case 0xce: //TODO:TIMER  0,1,2
					{
						pstar_ce=PSTARSINT[0];
						PSTARS_VAL=0x890000;
					}
					break;
				case 0xcf: //TODO:TIMER  0,1,2
					{
						pstar_ram[pstar_ce]=PSTARSINT[0];
						PSTARS_VAL=0x890000;
					}
					break;


				case 0xe7:
					{
						pstar_e7=(PSTARSINT[0]>>12)&0xf;
						PSTARS_REGS[pstar_e7]&=0xffff;
						PSTARS_REGS[pstar_e7]|=(PSTARSINT[0]&0xff)<<16;
						PSTARS_VAL=0x890000;
					}
					break;
				case 0xe5:
					{

						PSTARS_REGS[pstar_e7]&=0xff0000;
						PSTARS_REGS[pstar_e7]|=PSTARSINT[0];
						PSTARS_VAL=0x890000;
					}
					break;
				case 0xf8: //@73C
	   			{
	    			PSTARS_VAL=PSTARS_REGS[PSTARSINT[0]&0xf]&0xffffff;
	   			}
	   			break;


				case 0xba:
	   			{
	    			PSTARS_VAL=Pstar_ba[PSTARSINT[0]];
	   			}
	   			break;
				case 0xb0:
	   			{
	    			PSTARS_VAL=Pstar_b0[PSTARSINT[0]];
	   			}
	   			break;
				case 0xae:
	   			{
	    			PSTARS_VAL=Pstar_ae[PSTARSINT[0]];
	   			}
	   			break;
				case 0xa0:
	   			{
	    			PSTARS_VAL=Pstar_a0[PSTARSINT[0]];
	   			}
	   			break;
				case 0x9d:
	   			{
	    			PSTARS_VAL=Pstar_9d[PSTARSINT[0]];
	   			}
	   			break;
				case 0x90:
	   			{
	    			PSTARS_VAL=Pstar_90[PSTARSINT[0]];
	   			}
	   			break;
				case 0x8c:
	   			{
	    			PSTARS_VAL=Pstar_8c[PSTARSINT[0]];
	   			}
	   			break;
				case 0x80:
	   			{
	    			PSTARS_VAL=Pstar_80[PSTARSINT[0]];
	   			}
	   			break;
				default:
					 PSTARS_VAL=0x890000;
		}

	}
}

void pstars_reset()
{
	PSTARSKEY = 0;
	PSTARS_VAL = 0;
	PSTARSINT[0] = PSTARSINT[1] = 0;
	pstar_e7 = pstar_b1 = pstar_ce = 0;

	memset(PSTARS_REGS, 0, 16);
	memset(pstar_ram,   0,  3);
}

// Oriental Legends Super

unsigned short olds_bs, olds_cmd3;

unsigned short olds_r16(unsigned short offset)
{
	unsigned short res = 0;

	if (offset == 2)
	{
		if (kb_cmd == 1)
			res = reg & 0x7f;

		if (kb_cmd == 2)
			res = olds_bs | 0x80;

		if (kb_cmd == 3)
			res = olds_cmd3;

		if (kb_cmd == 5)
		{
			res = ((0x900000 | (PgmInput[7]+1)) >> (8 * (ptr - 1))) & 0xff; // includes region
		}
	}

	return res;
}

void olds_w16(unsigned int offset, unsigned short data)
{
	if (offset == 0)
	{
		kb_cmd = data;
	}
	else
	{
		if (kb_cmd == 0)
			reg = data;

		if (kb_cmd == 2)
		{
			olds_bs = ((data & 3) << 2) | ((data & 4) >> 1) | ((data & 8) >> 3);
		}

		if (kb_cmd == 3)
		{
			olds_cmd3 = ((data >> 4) + 1) & 3;
		}

		if (kb_cmd == 4)
			ptr = data;

		if (kb_cmd == 0x20)
			ptr++;
	}
}

void olds_reset()
{
	olds_bs =  olds_cmd3 = 0;
}


// reset protections

void prot_reset()
{
	asic3_reset();
	asic28_reset();
	killbldt_reset();
	pstars_reset();
	olds_reset();
}


// Savestate support

int asic28Scan(int nAction,int */*pnMin*/)
{
	if (nAction & ACB_DRIVER_DATA) {
		// Scan critical driver variables
		SCAN_VAR(ASIC28KEY);
		SCAN_VAR(ASIC28REGS);
		SCAN_VAR(ASICPARAMS);
		SCAN_VAR(ASIC28RCNT);
		SCAN_VAR(E0REGS);
		SCAN_VAR(photoy2k_seqpos);
		SCAN_VAR(photoy2k_trf);
		SCAN_VAR(photoy2k_soff);
	}

	return 0;
}

int asic3Scan(int nAction,int */*pnMin*/)
{
	if (nAction & ACB_DRIVER_DATA) {
		// Scan critical driver variables
		SCAN_VAR(asic3_reg);
		SCAN_VAR(asic3_latch);
		SCAN_VAR(asic3_x);
		SCAN_VAR(asic3_y);
		SCAN_VAR(asic3_z);
		SCAN_VAR(asic3_h1);
		SCAN_VAR(asic3_h2);
		SCAN_VAR(asic3_hold);
	}

	return 0;
}

int killbldtScan(int nAction,int */*pnMin*/)
{
	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= USER0 + 0x000000;
		ba.nLen		= 0x0004000;
		ba.nAddress = 0;
		ba.szName	= "ProtRAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(kb_cmd);
		SCAN_VAR(reg);
		SCAN_VAR(ptr);
	}

	return 0;
}

int pstarsScan(int nAction,int */*pnMin*/)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(PSTARSKEY);
		SCAN_VAR(PSTARS_REGS);
		SCAN_VAR(PSTARS_VAL);
		SCAN_VAR(PSTARSINT);
		SCAN_VAR(pstar_e7);
		SCAN_VAR(pstar_b1);
		SCAN_VAR(pstar_ce);
		SCAN_VAR(pstar_ram);
	}

	return 0;
}

int oldsScan(int nAction,int */*pnMin*/)
{
	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= USER0 + 0x100000;
		ba.nLen		= 0x0100000;
		ba.nAddress = 0;
		ba.szName	= "Protection RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(olds_bs);
		SCAN_VAR(olds_cmd3);
	}

	return 0;
}
