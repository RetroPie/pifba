// Z80 (Zed Eight-Ty) Interface
#include "burnint.h"
#include "zet.h"

#ifdef EMU_DOZE
 static DozeContext * ZetCPUContextDZ80 = NULL;
#endif // EMU_DOZE

#ifdef EMU_MAME_Z80
 static struct ZetExt * ZetCPUContextZ80 = NULL;
 static struct ZetExt * lastZetCPUContextZ80 = NULL;
#endif

#ifdef EMU_CZ80
 static cz80_struc * ZetCPUContext = NULL;
 static cz80_struc * lastZetCPUContext = NULL;
#endif

int nZetCpuCore = 1; // 0 - CZ80, 1 - MAME_Z80; not switchable on the fly, set before initializing!
static int nOpenedCPU = -1;
static int nCPUCount = 0;
int nHasZet = -1;

unsigned char __fastcall ZetDummyReadHandler(unsigned short) { return 0; }
void __fastcall ZetDummyWriteHandler(unsigned short, unsigned char) { }
unsigned char __fastcall ZetDummyInHandler(unsigned short) { return 0; }
void __fastcall ZetDummyOutHandler(unsigned short, unsigned char) { }

#ifdef EMU_MAME_Z80

unsigned char __fastcall ZetReadIO(unsigned int a)
{
	return lastZetCPUContextZ80->ZetIn(a);
}

void __fastcall ZetWriteIO(unsigned int a, unsigned char d)
{
	lastZetCPUContextZ80->ZetOut(a, d);
}

unsigned char __fastcall ZetReadProg(unsigned int a)
{
	unsigned char *pr = lastZetCPUContextZ80->pZetMemMap[0x000 + (a >> 8)];
	if (pr != NULL) return pr[a & 0xff];
	
	return lastZetCPUContextZ80->ZetRead(a);
}

void __fastcall ZetWriteProg(unsigned int a, unsigned char d)
{
	unsigned char *pr = lastZetCPUContextZ80->pZetMemMap[0x100 + (a >> 8)];
	if (pr != NULL) {
		pr[a & 0xff] = d;
		return;
	}
	lastZetCPUContextZ80->ZetWrite(a, d);	
//	bprintf(PRINT_NORMAL, _T("Write %x, %x\n"), a, d);
}

unsigned char __fastcall ZetReadOp(unsigned int a)
{
	unsigned char *pr = lastZetCPUContextZ80->pZetMemMap[0x200 + (a >> 8)];
	if (pr != NULL) return pr[a & 0xff];

	bprintf(PRINT_NORMAL, _T("Op Read %x\n"), a);
	return 0;
}

unsigned char __fastcall ZetReadOpArg(unsigned int a)
{
	unsigned char *pr = lastZetCPUContextZ80->pZetMemMap[0x300 + (a >> 8)];
	if (pr != NULL) return pr[a & 0xff];
	
	bprintf(PRINT_NORMAL, _T("Op Arg Read %x\n"), a);	
	return 0;
}
#endif

void ZetSetReadHandler(unsigned char (__fastcall *pHandler)(unsigned short))
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) Doze.ReadHandler = pHandler;
#endif
#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) lastZetCPUContextZ80->ZetRead = pHandler;
#endif
#ifdef EMU_CZ80
	if(nZetCpuCore == 0) lastZetCPUContext->Read_Byte = pHandler;
#endif
}

void ZetSetWriteHandler(void (__fastcall *pHandler)(unsigned short, unsigned char))
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) Doze.WriteHandler = pHandler;
#endif
#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) lastZetCPUContextZ80->ZetWrite = pHandler;
#endif
#ifdef EMU_CZ80
	if(nZetCpuCore == 0) lastZetCPUContext->Write_Byte = pHandler;
#endif
}

void ZetSetInHandler(unsigned char (__fastcall *pHandler)(unsigned short))
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) Doze.InHandler = pHandler;
#endif
#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) lastZetCPUContextZ80->ZetIn = pHandler;
#endif
#ifdef EMU_CZ80
	if(nZetCpuCore == 0) lastZetCPUContext->IN_Port = pHandler;
