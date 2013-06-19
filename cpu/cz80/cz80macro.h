/********************************************************************************/
/*                                                                              */
/* CZ80 macro file                                                              */
/* C Z80 emulator version 0.9                                                   */
/* Copyright 2004-2005 Stéphane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

#if CZ80_USE_JUMPTABLE
#define _SSOP(A,B) A##B
#define OP(A) _SSOP(OP,A)
#define OPCB(A) _SSOP(OPCB,A)
#define OPED(A) _SSOP(OPED,A)
#define OPXY(A) _SSOP(OPXY,A)
#define OPXYCB(A) _SSOP(OPXYCB,A)
#else
#define OP(A) case A
#define OPCB(A) case A
#define OPED(A) case A
#define OPXY(A) case A
#define OPXYCB(A) case A
#endif

#define GET_BYTE                \
	(*(u8*)PCData)

#define GET_BYTE_S              \
	(*(s8*)PCData)

#if CZ80_LITTLE_ENDIAN
#define GET_WORD                \
	((*(u8*)(PCData + 0)) | ((*(u8*)(PCData + 1)) << 8))
#else
#define GET_WORD                \
    ((*(u8*)(PCData + 1)) | ((*(u8*)(PCData + 0)) << 8))
#endif

#define FETCH_BYTE				\
	(*(u8*)PC++)

#define FETCH_BYTED				\
	(*(u8*)PCData++)

#define FETCH_BYTE_S			\
	(*(s8*)PCData++)

#define FETCH_WORD(A)			\
	A = GET_WORD;				\
	PC += 2;					\
	PCData += 2;

#if CZ80_SIZE_OPT
    #define RET(A)              \
        CCnt -= A;              \
        goto Cz80_Exec_Check;
#else
    #define RET(A)              \
        if ((CCnt -= A) <= 0) goto Cz80_Exec_End;  \
        goto Cz80_Exec;
#endif

#define SET_PC(A)               \
	CPU->BasePC = (u32) CPU->Fetch[(A) >> CZ80_FETCH_SFT];  \
	CPU->BasePCData = (u32) CPU->FetchData[(A) >> CZ80_FETCH_SFT];  \
	PC = (A) + CPU->BasePC;		\
	PCData = (A) + CPU->BasePCData;

/*
#define PRE_IO                  \
	{							\
		CPU->CycleIO = CCnt;	\
		CPU->PC = PC;			\
		CPU->PCData = PCData;	\
	}

#define POST_IO                 \
	{							\
		CCnt = CPU->CycleIO;	\
		PC = CPU->PC;			\
		PCData = CPU->PCData;	\
	}
*/

#define READ_BYTE(A, D)                 \
	{									\
		if (CPU->Read[(A) >> CZ80_FETCH_SFT])	\
			D = (CPU->Read[(A) >> CZ80_FETCH_SFT])[A];	\
		else {							\
			CPU->CycleIO = CCnt;		\
			CPU->PC = PC;				\
			CPU->PCData = PCData;		\
			D = CPU->Read_Byte(A);		\
			CCnt = CPU->CycleIO;		\
			PC = CPU->PC;				\
			PCData = CPU->PCData;		\
		}								\
	}

#if CZ80_USE_WORD_HANDLER
#if CZ80_LITTLE_ENDIAN
#define READ_WORD(A, D)                 \
	{									\
		if (CPU->Read[(A) >> CZ80_FETCH_SFT])	\
			D = (u32)(CPU->Read[(A) >> CZ80_FETCH_SFT])[A] | ((u32)((CPU->Read[(A) >> CZ80_FETCH_SFT])[(A) + 1]) << 8);	\
		else {							\
			CPU->CycleIO = CCnt;		\
			CPU->PC = PC;				\
			CPU->PCData = PCData;		\
			D = CPU->Read_Word(A);		\
			CCnt = CPU->CycleIO;		\
			PC = CPU->PC;				\
			PCData = CPU->PCData;		\
		}								\
	}
