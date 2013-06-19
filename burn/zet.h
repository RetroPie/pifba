// Z80 (Zed Eight-Ty) Interface
//#define EMU_DOZE					// Use Dave's 'Doze' Assembler Z80 emulator
#define EMU_DRZ80					// Use Reesy's Assembler Z80 Emulator

#ifdef EMU_DOZE
 #include "doze.h"
#endif

#ifdef EMU_DRZ80
 #include "DrZ80.h"
 
 extern struct DrZ80 Doze;

#endif

int ZetInit(int nCount);
void ZetExit();
void ZetNewFrame();
int ZetOpen(int nCPU);
void ZetClose();
int ZetMemCallback(int nStart,int nEnd,int nMode);
int ZetMemEnd();
int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem);
int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem01, unsigned char *Mem02);
int ZetReset();
int ZetPc(int n);
int ZetScan(int nAction);

#if defined(EMU_DOZE) 
#define ZET_IRQSTATUS_NONE DOZE_IRQSTATUS_NONE
#define ZET_IRQSTATUS_AUTO DOZE_IRQSTATUS_AUTO
#define ZET_IRQSTATUS_ACK  DOZE_IRQSTATUS_ACK

#elif defined(EMU_DRZ80) 
#define ZET_IRQSTATUS_NONE (0x8000)
#define ZET_IRQSTATUS_AUTO (0x2000)
#define ZET_IRQSTATUS_ACK  (0x1000)

#endif

#ifndef EMU_DRZ80

inline static int ZetNmi()
{
#ifdef EMU_DOZE
	int nCycles = DozeNmi();
	Doze.nCyclesTotal += nCycles;
	return nCycles
#else
	return 12;
#endif
}

#else

int ZetNmi();

#endif

inline static void ZetSetIRQLine(const int line, const int status)
{
#if defined(EMU_DOZE) || defined(EMU_DRZ80)
	Doze.nInterruptLatch = line | status;
#endif
}

#define ZetRaiseIrq(n) ZetSetIRQLine(n, ZET_IRQSTATUS_AUTO)
#define ZetLowerIrq() ZetSetIRQLine(0, ZET_IRQSTATUS_NONE)

int ZetRun(int nCycles);
void ZetRunEnd();

inline static int ZetIdle(int nCycles)
{
#if defined(EMU_DOZE) || defined(EMU_DRZ80)
	Doze.nCyclesTotal += nCycles;
#endif
	return nCycles;
}

inline static int ZetSegmentCycles()
{
#if defined(EMU_DOZE) || defined(EMU_DRZ80)
	return Doze.nCyclesSegment - Doze.nCyclesLeft;
#else
	return 0;
#endif
}

inline static int ZetTotalCycles()
{
#if defined(EMU_DOZE) || defined(EMU_DRZ80)
	return Doze.nCyclesTotal - Doze.nCyclesLeft;
#else
	return 0;
#endif
}

void ZetSetReadHandler(unsigned char (__fastcall *pHandler)(unsigned short));
void ZetSetWriteHandler(void (__fastcall *pHandler)(unsigned short, unsigned char));
void ZetSetInHandler(unsigned char (__fastcall *pHandler)(unsigned short));
void ZetSetOutHandler(void (__fastcall *pHandler)(unsigned short, unsigned char));