#endif
}

void ZetSetOutHandler(void (__fastcall *pHandler)(unsigned short, unsigned char))
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) Doze.OutHandler = pHandler;
#endif
#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) lastZetCPUContextZ80->ZetOut = pHandler;
#endif
#ifdef EMU_CZ80
	if(nZetCpuCore == 0) lastZetCPUContext->OUT_Port = pHandler;
#endif
}

void ZetNewFrame()
{
#ifdef EMU_DOZE	
	if(nZetCpuCore == 2) {
		for (int i = 0; i < nCPUCount; i++) {
			ZetCPUContextDZ80[i].nCyclesTotal = 0;
		}

		Doze.nCyclesTotal = 0;
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) 
		for (int i = 0; i < nCPUCount; i++) {
			ZetCPUContextZ80[i].nCyclesTotal = 0;
		}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) 
		for (int i = 0; i < nCPUCount; i++) {
			ZetCPUContext[i].nCyclesTotal = 0;
		}
#endif

}

int ZetInit(int nCount)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		ZetCPUContextDZ80 = (DozeContext*)malloc(nCount * sizeof(DozeContext));
		if (ZetCPUContextDZ80 == NULL) {
			return 1;
		}

		memset(ZetCPUContextDZ80, 0, nCount * sizeof(DozeContext));

		for (int i = 0; i < nCount; i++) {
			ZetCPUContextDZ80[i].nInterruptLatch = -1;

			ZetCPUContextDZ80[i].ReadHandler = ZetDummyReadHandler;
			ZetCPUContextDZ80[i].WriteHandler = ZetDummyWriteHandler;
			ZetCPUContextDZ80[i].InHandler = ZetDummyInHandler;
			ZetCPUContextDZ80[i].OutHandler = ZetDummyOutHandler;

			ZetCPUContextDZ80[i].ppMemFetch = (unsigned char**)malloc(0x0100 * sizeof(char*));
			ZetCPUContextDZ80[i].ppMemFetchData = (unsigned char**)malloc(0x0100 * sizeof(char*));
			ZetCPUContextDZ80[i].ppMemRead = (unsigned char**)malloc(0x0100 * sizeof(char*));
			ZetCPUContextDZ80[i].ppMemWrite = (unsigned char**)malloc(0x0100 * sizeof(char*));

			if (ZetCPUContextDZ80[i].ppMemFetch == NULL || ZetCPUContextDZ80[i].ppMemFetchData == NULL || ZetCPUContextDZ80[i].ppMemRead == NULL || ZetCPUContextDZ80[i].ppMemWrite == NULL) {
				ZetExit();
				return 1;
			}

			for (int j = 0; j < 0x0100; j++) {
				ZetCPUContextDZ80[i].ppMemFetch[j] = NULL;
				ZetCPUContextDZ80[i].ppMemFetchData[j] = NULL;
				ZetCPUContextDZ80[i].ppMemRead[j] = NULL;
				ZetCPUContextDZ80[i].ppMemWrite[j] = NULL;
			}
		}

		ZetOpen(0);

		nCPUCount = nCount;
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		ZetCPUContextZ80 = (struct ZetExt *) malloc(nCount * sizeof(struct ZetExt));
		if (ZetCPUContextZ80 == NULL) return 1;
		memset(ZetCPUContextZ80, 0, nCount * sizeof(struct ZetExt));
		
		Z80Init();
		
		for (int i = 0; i < nCount; i++) {
			ZetCPUContextZ80[i].ZetIn = ZetDummyInHandler;
			ZetCPUContextZ80[i].ZetOut = ZetDummyOutHandler;
			ZetCPUContextZ80[i].ZetRead = ZetDummyReadHandler;
			ZetCPUContextZ80[i].ZetWrite = ZetDummyWriteHandler;
			// TODO: Z80Init() will set IX IY F regs with default value, so get them ...
			Z80GetContext(& (ZetCPUContextZ80[i].reg) );
		}
		
		Z80SetIOReadHandler(ZetReadIO);
		Z80SetIOWriteHandler(ZetWriteIO);
		Z80SetProgramReadHandler(ZetReadProg);
		Z80SetProgramWriteHandler(ZetWriteProg);
		Z80SetCPUOpReadHandler(ZetReadOp);
		Z80SetCPUOpArgReadHandler(ZetReadOpArg);
		
		ZetOpen(0);
		
		nCPUCount = nCount;
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		ZetCPUContext = (cz80_struc *) malloc(nCount * sizeof(cz80_struc));
		if (ZetCPUContext == NULL) return 1;
		Cz80_InitFlags();
		//memset(ZetCPUContext, 0, nCount * sizeof(cz80_struc));
		
		for (int i = 0; i < nCount; i++) {
			Cz80_Init( &ZetCPUContext[i] );
			ZetCPUContext[i].nInterruptLatch = -1;
			ZetCPUContext[i].IN_Port = ZetDummyInHandler;
			ZetCPUContext[i].OUT_Port = ZetDummyOutHandler;
			ZetCPUContext[i].Read_Byte = ZetDummyReadHandler;
			ZetCPUContext[i].Write_Byte = ZetDummyWriteHandler;
		}
		ZetOpen(0);
		nCPUCount = nCount;
	}