#else
#define READ_WORD(A, D)                 \
	{									\
		if (CPU->Read[(A) >> CZ80_FETCH_SFT])	\
			D = ((u32)((CPU->Read[(A) >> CZ80_FETCH_SFT])[A]) << 8) | (u32)(CPU->Read[(A) >> CZ80_FETCH_SFT])[(A) + 1];	\
		else {							\
			CPU->CycleIO = CCnt;		\
			CPU->PC = PC;				\
			CPU->PCData = PCData;		\
			D = CPU->Read_Word(A);		\
			CCnt = CPU->CycleIO;		\
			PC = CPU->PC;				\
			PCData = CPU->PCData;		\
		}								\
	}
#endif
#elif CZ80_LITTLE_ENDIAN
#define READ_WORD(A, D)                 \
	{									\
		if (CPU->Read[(A) >> CZ80_FETCH_SFT])	\
			D = (u32)(CPU->Read[(A) >> CZ80_FETCH_SFT])[A] | ((u32)((CPU->Read[(A) >> CZ80_FETCH_SFT])[(A) + 1]) << 8);	\
		else {							\
			CPU->CycleIO = CCnt;		\
			CPU->PC = PC;				\
			CPU->PCData = PCData;		\
			D = CPU->Read_Byte(A) | (CPU->Read_Byte((A) + 1) << 8);	\
			CCnt = CPU->CycleIO;		\
			PC = CPU->PC;				\
			PCData = CPU->PCData;		\
		}								\
	}
#else
#define READ_WORD(A, D)                 \
	{									\
		if (CPU->Read[(A) >> CZ80_FETCH_SFT])	\
			D = ((u32)((CPU->Read[(A) >> CZ80_FETCH_SFT])[A]) << 8) | (u32)(CPU->Read[(A) >> CZ80_FETCH_SFT])[(A) + 1];	\
		else {							\
			CPU->CycleIO = CCnt;		\
			CPU->PC = PC;				\
			CPU->PCData = PCData;		\
			D = (CPU->Read_Byte(A) << 8) | CPU->Read_Byte((A) + 1);	\
			CCnt = CPU->CycleIO;		\
			PC = CPU->PC;				\
			PCData = CPU->PCData;		\
		}								\
	}
#endif

#define READSX_BYTE(A, D)               \
	{									\
		if (CPU->Read[(A) >> CZ80_FETCH_SFT])	\
			D = (s32)(s8)((CPU->Read[(A) >> CZ80_FETCH_SFT])[A]);	\
		else {							\
			CPU->CycleIO = CCnt;		\
			CPU->PC = PC;				\
			CPU->PCData = PCData;		\
			D = (s32)(s8)CPU->Read_Byte(A);	\
			CCnt = CPU->CycleIO;		\
			PC = CPU->PC;				\
			PCData = CPU->PCData;		\
		}								\
	}

#define WRITE_BYTE(A, D)                \
	{									\
		if (CPU->Write[(A) >> CZ80_FETCH_SFT])	\
			(CPU->Write[(A) >> CZ80_FETCH_SFT])[A] = D;	\
		else {							\
			CPU->CycleIO = CCnt;		\
			CPU->PC = PC;				\
			CPU->PCData = PCData;		\
			CPU->Write_Byte(A, D);		\
			CCnt = CPU->CycleIO;		\
			PC = CPU->PC;				\
			PCData = CPU->PCData;		\
		}								\
	}

#if CZ80_USE_WORD_HANDLER
#if CZ80_LITTLE_ENDIAN
#define WRITE_WORD(A, D)                \
	{									\
		if (CPU->Write[(A) >> CZ80_FETCH_SFT]) {	\
			(CPU->Write[(A) >> CZ80_FETCH_SFT])[A] = (D);	\
			(CPU->Write[(A) >> CZ80_FETCH_SFT])[(A) + 1] = (D) >> 8;	\
		} else {						\
			CPU->CycleIO = CCnt;		\
			CPU->PC = PC;				\
			CPU->PCData = PCData;		\
			CPU->Write_Word(A, D);		\
			CCnt = CPU->CycleIO;		\
			PC = CPU->PC;				\
			PCData = CPU->PCData;		\
		}								\
	}
#else
#define WRITE_WORD(A, D)                \
	{									\
		if (CPU->Write[(A) >> CZ80_FETCH_SFT]) {	\
			(CPU->Write[(A) >> CZ80_FETCH_SFT])[A] = (D) >> 8;	\
			(CPU->Write[(A) >> CZ80_FETCH_SFT])[(A) + 1] = (D);	\
		} else {						\
			CPU->CycleIO = CCnt;		\
			CPU->PC = PC;				\
			CPU->PCData = PCData;		\
			CPU->Write_Word(A, D);		\
			CCnt = CPU->CycleIO;		\
			PC = CPU->PC;				\
			PCData = CPU->PCData;		\
		}								\
	}
