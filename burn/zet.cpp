// Z80 (Zed Eight-Ty) Interface
#include "burnint.h"
#include "gp2xmemfuncs.h"

#ifdef EMU_DRZ80

struct DrZ80 Doze;
struct DrZ80 *ZetCPUContext = NULL;
#endif // EMU_DRZ80

static int nOpenedCPU = -1;
static int nCPUCount = 0;

unsigned char __fastcall ZetDummyReadHandler(unsigned short) { return 0; }
void __fastcall ZetDummyWriteHandler(unsigned short, unsigned char) { }
unsigned char __fastcall ZetDummyInHandler(unsigned short) { return 0; }
void __fastcall ZetDummyOutHandler(unsigned short, unsigned char) { }

static void Z80DebugCallback(unsigned short pc, unsigned int d)
{
	printf("z80 error at PC: 0x%08x   OpCodes: %08X\n", pc, d);
}

static unsigned int z80_rebasePC(unsigned short address)
{
	Doze.Z80PC_BASE	= (unsigned int) Doze.ppMemFetch[ address >> 8 ];
	Doze.Z80PC		= Doze.Z80PC_BASE + address;
	return Doze.Z80PC;
}

static void z80_write8(unsigned short a, unsigned char d)
{
	//printf("z80_write8(0x%04x, 0x%04x);\n", d, a);
	unsigned char * p = Doze.ppMemWrite[ a >> 8 ];
	if ( p )
		* (p + a) = d;
	else
		; //Z80.WriteHandler(d, a);
}

static void z80_write16(unsigned short d,unsigned short a)
{
//	printf("z80_write16(0x%04x, 0x%04x);\n", a, d);
	unsigned char * p = Doze.ppMemWrite[ a >> 8 ];
	if ( p ) {
		p+=a;	*p = d & 0xff;
		p++;	*p = d >> 8;
	} else {
		printf("z80_write16(0x%04x, 0x%04x); error at pc: 0x%04x\n", d, a, Doze.Z80PC - Doze.Z80PC_BASE);
		//z80_write8( (unsigned char)(d & 0xFF), a );
		//z80_write8( (unsigned char)(d >> 8), a + 1 );
		//Doze.z80_write8 ( a, d & 0xff );
		//Doze.z80_write8 ( a + 1, d >> 8 );
	}
} 

static unsigned char z80_read8(unsigned short a)
{
	//printf("z80_read8(0x%04x);  PC: %08x\n", a, Doze.Z80PC - Doze.Z80PC_BASE);
	unsigned char * p = Doze.ppMemRead[ a >> 8 ];
	if ( p )
		return *(p + a);
	else
		return 0; // Doze.ReadHandler(a);
}

static unsigned short z80_read16(unsigned short a)
{
//	printf("z80_read16(0x%04x);\n", a);
	
	unsigned short d = 0;
	unsigned char * p = Doze.ppMemRead[ a >> 8 ];
	if ( p ) {
		p+=a;	d = *p;
		p++;	d |= (*p) << 8;
	} else {
		printf("z80_read16(0x%04x); error at pc: 0x%04x\n", a, Doze.Z80PC - Doze.Z80PC_BASE);
		//return z80_read8(a) | (z80_read8(a+1) << 8) ;
	}
	return d;
}

static void z80_irq_callback(void)
{
	//printf("z80_irq_callback();\n");
	Doze.Z80_IRQ = 0x00;
}

void ZetSetReadHandler(unsigned char (__fastcall *pHandler)(unsigned short))
{
	//printf("ZetSetReadHandler(%p);\n", pHandler);
	Doze.z80_read8 = pHandler;
}

void ZetSetWriteHandler(void (__fastcall *pHandler)(unsigned short, unsigned char))
{
	//printf("ZetSetWriteHandler(%p);\n", pHandler);
	Doze.z80_write8 = pHandler;
}

