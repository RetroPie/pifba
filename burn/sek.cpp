// 680x0 (Sixty Eight K) Interface

// Sek with Dave's Cyclone CORE
// Write by OopsWare 2007.1

#include "burnint.h"
#include "sekdebug.h"
#include "gp2xmemfuncs.h"

struct Cyclone PicoCpu;
static bool bCycloneInited = false;

static int nSekCount = -1;							// Number of allocated 68000s
struct SekExt tSekExt, *pSekExt;

int nSekActive = -1;								// The cpu which is currently being emulated
int nSekCyclesTotal, nSekCyclesScanline, nSekCyclesSegment, nSekCyclesDone, nSekCyclesToDo;

//int nSekCPUType[SEK_MAX], nSekCycles[SEK_MAX], nSekIRQPending[SEK_MAX];
int nSekCPUType = 0x68000; 
int nSekIRQPending;

// ----------------------------------------------------------------------------
// Default memory access handlers

unsigned char __fastcall DefReadByte(unsigned int) { return 0; }
void __fastcall DefWriteByte(unsigned int, unsigned char) { }

#define DEFWORDHANDLERS(i)																				\
	unsigned short __fastcall DefReadWord##i(unsigned int a) { SEK_DEF_READ_WORD(i, a) }				\
	void __fastcall DefWriteWord##i(unsigned int a, unsigned short d) { SEK_DEF_WRITE_WORD(i, a ,d) }
#define DEFLONGHANDLERS(i)																				\
	unsigned int __fastcall DefReadLong##i(unsigned int a) { SEK_DEF_READ_LONG(i, a) }					\
	void __fastcall DefWriteLong##i(unsigned int a, unsigned int d) { SEK_DEF_WRITE_LONG(i, a , d) }

DEFWORDHANDLERS(0);
DEFLONGHANDLERS(0);

#if SEK_MAXHANDLER >= 2
 DEFWORDHANDLERS(1);
 DEFLONGHANDLERS(1);
#endif

#if SEK_MAXHANDLER >= 3
 DEFWORDHANDLERS(2);
 DEFLONGHANDLERS(2);
#endif

#if SEK_MAXHANDLER >= 4
 DEFWORDHANDLERS(3);
 DEFLONGHANDLERS(3);
#endif

#if SEK_MAXHANDLER >= 5
 DEFWORDHANDLERS(4);
 DEFLONGHANDLERS(4);
#endif

#if SEK_MAXHANDLER >= 6
 DEFWORDHANDLERS(5);
 DEFLONGHANDLERS(5);
#endif

#if SEK_MAXHANDLER >= 7
 DEFWORDHANDLERS(6);
 DEFLONGHANDLERS(6);
#endif

#if SEK_MAXHANDLER >= 8
 DEFWORDHANDLERS(7);
 DEFLONGHANDLERS(7);
#endif

// ----------------------------------------------------------------------------
// Memory access functions

// Mapped Memory lookup (               for read)
#define FIND_R(x) pSekExt->MemMap[ x >> SEK_SHIFT]
// Mapped Memory lookup (+ SEK_WADD     for write)
#define FIND_W(x) pSekExt->MemMap[(x >> SEK_SHIFT) + SEK_WADD]
// Mapped Memory lookup (+ SEK_WADD * 2 for fetch)
#define FIND_F(x) pSekExt->MemMap[(x >> SEK_SHIFT) + SEK_WADD * 2]

// Normal memory access functions
inline static unsigned char ReadByte(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("read8 0x%08X\n"), a);
	//dprintf("read8(0x%08x);\n", a);

	pr = FIND_R(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		return pr[a & SEK_PAGEM];
	}
	return pSekExt->ReadByte[(unsigned int)pr](a);
}

inline static unsigned char FetchByte(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("fetch8 0x%08X\n"), a);
//	dprintf("fetch8(0x%08x);\n", a);

	pr = FIND_F(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		return pr[a & SEK_PAGEM];
	}
	return pSekExt->ReadByte[(unsigned int)pr](a);
}