#endif

	nHasZet = nCount;
	return 0;
}

unsigned char ZetReadByte(unsigned short address)
{
	if (nOpenedCPU < 0) return 0;

#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		if (ZetCPUContextDZ80[nOpenedCPU].ppMemRead[address>>8] != NULL) {
			return ZetCPUContextDZ80[nOpenedCPU].ppMemRead[address>>8][address];
		}

		if (ZetCPUContextDZ80[nOpenedCPU].ReadHandler != NULL) {
			return ZetCPUContextDZ80[nOpenedCPU].ReadHandler(address);
		}
	}
#endif

	return 0;
}

void ZetWriteByte(unsigned short address, unsigned char data)
{
	if (nOpenedCPU < 0) return;

#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		if (ZetCPUContextDZ80[nOpenedCPU].ppMemWrite[address>>8] != NULL) {
			ZetCPUContextDZ80[nOpenedCPU].ppMemWrite[address>>8][address] = data;
		}

		if (ZetCPUContextDZ80[nOpenedCPU].WriteHandler != NULL) {
			ZetCPUContextDZ80[nOpenedCPU].WriteHandler(address, data);
		}
	}
#endif
}

void ZetClose()
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) ZetCPUContextDZ80[nOpenedCPU] = Doze;
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) 
	// Set handlers here too
	if (nOpenedCPU >= 0)
		Z80GetContext(&(ZetCPUContextZ80[nOpenedCPU].reg));
#endif

#ifdef EMU_CZ80
	//
#endif

	nOpenedCPU = -1;
}

int ZetOpen(int nCPU)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) Doze = ZetCPUContextDZ80[nCPU];
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		// Set handlers here too
		Z80SetContext(&ZetCPUContextZ80[nCPU].reg);
		lastZetCPUContextZ80 = &ZetCPUContextZ80[nCPU];
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) lastZetCPUContext = &ZetCPUContext[nCPU];
#endif

	nOpenedCPU = nCPU;

	return 0;
}


int ZetGetActive()
{
	return nOpenedCPU;
}

int ZetRun(int nCycles)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		if (nCycles <= 0 || ZetCPUContextDZ80 == 0) return 0;
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
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		if (nCycles <= 0 || ZetCPUContextZ80 == 0) return 0;
		lastZetCPUContextZ80->nCyclesTotal += nCycles;
		lastZetCPUContextZ80->nCyclesSegment = nCycles;
		lastZetCPUContextZ80->nCyclesLeft = nCycles;
		
		nCycles = Z80Execute(nCycles);
		
		lastZetCPUContextZ80->nCyclesLeft = lastZetCPUContextZ80->nCyclesLeft - nCycles;
		lastZetCPUContextZ80->nCyclesTotal -= lastZetCPUContextZ80->nCyclesLeft;
		lastZetCPUContextZ80->nCyclesLeft = 0;
		lastZetCPUContextZ80->nCyclesSegment = 0;
		
		return nCycles;
	}