void ZetSetInHandler(unsigned char (__fastcall *pHandler)(unsigned short))
{
	//printf("ZetSetInHandler(%p);\n", pHandler);
	Doze.z80_in = pHandler;
}

void ZetSetOutHandler(void (__fastcall *pHandler)(unsigned short, unsigned char))
{
	//printf("ZetSetOutHandler(%p);\n", pHandler);
	Doze.z80_out = pHandler;
}

void ZetNewFrame()
{
//	printf("ZetNewFrame();\n");
	for (int i = 0; i < nCPUCount; i++) {
		ZetCPUContext[i].nCyclesTotal = 0;
	}
	Doze.nCyclesTotal = 0;
}

int ZetInit(int nCount)
{
	ZetCPUContext = (DrZ80 *)malloc(nCount * sizeof(DrZ80));
	if (ZetCPUContext == NULL) {
		return 1;
	}

	gp2x_memset(ZetCPUContext, 0, nCount * sizeof(DrZ80));

	for (int i = 0; i < nCount; i++)
	{
		ZetCPUContext[i].z80_in			= ZetDummyInHandler;
		ZetCPUContext[i].z80_out		= ZetDummyOutHandler;
		ZetCPUContext[i].z80_rebasePC	= z80_rebasePC;
		//Doze.z80_rebaseSP	= z80_rebaseSP;
		ZetCPUContext[i].z80_read8		= z80_read8;
		ZetCPUContext[i].z80_read16		= z80_read16;
		ZetCPUContext[i].z80_write8		= z80_write8;
		ZetCPUContext[i].z80_write16	= z80_write16;
/*
		ZetCPUContext[i].z80_read8		= ZetDummyReadHandler;
		ZetCPUContext[i].z80_read16		= z80_read16;
		ZetCPUContext[i].z80_write8		= ZetDummyWriteHandler;
		ZetCPUContext[i].z80_write16	= z80_write16;
*/
		ZetCPUContext[i].z80_irq_callback=z80_irq_callback;
		ZetCPUContext[i].debugCallback	= Z80DebugCallback;
	
		ZetCPUContext[i].nInterruptLatch = -1;

		ZetCPUContext[i].ppMemFetch = (unsigned char**)malloc(0x0100 * sizeof(char*));
		ZetCPUContext[i].ppMemFetchData = (unsigned char**)malloc(0x0100 * sizeof(char*));
		ZetCPUContext[i].ppMemRead = (unsigned char**)malloc(0x0100 * sizeof(char*));
		ZetCPUContext[i].ppMemWrite = (unsigned char**)malloc(0x0100 * sizeof(char*));

		if (ZetCPUContext[i].ppMemFetch == NULL || ZetCPUContext[i].ppMemFetchData == NULL || ZetCPUContext[i].ppMemRead == NULL || ZetCPUContext[i].ppMemWrite == NULL) {
			ZetExit();
			return 1;
		}

		gp2x_memset( ZetCPUContext[i].ppMemFetch, 0, 0x0400 );
		gp2x_memset( ZetCPUContext[i].ppMemFetchData, 0, 0x0400 );
		gp2x_memset( ZetCPUContext[i].ppMemRead, 0, 0x0400 );
		gp2x_memset( ZetCPUContext[i].ppMemWrite, 0, 0x0400 );
	}
		
	ZetOpen(0);

	nCPUCount = nCount;

	return 0;
}

void ZetClose()
{
	ZetCPUContext[nOpenedCPU] = Doze;
	nOpenedCPU = -1;
}

int ZetOpen(int nCPU)
{
	Doze = ZetCPUContext[nCPU];
	nOpenedCPU = nCPU;

	return 0;
}

#ifdef EMU_DRZ80