inline static void WriteByte(unsigned int a, unsigned char d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("write8 0x%08X\n"), a);
//	dprintf("write8(0x%08X, 0x%08X); cyc: %d\n", a, d, PicoCpu.cycles);

	pr = FIND_W(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		pr[a & SEK_PAGEM] = (unsigned char)d;
		return;
	}
	pSekExt->WriteByte[(unsigned int)pr](a, d);
}

inline static void WriteByteROM(unsigned int a, unsigned char d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_R(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		pr[a & SEK_PAGEM] = (unsigned char)d;
		return;
	}
	pSekExt->WriteByte[(unsigned int)pr](a, d);
}

inline static unsigned short ReadWord(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("read16 0x%08X\n"), a);
	//if (a & 1) printf(" !!! read16(0x%08x); \n", a);

	
	pr = FIND_R(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		return *((unsigned short*)(pr + (a & SEK_PAGEM)));
	}

	return pSekExt->ReadWord[(unsigned int)pr](a);
}

inline static unsigned short FetchWord(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("fetch16 0x%08X\n"), a);
	//if (a & 1) printf(" !!! fetch16(0x%08x);\n", a);

	pr = FIND_F(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		return *((unsigned short*)(pr + (a & SEK_PAGEM)));
	}
	return pSekExt->ReadWord[(unsigned int)pr](a);
}

inline static void WriteWord(unsigned int a, unsigned short d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("write16 0x%08X\n"), a);
	//if (a & 1) printf(" !!! write16(0x%08x, 0x%04x); pc: 0x%08x\n", a, d, PicoCpu.pc - PicoCpu.membase );

	pr = FIND_W(a);
	
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		*((unsigned short*)(pr + (a & SEK_PAGEM))) = (unsigned short)d;
		return;
	}
	pSekExt->WriteWord[(unsigned int)pr](a, d);
}	


inline static void WriteWordROM(unsigned int a, unsigned short d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	//if (a & 1) printf(" !!! write16ROM(0x%08x, 0x%04x); pc: 0x%08x\n", a, d, PicoCpu.pc - PicoCpu.membase );

	pr = FIND_R(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		*((unsigned short*)(pr + (a & SEK_PAGEM))) = (unsigned short)d;
		return;
	}
	pSekExt->WriteWord[(unsigned int)pr](a, d);
}

inline static unsigned int ReadLong(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("read32 0x%08X\n"), a);

	// if (a & 1) printf(" !!!!!! read32(0x%08x);\n", a);

	pr = FIND_R(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
#ifndef OOPSWARE_FIX
		unsigned int r = *((unsigned int*)(pr + (a & SEK_PAGEM)));
		r = (r >> 16) | (r << 16);
#else
		unsigned int r = ( *((unsigned short*)(pr + (a & SEK_PAGEM))) ) << 16;
		r |= *((unsigned short*)(pr + (a & SEK_PAGEM) + 2));
#endif
		return r;
	}
	return pSekExt->ReadLong[(unsigned int)pr](a);
}

inline static unsigned int FetchLong(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("fetch32 0x%08X\n"), a);
	//if ( a & 1 ) printf(" !!!!!! fetch32(0x%08x);\n", a);

	pr = FIND_F(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
#ifndef OOPSWARE_FIX
		unsigned int r = *((unsigned int*)(pr + (a & SEK_PAGEM)));
		r = (r >> 16) | (r << 16);
#else
		unsigned int r = ( *((unsigned short*)(pr + (a & SEK_PAGEM))) ) << 16;
		r |= *((unsigned short*)(pr + (a & SEK_PAGEM) + 2));
#endif
		return r;
	}
	return pSekExt->ReadLong[(unsigned int)pr](a);
}