#endif


#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		if (nCycles <= 0 || ZetCPUContext == 0) return 0;
		lastZetCPUContext->nCyclesTotal += nCycles;
		lastZetCPUContext->nCyclesSegment = nCycles;
		lastZetCPUContext->nCyclesLeft = nCycles;

		nCycles = Cz80_Exec(lastZetCPUContext);
		//nCycles = Doze.nCyclesSegment - Doze.nCyclesLeft;

		lastZetCPUContext->nCyclesTotal -= lastZetCPUContext->nCyclesLeft;
		lastZetCPUContext->nCyclesLeft = 0;
		lastZetCPUContext->nCyclesSegment = 0;

		return nCycles;
	}
#endif

}

void ZetRunAdjust(int nCycles)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		if (nCycles < 0 && Doze.nCyclesLeft < -nCycles) {
			nCycles = 0;
		}

		Doze.nCyclesTotal += nCycles;
		Doze.nCyclesSegment += nCycles;
		Doze.nCyclesLeft += nCycles;
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		if (nCycles < 0 && lastZetCPUContextZ80->nCyclesLeft < -nCycles) {
			nCycles = 0;
		}

		lastZetCPUContextZ80->nCyclesTotal += nCycles;
		lastZetCPUContextZ80->nCyclesSegment += nCycles;
		lastZetCPUContextZ80->nCyclesLeft += nCycles;
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		if (nCycles < 0 && lastZetCPUContext->nCyclesLeft < -nCycles) {
			nCycles = 0;
		}

		lastZetCPUContext->nCyclesTotal += nCycles;
		lastZetCPUContext->nCyclesSegment += nCycles;
		lastZetCPUContext->nCyclesLeft += nCycles;
	}
#endif

}

void ZetRunEnd()
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		Doze.nCyclesTotal -= Doze.nCyclesLeft;
		Doze.nCyclesSegment -= Doze.nCyclesLeft;
		Doze.nCyclesLeft = 0;
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		lastZetCPUContextZ80->nCyclesTotal -= lastZetCPUContextZ80->nCyclesLeft;
		lastZetCPUContextZ80->nCyclesSegment -= lastZetCPUContextZ80->nCyclesLeft;
		lastZetCPUContextZ80->nCyclesLeft = 0;
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		lastZetCPUContext->nCyclesTotal -= lastZetCPUContext->nCyclesLeft;
		lastZetCPUContext->nCyclesSegment -= lastZetCPUContext->nCyclesLeft;
		lastZetCPUContext->nCyclesLeft = 0;
	}
#endif

}

// This function will make an area callback ZetRead/ZetWrite
int ZetMemCallback(int nStart, int nEnd, int nMode)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
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
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		unsigned char cStart = (nStart >> 8);
		unsigned char **pMemMap = lastZetCPUContextZ80->pZetMemMap;

		for (unsigned short i = cStart; i <= (nEnd >> 8); i++) {
			switch (nMode) {
				case 0:
					pMemMap[0     + i] = NULL;
					break;
				case 1:
					pMemMap[0x100 + i] = NULL;
					break;
				case 2:
					pMemMap[0x200 + i] = NULL;
					//pMemMap[0x300 + i] = NULL;
					break;
			}
		}
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		nStart >>= CZ80_FETCH_SFT;
		nEnd += CZ80_FETCH_BANK - 1;
		nEnd >>= CZ80_FETCH_SFT;

		// Leave the section out of the memory map, so the callback with be used
		for (int i = nStart; i < nEnd; i++) {
			switch (nMode) {
				case 0:
					lastZetCPUContext->Read[i] = NULL;
					break;
				case 1:
					lastZetCPUContext->Write[i] = NULL;
					break;
				case 2:
					lastZetCPUContext->Fetch[i] = NULL;
					break;
			}
		}
	}