void DozeAsmCall(unsigned short a)
{
	//IFLOG printf("DozeAsmCall(0x%04x); PC: 0x%04x SP: 0x%04x", a, Doze.Z80PC-Doze.Z80PC_BASE, Doze.Z80SP);
	
	int pc = Doze.Z80PC-Doze.Z80PC_BASE;
	
	Doze.Z80SP -= 2;
	
	unsigned char * p = Doze.ppMemWrite[ Doze.Z80SP >> 8 ];
	if ( p ) {
		p += Doze.Z80SP;
		*p = pc & 0xff;
		p++;
		*p = pc >> 8;
		//*((unsigned char *)Doze.Z80SP) = pc & 0xff;
		//*((unsigned char *)(Doze.Z80SP+1)) = pc >> 8;
	} else {
		
		printf("DozeAsmCall Error PUSH PC!\n");
	}
	
	Doze.Z80PC  = Doze.z80_rebasePC( a );
	
	//IFLOG printf("-> 0x%04x 0x%04x\n", Doze.Z80PC-Doze.Z80PC_BASE, Doze.Z80SP);
}

static unsigned char DozeAsmRead(int a)
{
	unsigned char * p = Doze.ppMemRead[ a >> 8 ];
	if ( p )
		return *(p + a);
	else
		return 0; // Doze.ReadHandler(a);
}

static int Interrupt(int nVal)
{
	//if ((Z80.Z80IF & 0x03) == 0) 					// ???? 
	if ((Doze.Z80IF & 0xFF) == 0) 					// not enabled
		return 0;

	//IFLOG printf("    IRQ taken  nIF: %02x  nIM: %02x PC: 0x%04x OpCode: 0x%02x\n", Doze.Z80IF, Doze.Z80IM, Doze.Z80PC-Doze.Z80PC_BASE, DozeAsmRead(Doze.Z80PC-Doze.Z80PC_BASE) );
	
	if ( DozeAsmRead(Doze.Z80PC-Doze.Z80PC_BASE) == 0x76 )
		Doze.Z80PC ++;
	
	Doze.Z80IF = 0;

	if (Doze.Z80IM == 0) {
		DozeAsmCall((unsigned short)(nVal & 0x38));	// rst nn
		return 13;									// cycles done
	} else {
		if (Doze.Z80IM == 2) {
			int nTabAddr = 0, nIntAddr = 0;
printf("Doze.Z80IM == 2\n");			
			// Get interrupt address from table (I points to the table)
			nTabAddr = (Doze.Z80I & 0xFF00) + nVal;

			// Read 16-bit table value
			nIntAddr  = DozeAsmRead((unsigned short)(nTabAddr + 1)) << 8;
			nIntAddr |= DozeAsmRead((unsigned short)(nTabAddr));

			DozeAsmCall((unsigned short)(nIntAddr));
			return 19;								// cycles done
		} else {
			DozeAsmCall(0x38);						// rst 38h
			return 13;								// cycles done
		}
	}
}

static inline void TryInt()
{
	int nDid;

	if (Doze.nInterruptLatch & ZET_IRQSTATUS_NONE) return;

	nDid = Interrupt(Doze.nInterruptLatch & 0xFF);	// Success! we did some cycles, and took the interrupt
	if (nDid > 0 && (Doze.nInterruptLatch & ZET_IRQSTATUS_AUTO)) {
		Doze.nInterruptLatch = ZET_IRQSTATUS_NONE;
	}

	Doze.nCyclesLeft -= nDid;
}