inline static void WriteLong(unsigned int a, unsigned int d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("write32 0x%08X\n"), a);
	//if (a & 1) printf(" !!!!!! write32(0x%08x, 0x%08x);\n", a, d);

	pr = FIND_W(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
#ifndef OOPSWARE_FIX		
		d = (d >> 16) | (d << 16);
		*((unsigned int*)(pr + (a & SEK_PAGEM))) = d;
#else
		*((unsigned short*)(pr + (a & SEK_PAGEM))) = d >> 16;
		*((unsigned short*)(pr + (a & SEK_PAGEM) + 2)) = d & 0xffff;
#endif		
		return;
	}
	pSekExt->WriteLong[(unsigned int)pr](a, d);
}

inline static void WriteLongROM(unsigned int a, unsigned int d)
{
	unsigned char* pr;
	a &= 0xFFFFFF;

	//if (a & 1) printf(" !!! write32ROM(0x%08x, 0x%08x);\n", a, d);

	pr = FIND_R(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
#ifndef OOPSWARE_FIX		
		d = (d >> 16) | (d << 16);
		*((unsigned int*)(pr + (a & SEK_PAGEM))) = d;
#else
		*((unsigned short*)(pr + (a & SEK_PAGEM))) = d >> 16;
		*((unsigned short*)(pr + (a & SEK_PAGEM) + 2)) = d & 0xffff;
#endif
		return;
	}
	pSekExt->WriteLong[(unsigned int)pr](a, d);
}

// ----------------------------------------------------------------------------
// Memory accesses (non-emu specific)

unsigned int SekReadByte(unsigned int a) { return (unsigned int)ReadByte(a); }
unsigned int SekReadWord(unsigned int a) { return (unsigned int)ReadWord(a); }
unsigned int SekReadLong(unsigned int a) { return ReadLong(a); }

unsigned int SekFetchByte(unsigned int a) { return (unsigned int)FetchByte(a); }
unsigned int SekFetchWord(unsigned int a) { return (unsigned int)FetchWord(a); }
unsigned int SekFetchLong(unsigned int a) { return FetchLong(a); }

void SekWriteByte(unsigned int a, unsigned char d) { WriteByte(a, d); }
void SekWriteWord(unsigned int a, unsigned short d) { WriteWord(a, d); }
void SekWriteLong(unsigned int a, unsigned int d) { WriteLong(a, d); }

void SekWriteByteROM(unsigned int a, unsigned char d) { WriteByteROM(a, d); }
void SekWriteWordROM(unsigned int a, unsigned short d) { WriteWordROM(a, d); }
void SekWriteLongROM(unsigned int a, unsigned int d) { WriteLongROM(a, d); }

// ----------------------------------------------------------------------------
// Initialisation/exit/reset

void SekNewFrame()
{
	nSekCyclesTotal = 0;
}

void SekSetCyclesScanline(int nCycles)
{
	nSekCyclesScanline = nCycles;
}

#ifdef EMU_C68K

unsigned int PicoCheckPc(unsigned int pc)
{
	//dprintf("PicoCheckPc(0x%08X); membase: 0x%08X\n", pc, (int) FIND_F(pc) - (pc & ~SEK_PAGEM) );
	//dprintf("PicoCheckPc(0x%08x);\n", pc - PicoCpu.membase); 
	
	pc -= PicoCpu.membase; // Get real pc
	pc &= 0xffffff;
	
	PicoCpu.membase = (int)FIND_F(pc) - (pc & ~SEK_PAGEM); //PicoMemBase(pc);

	return PicoCpu.membase + pc;
}

static void PicoIrqCallback(int int_level)
{
//	printf("PicoIrqCallback(%d)  irq=0x%04x 0x%08x;\n", int_level, PicoCpu.irq, nSekIRQPending );
	
	if (nSekIRQPending & SEK_IRQSTATUS_AUTO)
		//PicoCpu.irq &= 0x78;
		PicoCpu.irq = 0;
		
	nSekIRQPending = 0;
}