#endif
	return 0;
}

int ZetMemEnd()
{
	return 0;
}

void ZetExit()
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		for (int i = 0; i < nCPUCount; i++) {
			free(ZetCPUContextDZ80[i].ppMemFetch);
			ZetCPUContextDZ80[i].ppMemFetch = NULL;
			free(ZetCPUContextDZ80[i].ppMemFetchData);
			ZetCPUContextDZ80[i].ppMemFetchData = NULL;
			free(ZetCPUContextDZ80[i].ppMemRead);
			ZetCPUContextDZ80[i].ppMemRead = NULL;
			free(ZetCPUContextDZ80[i].ppMemWrite);
			ZetCPUContextDZ80[i].ppMemWrite = NULL;
		}

		free(ZetCPUContextDZ80);
		ZetCPUContextDZ80 = NULL;
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		Z80Exit();
		free( ZetCPUContextZ80 );
		ZetCPUContextZ80 = NULL;
		lastZetCPUContextZ80 = NULL;
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		free( ZetCPUContext );
		ZetCPUContext = NULL;
		lastZetCPUContext = NULL;
	}
#endif

	nCPUCount = 0;
	nHasZet = -1;
}


int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
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
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		unsigned char cStart = (nStart >> 8);
		unsigned char **pMemMap = lastZetCPUContextZ80->pZetMemMap;

		for (unsigned short i = cStart; i <= (nEnd >> 8); i++) {
			switch (nMode) {
				case 0: {
					pMemMap[0     + i] = Mem + ((i - cStart) << 8);
					break;
				}
			
				case 1: {
					pMemMap[0x100 + i] = Mem + ((i - cStart) << 8);
					break;
				}
				
				case 2: {
					pMemMap[0x200 + i] = Mem + ((i - cStart) << 8);
					pMemMap[0x300 + i] = Mem + ((i - cStart) << 8);
					break;
				}
			}
		}
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		int s = nStart >> CZ80_FETCH_SFT;
		int e = (nEnd + CZ80_FETCH_BANK - 1) >> CZ80_FETCH_SFT;

		// Put this section in the memory map, giving the offset from Z80 memory to PC memory
		for (int i = s; i < e; i++) {
			switch (nMode) {
				case 0:
					lastZetCPUContext->Read[i] = Mem - nStart;
					break;
				case 1:
					lastZetCPUContext->Write[i] = Mem - nStart;
					break;
				case 2:
					lastZetCPUContext->Fetch[i] = Mem - nStart;
					lastZetCPUContext->FetchData[i] = Mem - nStart;
					break;
			}
		}
	/*
		if (nMode == 2) {
			s = lastZetCPUContext->PC - lastZetCPUContext->BasePC;
			e = s >> CZ80_FETCH_SFT;
			lastZetCPUContext->BasePC = (u32) lastZetCPUContext->Fetch[e];
			lastZetCPUContext->BasePCData = (u32) lastZetCPUContext->FetchData[e];
			lastZetCPUContext->PC = s + lastZetCPUContext->BasePC;
			lastZetCPUContext->PCData = s + lastZetCPUContext->BasePCData;
		}
	*/
	}
#endif
	return 0;
}

