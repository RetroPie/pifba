// Z80 (Zed Eight-Ty) Interface

#ifndef _ZET_H_
#define _ZET_H_

#ifndef FASTCALL
 #undef __fastcall
 #define __fastcall
#endif

//#define EMU_DRZ80
#define EMU_MAME_Z80
#define EMU_CZ80

#ifdef EMU_DOZE
 #include "doze.h"
#endif

#ifdef EMU_MAME_Z80
 #include "z80.h"
#endif

#ifdef EMU_CZ80
 #include "cz80.h"
#endif
extern int nHasZet;
extern int nZetCpuCore; // 0 - CZ80, 1 - MAME_Z80
void ZetWriteByte(unsigned short address, unsigned char data);
unsigned char ZetReadByte(unsigned short address);
int ZetInit(int nCount);
void ZetExit();
void ZetNewFrame();
int ZetOpen(int nCPU);
void ZetClose();
int ZetGetActive();
int ZetMemCallback(int nStart,int nEnd,int nMode);
int ZetMemEnd();
int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem);
int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem01, unsigned char *Mem02);
int ZetReset();
int ZetPc(int n);
int ZetBc(int n);
int ZetDe(int n);
int ZetHL(int n);
int ZetScan(int nAction);
int ZetRun(int nCycles);
void ZetRunEnd();
void ZetSetIRQLine(const int line, const int status);
int ZetNmi();
int ZetIdle(int nCycles);
int ZetSegmentCycles();
int ZetTotalCycles();

void ZetRaiseIrq(int n);
void ZetLowerIrq();

static inline int getZET_IRQSTATUS_NONE()
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) return DOZE_IRQSTATUS_NONE;
#endif
#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) return 0;
#endif
#ifdef EMU_CZ80
	if(nZetCpuCore == 0) return CZ80_IRQSTATUS_NONE;
#endif
}

static inline int getZET_IRQSTATUS_AUTO()
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) return DOZE_IRQSTATUS_AUTO;
#endif
#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) return 2;
#endif
#ifdef EMU_CZ80
	if(nZetCpuCore == 0) return CZ80_IRQSTATUS_AUTO;
#endif
}

static inline int getZET_IRQSTATUS_ACK()
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) return DOZE_IRQSTATUS_ACK;
#endif
#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) return 1;
#endif
#ifdef EMU_CZ80
	if(nZetCpuCore == 0) return CZ80_IRQSTATUS_ACK;
#endif
}

#define ZET_IRQSTATUS_NONE getZET_IRQSTATUS_NONE()
#define ZET_IRQSTATUS_AUTO getZET_IRQSTATUS_AUTO()
#define ZET_IRQSTATUS_ACK  getZET_IRQSTATUS_ACK()

#ifdef EMU_MAME_Z80

typedef unsigned char (__fastcall *pZetInHandler)(unsigned short a);
typedef void (__fastcall *pZetOutHandler)(unsigned short a, unsigned char d);
typedef unsigned char (__fastcall *pZetReadHandler)(unsigned short a);
typedef void (__fastcall *pZetWriteHandler)(unsigned short a, unsigned char d);

struct ZetExt {

	Z80_Regs reg;
	
	unsigned char* pZetMemMap[0x100 * 4];

	pZetInHandler ZetIn;
	pZetOutHandler ZetOut;
	pZetReadHandler ZetRead;
	pZetWriteHandler ZetWrite;
	
	int nCyclesTotal;
	int nCyclesSegment;
	int nCyclesLeft;
	
	int nEI;
	int nInterruptLatch;
};

#endif

void ZetSetReadHandler(unsigned char (__fastcall *pHandler)(unsigned short));
void ZetSetWriteHandler(void (__fastcall *pHandler)(unsigned short, unsigned char));
void ZetSetInHandler(unsigned char (__fastcall *pHandler)(unsigned short));
void ZetSetOutHandler(void (__fastcall *pHandler)(unsigned short, unsigned char));

#endif //_ZET_H_