static void PicoResetCallback()
{
	//dprintf("ResetCallback();\n" );
	
	if (pSekExt->ResetCallback) {
		pSekExt->ResetCallback();
	}
}

static int UnrecognizedCallback()
{
	printf("UnrecognizedCallback();\n");
	return 0;
}


#endif

int SekInit(int nCount, int nCPUType)
{
//	dprintf("SekInit(%d, %x);\n", nCount, nCPUType);
	
	struct SekExt* ps = NULL;

	if (nSekActive >= 0) {
		SekClose();
		nSekActive = -1;
	}
	
	if (nCount > nSekCount)
		nSekCount = nCount;
	
//	if (nCount > 0 || nCPUType != 0x68000) {
//		printf("Sorry, it support ONE 68000 cpu ONLY!\n");
//		return 1;
//	}

	// Allocate cpu extenal data (memory map etc)
	//SekExt[nCount] = (struct SekExt*)malloc(sizeof(struct SekExt));
	gp2x_memset(&tSekExt, 0, sizeof(struct SekExt));

	// Put in default memory handlers
	ps = &tSekExt;

	for (int j = 0; j < SEK_MAXHANDLER; j++) {
		ps->ReadByte[j]  = DefReadByte;
		ps->WriteByte[j] = DefWriteByte;
	}

	ps->ReadWord[0]  = DefReadWord0;
	ps->WriteWord[0] = DefWriteWord0;
	ps->ReadLong[0]  = DefReadLong0;
	ps->WriteLong[0] = DefWriteLong0;

#if SEK_MAXHANDLER >= 2
	ps->ReadWord[1]  = DefReadWord1;
	ps->WriteWord[1] = DefWriteWord1;
	ps->ReadLong[1]  = DefReadLong1;
	ps->WriteLong[1] = DefWriteLong1;
#endif

#if SEK_MAXHANDLER >= 3
	ps->ReadWord[2]  = DefReadWord2;
	ps->WriteWord[2] = DefWriteWord2;
	ps->ReadLong[2]  = DefReadLong2;
	ps->WriteLong[2] = DefWriteLong2;
#endif

#if SEK_MAXHANDLER >= 4
	ps->ReadWord[3]  = DefReadWord3;
	ps->WriteWord[3] = DefWriteWord3;
	ps->ReadLong[3]  = DefReadLong3;
	ps->WriteLong[3] = DefWriteLong3;
#endif

#if SEK_MAXHANDLER >= 5
	ps->ReadWord[4]  = DefReadWord4;
	ps->WriteWord[4] = DefWriteWord4;
	ps->ReadLong[4]  = DefReadLong4;
	ps->WriteLong[4] = DefWriteLong4;
#endif

#if SEK_MAXHANDLER >= 6
	ps->ReadWord[5]  = DefReadWord5;
	ps->WriteWord[5] = DefWriteWord5;
	ps->ReadLong[5]  = DefReadLong5;
	ps->WriteLong[5] = DefWriteLong5;
#endif

#if SEK_MAXHANDLER >= 7
	ps->ReadWord[6]  = DefReadWord6;
	ps->WriteWord[6] = DefWriteWord6;
	ps->ReadLong[6]  = DefReadLong6;
	ps->WriteLong[6] = DefWriteLong6;
#endif

#if SEK_MAXHANDLER >= 8
	ps->ReadWord[7]  = DefReadWord7;
	ps->WriteWord[7] = DefWriteWord7;
	ps->ReadLong[7]  = DefReadLong7;
	ps->WriteLong[7] = DefWriteLong7;
#endif

#if SEK_MAXHANDLER >= 9
	for (int j = 8; j < SEK_MAXHANDLER; j++) {
		ps->ReadWord[j]  = DefReadWord0;
		ps->WriteWord[j] = DefWriteWord0;
		ps->ReadLong[j]  = DefReadLong0;
		ps->WriteLong[j] = DefWriteLong0;
	}
#endif


	// Map the normal memory handlers
#ifdef EMU_M68K
		m68k_init();
		if (SekInitCPUM68K(nCount, nCPUType)) {
			SekExit();
			return 1;
		}
#endif

#ifdef EMU_C68K

	if (!bCycloneInited) {
		CycloneInit();
		bCycloneInited = true;
	}
	gp2x_memset(&PicoCpu, 0, sizeof(PicoCpu));
	
	PicoCpu.read8	= ReadByte;
	PicoCpu.read16	= ReadWord;
	PicoCpu.read32	= ReadLong;

	PicoCpu.write8	= WriteByte;
	PicoCpu.write16	= WriteWord;
	PicoCpu.write32	= WriteLong;

	PicoCpu.fetch8	= FetchByte;
	PicoCpu.fetch16	= FetchWord;
	PicoCpu.fetch32	= FetchLong;	
	
	PicoCpu.checkpc = PicoCheckPc;
	
	PicoCpu.IrqCallback = PicoIrqCallback;
	PicoCpu.ResetCallback = PicoResetCallback;
	PicoCpu.UnrecognizedCallback = UnrecognizedCallback;
	
#endif


	nSekIRQPending = 0;

	nSekCyclesTotal = 0;
	nSekCyclesScanline = 0;
	
	pSekExt = &tSekExt;
	
	return 0;
}