int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem01, unsigned char *Mem02)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
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
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		unsigned char cStart = (nStart >> 8);
		unsigned char **pMemMap = lastZetCPUContextZ80->pZetMemMap;
		
		if (nMode != 2) {
			return 1;
		}
		
		for (unsigned short i = cStart; i <= (nEnd >> 8); i++) {
			pMemMap[0x200 + i] = Mem01 + ((i - cStart) << 8);
			pMemMap[0x300 + i] = Mem02 + ((i - cStart) << 8);
		}
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		int s = nStart >> CZ80_FETCH_SFT;
		int e = (nEnd + CZ80_FETCH_BANK - 1) >> CZ80_FETCH_SFT;

		if (nMode != 2) {
			return 1;
		}

		// Put this section in the memory map, giving the offset from Z80 memory to PC memory
		for (int i = s; i < e; i++) {
			lastZetCPUContext->Fetch[i] = Mem01 - nStart;
			lastZetCPUContext->FetchData[i] = Mem02 - nStart;
		}
	/*	
		s = lastZetCPUContext->PC - lastZetCPUContext->BasePC;
		e = s >> CZ80_FETCH_SFT;
		lastZetCPUContext->BasePC = (u32) lastZetCPUContext->Fetch[e];
		lastZetCPUContext->BasePCData = (u32) lastZetCPUContext->FetchData[e];
		lastZetCPUContext->PC = s + lastZetCPUContext->BasePC;
		lastZetCPUContext->PCData = s + lastZetCPUContext->BasePCData;
	*/	
	}
#endif
	return 0;
}

int ZetReset()
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) DozeReset();
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) Z80Reset();
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) Cz80_Reset( lastZetCPUContext );
#endif
	return 0;
}

int ZetPc(int n)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		if (n < 0) {
			return Doze.pc;
		} else {
			return ZetCPUContextDZ80[n].pc;
		}
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		if (n < 0) {
			return lastZetCPUContextZ80->reg.pc.w.l;
		} else {
			return ZetCPUContextZ80[n].reg.pc.w.l;
		}
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		if (n < 0) {
			return Cz80_Get_PC(lastZetCPUContext);
		} else {
			return Cz80_Get_PC(&ZetCPUContext[n]);
		}
	}
#endif
	return 0;
}

int ZetBc(int n)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		if (n < 0) {
			return Doze.bc;
		} else {
			return ZetCPUContextDZ80[n].bc;
		}
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		if (n < 0) {
			return lastZetCPUContextZ80->reg.bc.w.l;
		} else {
			return ZetCPUContextZ80[n].reg.bc.w.l;
		}
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		if (n < 0) {
			return Cz80_Get_BC(lastZetCPUContext);
		} else {
			return Cz80_Get_BC(&ZetCPUContext[n]);
		}
	}
#endif
}

int ZetDe(int n)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		if (n < 0) {
			return Doze.de;
		} else {
			return ZetCPUContextDZ80[n].de;
		}
	}
#endif

// correct?

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		if (n < 0) {
			return lastZetCPUContextZ80->reg.de.w.l;
		} else {
			return ZetCPUContextZ80[n].reg.de.w.l;
		}
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		if (n < 0) {
			return Cz80_Get_DE(lastZetCPUContext);
		} else {
			return Cz80_Get_DE(&ZetCPUContext[n]);
		}
	}
#endif
}

int ZetHL(int n)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		if (n < 0) {
			return Doze.hl;
		} else {
			return ZetCPUContextDZ80[n].hl;
		}
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		if (n < 0) {
			return lastZetCPUContextZ80->reg.hl.w.l;
		} else {
			return ZetCPUContextZ80[n].reg.hl.w.l;
		}
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		if (n < 0) {
			return Cz80_Get_HL(lastZetCPUContext);
		} else {
			return Cz80_Get_HL(&ZetCPUContext[n]);
		}
	}
#endif
}

int ZetScan(int nAction)
{
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 0;
	}

#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		char szText[] = "Z80 #0";

		for (int i = 0; i < nCPUCount; i++) {
			szText[5] = '1' + i;

			ScanVar(&ZetCPUContextDZ80[i], 32 + 16, szText);
		}
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		char szText[] = "Z80 #0";

		for (int i = 0; i < nCPUCount; i++) {
			szText[5] = '1' + i;

			ScanVar(&ZetCPUContextZ80[i], sizeof(Z80_Regs), szText);
		}
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		char szText[] = "Z80 #0";

		for (int i = 0; i < nCPUCount; i++) {
			szText[5] = '1' + i;

			ZetCPUContext[i].PC -= ZetCPUContext[i].BasePC;
			ZetCPUContext[i].BasePC = 0;
			ScanVar(&ZetCPUContext[i], (unsigned int)&(ZetCPUContext[i].Fetch)-(unsigned int)&ZetCPUContext[i], szText);
			Cz80_Set_PC(&ZetCPUContext[i], ZetCPUContext[i].PC);
		}
	}