#endif
#elif CZ80_LITTLE_ENDIAN
#define WRITE_WORD(A, D)                \
	{									\
		if (CPU->Write[(A) >> CZ80_FETCH_SFT]) {	\
			(CPU->Write[(A) >> CZ80_FETCH_SFT])[A] = (D);	\
			(CPU->Write[(A) >> CZ80_FETCH_SFT])[(A) + 1] = (D) >> 8;	\
		} else {						\
			CPU->CycleIO = CCnt;		\
			CPU->PC = PC;				\
			CPU->PCData = PCData;		\
			CPU->Write_Byte(A, D);		\
			CPU->Write_Byte((A) + 1, (D) >> 8);	\
			CCnt = CPU->CycleIO;		\
			PC = CPU->PC;				\
			PCData = CPU->PCData;		\
		}								\
	}
#else
#define WRITE_WORD(A, D)                \
	{									\
		if (CPU->Write[(A) >> CZ80_FETCH_SFT]) {	\
			(CPU->Write[(A) >> CZ80_FETCH_SFT])[A] = (D) >> 8;	\
			(CPU->Write[(A) >> CZ80_FETCH_SFT])[(A) + 1] = (D);	\
		} else {						\
			CPU->CycleIO = CCnt;		\
			CPU->PC = PC;				\
			CPU->PCData = PCData;		\
			CPU->Write_Byte(A, (D) >> 8);	\
			CPU->Write_Byte((A) + 1, D);	\
			CCnt = CPU->CycleIO;		\
			PC = CPU->PC;				\
			PCData = CPU->PCData;		\
		}								\
	}
#endif

#define PUSH_16(A)              \
    {                           \
        u32 sp;                 \
                                \
        zSP -= 2;               \
        sp = zSP;               \
        WRITE_WORD(sp, A);      \
    }

#define POP_16(A)           \
    {                       \
        u32 sp;             \
                            \
        sp = zSP;           \
        READ_WORD(sp, A)    \
        zSP = sp + 2;       \
    }

#define IN(A, D)            \
    D = CPU->IN_Port(A);


#define OUT(A, D)           \
    CPU->OUT_Port(A, D);

#define CHECK_INT                                           \
    if (CPU->Status & (zIFF1 | CZ80_HAS_NMI))               \
    {                                                       \
        u32 newPC;                                          \
                                                            \
        if (CPU->Status & CZ80_HAS_NMI)                     \
        {                                                   \
            /* NMI */                                       \
            CPU->Status &= ~(CZ80_HALTED | CZ80_HAS_NMI);   \
            zIFF1 = 0;                                      \
            newPC = 0x66;                                   \
        }                                                   \
        else                                                \
        {                                                   \
            /* MI */                                        \
	        if (CPU->InterruptLatch & CZ80_IRQSTATUS_AUTO)  \
	        {                                               \
	            CPU->Status &= ~(CZ80_HALTED | CZ80_HAS_INT);   \
	            CPU->InterruptLatch = CZ80_IRQSTATUS_NONE;	\
	        }                                               \
	        else                                            \
	        {                                               \
	            CPU->Status &= ~(CZ80_HALTED);				\
	        }                                               \
            zIFF= 0;                                        \
                                                            \
            if (zIM == 1) newPC = 0x38;                     \
            else                                            \
            {                                               \
                u32 adr;                                    \
                                                                    \
                /*Opcode = CPU->Interrupt_Ack(CPU->IntVect) & 0xFF;*/   \
                if (zIM == 0) goto Cz80_Exec;                       \
                                                            \
                adr = CPU->IntVect | (zI << 8);             \
                /*adr = Opcode | (zI << 8);*/                   \
                READ_WORD(adr, newPC)                       \
                CCnt -= 8;                                  \
            }                                               \
        }                                                   \
                                                            \
        {                                                   \
            u32 src = zRealPC;                              \
                                                            \
            PUSH_16(src)                                    \
            SET_PC(newPC)                                   \
            CCnt -= 11;                                     \
        }                                                   \
    }