#ifdef EMU_C68K
static void PicoReset()
{
	//gp2x_memset(&PicoCpu,0,PicoCpu.pad1-PicoCpu.d); // clear all regs
	gp2x_memset(&PicoCpu, 0, 22 * 4); // clear all regs
	
	PicoCpu.stopped	= 0;
	PicoCpu.srh		= 0x27; // Supervisor mode
	PicoCpu.a[7]	= FetchLong(0); // Stack Pointer
	PicoCpu.membase	= 0;
	PicoCpu.pc		= PicoCpu.checkpc(FetchLong(4)); // Program Counter
}
#endif

int SekExit()
{
	// Deallocate cpu extenal data (memory map etc)
	pSekExt = NULL;

	nSekActive = -1;
	nSekCount = -1;

	return 0;
}

void SekReset()
{
//	dprintf("SekReset();\n");
	
#ifdef EMU_M68K
	m68k_pulse_reset();
#endif

#ifdef EMU_C68K
	PicoReset();
#endif
}

// ----------------------------------------------------------------------------
// Control the active CPU

// Open a CPU
void SekOpen(const int i)
{
	nSekActive = i;
//	pSekExt = &tSekExt;						// Point to cpu context
}

// Close the active cpu
void SekClose()
{
	//nSekCycles[nSekActive] = nSekCyclesTotal;
}

// Set the status of an IRQ line on the active CPU
void SekSetIRQLine(const int line, const int status)
{
//	dprintf("SekSetIRQLine(%d, 0x%08x);\n", line, status);

	if (status) {
		nSekIRQPending = line | status;
		
		nSekCyclesTotal += (nSekCyclesToDo - nSekCyclesDone) - m68k_ICount;
		nSekCyclesDone += (nSekCyclesToDo - nSekCyclesDone) - m68k_ICount;

		PicoCpu.irq = line;
		m68k_ICount = nSekCyclesToDo = -1;			
		return;
	}

	nSekIRQPending = 0;
	
	//PicoCpu.irq &= 0x78;
	PicoCpu.irq = 0;
}

// Adjust the active CPU's timeslice
void SekRunAdjust(const int nCycles)
{
//	dprintf("SekRunAdjust(%d);\n", nCycles);
	
	if (nCycles < 0 && m68k_ICount < -nCycles) {
		SekRunEnd();
		return;
	}

	m68k_ICount += nCycles;
	nSekCyclesToDo += nCycles;
	nSekCyclesSegment += nCycles;
}