#endif
	return 0;
}

void ZetSetIRQLine(const int line, const int status)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) Doze.nInterruptLatch = line | status;
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		if(status == ZET_IRQSTATUS_NONE ) {
			Z80SetIrqLine(0, 0);
		} else if(status == ZET_IRQSTATUS_ACK) {
			Z80SetIrqLine(line, 1);
		} else if(status == ZET_IRQSTATUS_AUTO) {
			Z80SetIrqLine(line, 1);
			Z80Execute(0);
			Z80SetIrqLine(0, 0);
		}
	/*	switch ( status ) {
		case ZET_IRQSTATUS_NONE:
			Z80SetIrqLine(0, 0);
			break;
		case ZET_IRQSTATUS_ACK: 	
			Z80SetIrqLine(line, 1);
			break;
		case ZET_IRQSTATUS_AUTO:
			Z80SetIrqLine(line, 1);
			Z80Execute(0);
			Z80SetIrqLine(0, 0);
			break;
		}*/
	}
#endif


#ifdef EMU_CZ80
	if(nZetCpuCore == 0) lastZetCPUContext->nInterruptLatch = line | status;
#endif
}

void ZetRaiseIrq(int n)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) ZetSetIRQLine(n, ZET_IRQSTATUS_AUTO);
#endif
#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) ZetSetIRQLine(n, Z80_ASSERT_LINE);
#endif
#ifdef EMU_CZ80
	if(nZetCpuCore == 0) ZetSetIRQLine(n, CZ80_IRQSTATUS_AUTO);
#endif
}

void ZetLowerIrq()
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
#endif
#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) ZetSetIRQLine(0, Z80_CLEAR_LINE);
#endif
#ifdef EMU_CZ80
	if(nZetCpuCore == 0) ZetSetIRQLine(0, CZ80_IRQSTATUS_NONE);
#endif
}

int ZetNmi()
{
	int nCycles;
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) {
		nCycles = DozeNmi();
		// Taking an NMI requires 12 cycles
		Doze.nCyclesTotal += nCycles;
	}
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) {
		Z80SetIrqLine(Z80_INPUT_LINE_NMI, 1);
		Z80Execute(0);
		Z80SetIrqLine(Z80_INPUT_LINE_NMI, 0);
		//Z80Execute(0);
		nCycles = 12;
		lastZetCPUContextZ80->nCyclesTotal += nCycles;
	}
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) {
		nCycles = Cz80_Set_NMI(lastZetCPUContext);
		lastZetCPUContext->nCyclesTotal += nCycles;
	}
#endif

	return nCycles;
}

int ZetIdle(int nCycles)
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) Doze.nCyclesTotal += nCycles;
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) lastZetCPUContextZ80->nCyclesTotal += nCycles;
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) lastZetCPUContext->nCyclesTotal += nCycles;
#endif

	return nCycles;
}

int ZetSegmentCycles()
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) return Doze.nCyclesSegment - Doze.nCyclesLeft;
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) return lastZetCPUContextZ80->nCyclesSegment - lastZetCPUContextZ80->nCyclesLeft;
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) return lastZetCPUContext->nCyclesSegment - lastZetCPUContext->nCyclesLeft;
#endif

}

int ZetTotalCycles()
{
#ifdef EMU_DOZE
	if(nZetCpuCore == 2) return Doze.nCyclesTotal - Doze.nCyclesLeft;
#endif

#ifdef EMU_MAME_Z80
	if(nZetCpuCore == 1) return lastZetCPUContextZ80->nCyclesTotal - lastZetCPUContextZ80->nCyclesLeft;
#endif

#ifdef EMU_CZ80
	if(nZetCpuCore == 0) return lastZetCPUContext->nCyclesTotal - lastZetCPUContext->nCyclesLeft;
#endif
}