static void DozeRun()
{
	TryInt();

	if (Doze.nCyclesLeft < 0) {
		//printf("DozeRun() -- (nCyclesLeft < 0)\n");		
		return;
	}

	if (DozeAsmRead(Doze.Z80PC-Doze.Z80PC_BASE) == 0x76) {
		
		//IFLOG printf("DozeRun() -- (*pc == 0x76)\n");
		
		// cpu is halted (repeatedly doing halt inst.)
		int nDid = (Doze.nCyclesLeft >> 2) + 1;
		Doze.Z80I = (unsigned short)(((Doze.Z80I + nDid) & 0x7F) | (Doze.Z80I & 0xFF80)); // Increase R register
		Doze.nCyclesLeft -= nDid;
		return;
	}
	
	// Find out about mid-exec EIs
	Doze.nEI = 1;
	DrZ80Run(&Doze, Doze.nCyclesLeft);

#if 0
	IFLOG printf("CyclesLeft %d\n", Doze.nCyclesLeft );
	IFLOG printf(" AF: 0x%04x BC: 0x%04x DE: 0x%04x HL: 0x%04x\n", Doze.Z80A >> 16 | Doze.Z80F, Doze.Z80BC >> 16,  Doze.Z80DE >> 16,  Doze.Z80HL >> 16 );
	IFLOG printf(" AF' 0x%04x BC' 0x%04x DE' 0x%04x HL' 0x%04x\n", Doze.Z80A2 >> 16 | Doze.Z80F2, Doze.Z80BC2 >> 16,  Doze.Z80DE2 >> 16,  Doze.Z80HL2 >> 16 );
	IFLOG printf(" IX: 0x%04x IY: 0x%04x PC: 0x%04x SP: 0x%04x\n", Doze.Z80IX >> 16,  Doze.Z80IY >> 16,  Doze.Z80PC-Doze.Z80PC_BASE,  Doze.Z80SP );
	IFLOG printf(" IR: 0x%08x IF: 0x%02x IM: 0x%02x\n", Doze.Z80I,  Doze.Z80IF,  Doze.Z80IM );
#endif	

//IFLOG printf("Find out about mid-exec EIs: 0x%08x\n", Doze.nEI );

	// Just enabled interrupts
	while (Doze.nEI == 2) {


		//printf("    EI executed\n");
		// (do one more instruction before interrupt)
		int nTodo = Doze.nCyclesLeft;
		Doze.nCyclesLeft = 0;
		Doze.nEI = 0;

		DrZ80Run(&Doze, Doze.nCyclesLeft);
		
#if 0
	IFLOG printf("CyclesLeft %d\n", Doze.nCyclesLeft );
	IFLOG printf(" AF: 0x%04x BC: 0x%04x DE: 0x%04x HL: 0x%04x\n", Doze.Z80A >> 16 | Doze.Z80F, Doze.Z80BC >> 16,  Doze.Z80DE >> 16,  Doze.Z80HL >> 16 );
	IFLOG printf(" AF' 0x%04x BC' 0x%04x DE' 0x%04x HL' 0x%04x\n", Doze.Z80A2 >> 16 | Doze.Z80F2, Doze.Z80BC2 >> 16,  Doze.Z80DE2 >> 16,  Doze.Z80HL2 >> 16 );
	IFLOG printf(" IX: 0x%04x IY: 0x%04x PC: 0x%04x SP: 0x%04x\n", Doze.Z80IX >> 16,  Doze.Z80IY >> 16,  Doze.Z80PC-Doze.Z80PC_BASE,  Doze.Z80SP );
	IFLOG printf(" IR: 0x%08x IF: 0x%02x IM: 0x%02x\n", Doze.Z80I,  Doze.Z80IF,  Doze.Z80IM );
#endif
		
		Doze.nCyclesLeft += nTodo;

		TryInt();

		// And continue the rest of the exec
		DrZ80Run(&Doze, Doze.nCyclesLeft);
#if 0
	IFLOG printf("CyclesLeft %d\n", Doze.nCyclesLeft );
	IFLOG printf(" AF: 0x%04x BC: 0x%04x DE: 0x%04x HL: 0x%04x\n", Doze.Z80A >> 16 | Doze.Z80F, Doze.Z80BC >> 16,  Doze.Z80DE >> 16,  Doze.Z80HL >> 16 );
	IFLOG printf(" AF' 0x%04x BC' 0x%04x DE' 0x%04x HL' 0x%04x\n", Doze.Z80A2 >> 16 | Doze.Z80F2, Doze.Z80BC2 >> 16,  Doze.Z80DE2 >> 16,  Doze.Z80HL2 >> 16 );
	IFLOG printf(" IX: 0x%04x IY: 0x%04x PC: 0x%04x SP: 0x%04x\n", Doze.Z80IX >> 16,  Doze.Z80IY >> 16,  Doze.Z80PC-Doze.Z80PC_BASE,  Doze.Z80SP );
	IFLOG printf(" IR: 0x%08x IF: 0x%02x IM: 0x%02x\n", Doze.Z80I,  Doze.Z80IF,  Doze.Z80IM );
#endif
		
	}
	
//IFLOG printf("DozeRun(); %d  PC: %04x  PS: %04x\n", Doze.nCyclesLeft, Doze.Z80PC - Doze.Z80PC_BASE, Doze.Z80SP );
}

