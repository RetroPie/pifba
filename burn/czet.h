// Z80 (Zed Eight-Ty) Interface
 #include "cz80.h"

 #define CZET_IRQSTATUS_NONE CZ80_IRQSTATUS_NONE
 #define CZET_IRQSTATUS_AUTO CZ80_IRQSTATUS_AUTO
 #define CZET_IRQSTATUS_ACK  CZ80_IRQSTATUS_ACK
 #define CZetRaiseIrq(n) CZetSetIRQLine(n, CZET_IRQSTATUS_AUTO)
 #define CZetLowerIrq() CZetSetIRQLine(0, CZET_IRQSTATUS_NONE)

int CZetInit(int nCount);
void CZetExit();
void CZetNewFrame();
int CZetOpen(int nCPU);
void CZetClose();
int CZetMemCallback(int nStart,int nEnd,int nMode);
int CZetMemEnd();
int CZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem);
int CZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem01, unsigned char *Mem02);
int CZetReset();
int CZetPc(int n);
int CZetBc(int n);
int CZetHL(int n);
int CZetScan(int nAction);
int CZetRun(int nCycles);
void CZetRunEnd();
void CZetSetIRQLine(const int line, const int status);
int CZetNmi();
int CZetIdle(int nCycles);
int CZetSegmentCycles();
int CZetTotalCycles();

#define MAX_CPUS	4

void CZetSetReadHandler(unsigned char (__fastcall *pHandler)(unsigned short));
void CZetSetWriteHandler(void (__fastcall *pHandler)(unsigned short, unsigned char));
void CZetSetInHandler(unsigned char (__fastcall *pHandler)(unsigned short));
void CZetSetOutHandler(void (__fastcall *pHandler)(unsigned short, unsigned char));