// End the active CPU's timeslice
void SekRunEnd()
{
//	dprintf("SekRunEnd();\n");
	
	nSekCyclesTotal += (nSekCyclesToDo - nSekCyclesDone) - m68k_ICount;
	nSekCyclesDone += (nSekCyclesToDo - nSekCyclesDone) - m68k_ICount;
	nSekCyclesSegment = nSekCyclesDone;
	m68k_ICount = nSekCyclesToDo = -1;						// Force A68K to exit

}

// Run the active CPU
int SekRun(const int nCycles)
{
//	dprintf("SekRun(%d);\n", nCycles);

	nSekCyclesDone = 0;
	nSekCyclesSegment = nCycles;
	do {
		m68k_ICount = nSekCyclesToDo = nSekCyclesSegment - nSekCyclesDone;

		if (PicoCpu.irq == 0x80) {						// Cpu is in stopped state till interrupt
			// dprintf("Cpu is in stopped state till interrupt\n", nCycles);
			nSekCyclesDone = nSekCyclesSegment;
			nSekCyclesTotal += nSekCyclesSegment;
		} else {
			CycloneRun(&PicoCpu);
			nSekCyclesDone += nSekCyclesToDo - m68k_ICount;
			nSekCyclesTotal += nSekCyclesToDo - m68k_ICount;
		}
	} while (nSekCyclesDone < nSekCyclesSegment);

	nSekCyclesSegment = nSekCyclesDone;
	nSekCyclesToDo = m68k_ICount = -1;
	nSekCyclesDone = 0;

	return nSekCyclesSegment;		
}

// ----------------------------------------------------------------------------
// Memory map setup

// Note - each page is 1 << SEK_BITS.
int SekMapMemory(unsigned char* pMemory, unsigned int nStart, unsigned int nEnd, int nType)
{
//	dprintf("SekMapMemory(%p, 0x%08X, 0x%08X, 0x%X);\n", pMemory, nStart, nEnd, nType);
	
	unsigned char* Ptr = pMemory - nStart;
	unsigned char** pMemMap = pSekExt->MemMap + (nStart >> SEK_SHIFT);

	// Special case for ROM banks
	if (nType == SM_ROM) {
		for (unsigned int i = (nStart & ~SEK_PAGEM); i <= nEnd; i += SEK_PAGE_SIZE, pMemMap++) {
			pMemMap[0]			  = Ptr + i;
			pMemMap[SEK_WADD * 2] = Ptr + i;
		}

		return 0;
	}

	for (unsigned int i = (nStart & ~SEK_PAGEM); i <= nEnd; i += SEK_PAGE_SIZE, pMemMap++) {

		if (nType & SM_READ) {					// Read
			pMemMap[0]			  = Ptr + i;
		}
		if (nType & SM_WRITE) {					// Write
			pMemMap[SEK_WADD]	  = Ptr + i;
		}
		if (nType & SM_FETCH) {					// Fetch
			pMemMap[SEK_WADD * 2] = Ptr + i;
		}
	}

	return 0;
}

int SekMapHandler(unsigned int nHandler, unsigned int nStart, unsigned int nEnd, int nType)
{
//	dprintf("SekMapHandler(0x%08X, 0x%08X, 0x%08X, 0x%X);\n", nHandler, nStart, nEnd, nType);
	
	unsigned char** pMemMap = pSekExt->MemMap + (nStart >> SEK_SHIFT);

	// Add to memory map
	for (unsigned int i = (nStart & ~SEK_PAGEM); i <= nEnd; i += SEK_PAGE_SIZE, pMemMap++) {

		if (nType & SM_READ) {					// Read
			pMemMap[0]			  = (unsigned char*)nHandler;
		}
		if (nType & SM_WRITE) {					// Write
			pMemMap[SEK_WADD]	  = (unsigned char*)nHandler;
		}
		if (nType & SM_FETCH) {					// Fetch
			pMemMap[SEK_WADD * 2] = (unsigned char*)nHandler;
		}
	}

	return 0;
}