#endif

int ZetRun(int nCycles)
{
	if (nCycles <= 0) {
		return 0;
	}

	Doze.nCyclesTotal += nCycles;
	Doze.nCyclesSegment = nCycles;
	Doze.nCyclesLeft = nCycles;

	DozeRun();
	nCycles = Doze.nCyclesSegment - Doze.nCyclesLeft;

	Doze.nCyclesTotal -= Doze.nCyclesLeft;
	Doze.nCyclesLeft = 0;
	Doze.nCyclesSegment = 0;

	return nCycles;
}

void ZetRunAdjust(int nCycles)
{
	//printf("ZetRunAdjust(%d);\n", nCycles);
	
	if (nCycles < 0 && Doze.nCyclesLeft < -nCycles) {
		nCycles = 0;
	}

	Doze.nCyclesTotal += nCycles;
	Doze.nCyclesSegment += nCycles;
	Doze.nCyclesLeft += nCycles;
}

void ZetRunEnd()
{
//	printf("ZetRunEnd();\n");
	
	Doze.nCyclesTotal -= Doze.nCyclesLeft;
	Doze.nCyclesSegment -= Doze.nCyclesLeft;
	Doze.nCyclesLeft = 0;

}

// This function will make an area callback ZetRead/ZetWrite
int ZetMemCallback(int nStart, int nEnd, int nMode)
{
//	printf("ZetMemCallback(0x%04x, 0x%04x, %d);\n", nStart, nEnd, nMode);
	
	nStart >>= 8;
	nEnd += 0xff;
	nEnd >>= 8;

	// Leave the section out of the memory map, so the Doze* callback with be used
	for (int i = nStart; i < nEnd; i++) {
		switch (nMode) {
			case 0:
				Doze.ppMemRead[i] = NULL;
				break;
			case 1:
				Doze.ppMemWrite[i] = NULL;
				break;
			case 2:
				Doze.ppMemFetch[i] = NULL;
				break;
		}
	}

	return 0;
}

int ZetMemEnd()
{
	return 0;
}

void ZetExit()
{
	for (int i = 0; i < nCPUCount; i++) {
		free(ZetCPUContext[i].ppMemFetch);
		ZetCPUContext[i].ppMemFetch = NULL;
		free(ZetCPUContext[i].ppMemFetchData);
		ZetCPUContext[i].ppMemFetchData = NULL;
		free(ZetCPUContext[i].ppMemRead);
		ZetCPUContext[i].ppMemRead = NULL;
		free(ZetCPUContext[i].ppMemWrite);
		ZetCPUContext[i].ppMemWrite = NULL;
	}

	free(ZetCPUContext);
	ZetCPUContext = NULL;
	//gp2x_memset(&Doze, 0, sizeof(Doze));
	nCPUCount = 0;
}

