// Nec V20/V30/V33 interface
// Write by OopsWare
// http://oopsware.googlepages.com

#include "burnint.h"
#include "vez.h"

#define VEZ_MEM_SHIFT	11
#define VEZ_MEM_MASK	((1 << VEZ_MEM_SHIFT) - 1)

static struct VezContext * VezCPUContext = 0;
static struct VezContext * VezCurrentCPU = 0;
static int nCPUCount = 0;
static int nOpenedCPU = -1;

unsigned char __fastcall VezDummyReadHandler(unsigned int) { return 0; }
void __fastcall VezDummyWriteHandler(unsigned int, unsigned char) { }
unsigned char __fastcall VezDummyReadPort(unsigned int) { return 0; }
void __fastcall VezDummyWritePort(unsigned int, unsigned char) { }

// memory handler for nec emulator

unsigned char cpu_readport(unsigned char p)
{
	return VezCurrentCPU->ReadPort(p);
}

void cpu_writeport(unsigned int p,unsigned char d)
{
	VezCurrentCPU->WritePort(p, d);
}

unsigned char cpu_readmem20(unsigned int a)
{
	a &= 0xFFFFF;
	
	unsigned char * p = VezCurrentCPU->ppMemRead[ a >> VEZ_MEM_SHIFT ];
	if ( p )
		return *(p + a);
	else
		return VezCurrentCPU->ReadHandler(a);
}

void cpu_writemem20(unsigned int a, unsigned char d)
{
	a &= 0xFFFFF;
	
	unsigned char * p = VezCurrentCPU->ppMemWrite[ a >> VEZ_MEM_SHIFT ];
	if ( p )
		*(p + a) = d;
	else
		VezCurrentCPU->WriteHandler(a, d);
}

// fba interface

void VezSetReadHandler(unsigned char (__fastcall *pHandler)(unsigned int))
{
	VezCurrentCPU->ReadHandler = pHandler;
}

void VezSetWriteHandler(void (__fastcall *pHandler)(unsigned int, unsigned char))
{
	VezCurrentCPU->WriteHandler = pHandler;
}

void VezSetReadPort(unsigned char (__fastcall *pHandler)(unsigned int))
{
	VezCurrentCPU->ReadPort = pHandler;
}

void VezSetWritePort(void (__fastcall *pHandler)(unsigned int, unsigned char))
{
	VezCurrentCPU->WritePort = pHandler;
}

int VezInit(int nCount, unsigned int * typelist)
{
	VezCPUContext = (struct VezContext *)malloc(nCount * sizeof(struct VezContext));
	if (VezCPUContext == NULL) return 1;

	memset(VezCPUContext, 0, nCount * sizeof(struct VezContext));
	
	for(int i=0;i<nCount;i++) {
		VezCPUContext[i].reg.cpu_type = typelist[i];
		
		VezCPUContext[i].ReadHandler = VezDummyReadHandler;
		VezCPUContext[i].WriteHandler = VezDummyWriteHandler;
		VezCPUContext[i].ReadPort = VezDummyReadPort;
		VezCPUContext[i].WritePort = VezDummyWritePort;
	}
	
	nCPUCount = nCount;
	
	return 0;
}

void VezExit()
{
	free(VezCPUContext);
	VezCPUContext = 0;
	VezCurrentCPU = 0;
	nCPUCount = 0;
	nOpenedCPU = -1;
}

void VezNewFrame()
{
}

int VezOpen(int nCPU)
{
	nOpenedCPU = nCPU;
	VezCurrentCPU = VezCPUContext + nCPU;
	return 0;
}

void VezClose()
{
	nOpenedCPU = -1;
	VezCurrentCPU = 0;
}

int VezMemCallback(int nStart,int nEnd,int nMode)
{
	nStart >>= VEZ_MEM_SHIFT;
	nEnd += VEZ_MEM_MASK;
	nEnd >>= VEZ_MEM_SHIFT;

	for (int i = nStart; i < nEnd; i++) {
		switch (nMode) {
			case 0:
				VezCurrentCPU->ppMemRead[i] = NULL;
				break;
			case 1:
				VezCurrentCPU->ppMemWrite[i] = NULL;
				break;
		}
	}
	return 0;
}

int VezMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem)
{
	int s = nStart >> VEZ_MEM_SHIFT;
	int e = (nEnd + VEZ_MEM_MASK) >> VEZ_MEM_SHIFT;

	for (int i = s; i < e; i++) {
		switch (nMode) {
			case 0:
				VezCurrentCPU->ppMemRead[i] = Mem - nStart;
				break;
			case 1:
				VezCurrentCPU->ppMemWrite[i] = Mem - nStart;
				break;
		}
	}
	return 0;
}

int VezReset()
{
	nec_reset( &(VezCurrentCPU->reg) );
	
	return 0;
}

int VezRun(int nCycles)
{
	if (nCycles <= 0) return 0;

	return nec_execute( &(VezCurrentCPU->reg), nCycles);
}

int VezPc(int n)
{
	return 0;
}

int VezScan(int nAction)
{
	return 0;
}

void VezSetIRQLine(const int line, const int status)
{
	//?
	if ( status )
		nec_int(line);
}