// Set callbacks
int SekSetResetCallback(pSekResetCallback pCallback)
{
//	dprintf("SekSetResetCallback(%p);\n", pCallback);
	
	pSekExt->ResetCallback = pCallback;

	return 0;
}

// Set handlers
int SekSetReadByteHandler(int i, pSekReadByteHandler pHandler)
{
//	dprintf("SekSetReadByteHandler(%d, %p);\n", i, pHandler);
	
	if (i >= SEK_MAXHANDLER) {
		return 1;
	}

	pSekExt->ReadByte[i] = pHandler;

	return 0;
}

int SekSetWriteByteHandler(int i, pSekWriteByteHandler pHandler)
{
//	dprintf("SekSetWriteByteHandler(%d, %p);\n", i, pHandler);
	
	if (i >= SEK_MAXHANDLER) {
		return 1;
	}

	pSekExt->WriteByte[i] = pHandler;

	return 0;
}

int SekSetReadWordHandler(int i, pSekReadWordHandler pHandler)
{
//	dprintf("SekSetReadWordHandler(%d, %p);\n", i, pHandler);
	
	if (i >= SEK_MAXHANDLER) {
		return 1;
	}

	pSekExt->ReadWord[i] = pHandler;

	return 0;
}

int SekSetWriteWordHandler(int i, pSekWriteWordHandler pHandler)
{
//	dprintf("SekSetWriteWordHandler(%d, %p);\n", i, pHandler);
	
	if (i >= SEK_MAXHANDLER) {
		return 1;
	}

	pSekExt->WriteWord[i] = pHandler;

	return 0;
}

int SekSetReadLongHandler(int i, pSekReadLongHandler pHandler)
{
//	dprintf("SekSetReadLongHandler(%d, %p);\n", i, pHandler);
	
	if (i >= SEK_MAXHANDLER) {
		return 1;
	}

	pSekExt->ReadLong[i] = pHandler;

	return 0;
}

int SekSetWriteLongHandler(int i, pSekWriteLongHandler pHandler)
{
//	dprintf("SekSetWriteLongHandler(%d, %p);\n", i, pHandler);
	
	if (i >= SEK_MAXHANDLER) {
		return 1;
	}

	pSekExt->WriteLong[i] = pHandler;

	return 0;
}

// ----------------------------------------------------------------------------
// Query register values

int SekGetPC(int)
{
//	dprintf("SekGetPC(); = 0x%08x\n", PicoCpu.pc-PicoCpu.membase);
	return PicoCpu.pc-PicoCpu.membase;
}

int SekDbgGetCPUType()
{
	return 0x68000;
}

int SekDbgGetPendingIRQ()
{
	return nSekIRQPending & 7;
}

unsigned int SekDbgGetRegister(SekRegister nRegister)
{
	return 0;
}

bool SekDbgSetRegister(SekRegister nRegister, unsigned int nValue)
{
	return false;
}

// ----------------------------------------------------------------------------
// Savestate support

int SekScan(int nAction)
{
	// Scan the 68000 states
	
	struct BurnArea ba;

	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 1;
	}

	gp2x_memset(&ba, 0, sizeof(ba));

	nSekActive = -1;

	for (int i = 0; i <= nSekCount; i++) {
		char szName[] = "Cyclone #n";
		int nType = nSekCPUType;	//nSekCPUType[i];

		szName[9] = '0' + i;

		SCAN_VAR(nSekCPUType);

		//if (nSekCPUType != 0) {
			ba.Data = & PicoCpu;
			//ba.nLen = nSekM68KContextSize[i];
			ba.nLen = 24 * 4;
			ba.szName = szName;
			BurnAcb(&ba);
		//}

	}

	return 0;
}