int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem)
{
	//printf("ZetMapArea(0x%04x, 0x%04x, %d, %p);\n", nStart, nEnd, nMode, Mem);
	
	int s = nStart >> 8;
	int e = (nEnd + 0xFF) >> 8;

	// Put this section in the memory map, giving the offset from Z80 memory to PC memory
	for (int i = s; i < e; i++) {
		switch (nMode) {
			case 0:
				Doze.ppMemRead[i] = Mem - nStart;
				break;
			case 1:
				Doze.ppMemWrite[i] = Mem - nStart;
				break;
			case 2:
				Doze.ppMemFetch[i] = Mem - nStart;
				Doze.ppMemFetchData[i] = Mem - nStart;
				break;
		}
	}

	return 0;
}

int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem01, unsigned char *Mem02)
{
	//printf("ZetMapArea(0x%04x, 0x%04x, %d, %p, %p);\n", nStart, nEnd, nMode, Mem01, Mem02);
	
	int s = nStart >> 8;
	int e = (nEnd + 0xFF) >> 8;

	if (nMode != 2) {
		return 1;
	}

	// Put this section in the memory map, giving the offset from Z80 memory to PC memory
	for (int i = s; i < e; i++) {
		Doze.ppMemFetch[i] = Mem01 - nStart;
		Doze.ppMemFetchData[i] = Mem02 - nStart;
	}

	return 0;
}

int ZetReset()
{
	//printf("ZetReset();\n");
	
	Doze.spare			= 0;
	Doze.z80irqvector	= 0;
	
  	Doze.Z80A			= 0x00 <<24;
  	Doze.Z80F			= (1<<2); /* set ZFlag */
  	Doze.Z80BC			= 0x0000<<16;
  	Doze.Z80DE			= 0x0000<<16;
  	Doze.Z80HL			= 0x0000<<16;
  	Doze.Z80A2			= 0x00<<24;
  	Doze.Z80F2			= 1<<2;  /* set ZFlag */
  	Doze.Z80BC2			= 0x0000<<16;
  	Doze.Z80DE2			= 0x0000<<16;
  	Doze.Z80HL2			= 0x0000<<16;
  	Doze.Z80IX			= 0xFFFF<<16;
  	Doze.Z80IY			= 0xFFFF<<16;
	Doze.Z80I			= 0x00;
  	Doze.Z80IM			= 0x00;
  	Doze.Z80_IRQ		= 0x00;
  	Doze.Z80IF			= 0x00;
  	Doze.Z80PC			= Doze.z80_rebasePC(0);
  	//Doze.Z80SP			= 0Doze.z80_rebaseSP(0xffff); /* 0xf000 */
  	Doze.Z80SP			= 0x0000;
  	
  	//Doze.nInterruptLatch = 0;
  	//Doze.nEI			= 0;
  	//Doze.nCyclesLeft	= 0;
  	//Doze.nCyclesTotal	= 0;
  	//Doze.nCyclesSegment	= 0;

	return 0;
}

#ifdef EMU_DRZ80

int ZetNmi()
{
//	printf("ZetNmi();\n");

	//Doze.iff &= 0xFF00;						// reset iff1
	//DozeAsmCall((unsigned short)0x66);		// Do nmi
	Doze.Z80IF &= 0x02;						// reset iff1
	DozeAsmCall((unsigned short)0x66);		// Do nmi
	Doze.nCyclesTotal += 12;
	return 12;
}

#endif

int ZetPc(int n)
{
	//printf("ZetPc(%d);\n", n);
	if (n < 0) {
		return Doze.Z80PC - Doze.Z80PC_BASE;
	} else {
		return ZetCPUContext[n].Z80PC -ZetCPUContext[n].Z80PC_BASE;
	}
	return Doze.Z80PC - Doze.Z80PC_BASE;
}

int ZetScan(int nAction)
{
	//printf("ZetScan(%d);\n", nAction);
	
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 0;
	}

	char szText[] = "Z80 #0";
	for (int i = 0; i < nCPUCount; i++) {
		szText[5] = '1' + i;
		ScanVar(&Doze, 19 * 4 + 16, szText);
	}

	return 0;
}
