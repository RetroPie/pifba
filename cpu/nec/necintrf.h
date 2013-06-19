/* ASG 971222 -- rewrote this interface */
#ifndef __NEC_H_
#define __NEC_H_

typedef union
{                   /* eight general registers */
    unsigned short w[8];    /* viewed as 16 bits registers */
    unsigned char  b[16];   /* or as 8 bit registers */
} necbasicregs;

typedef struct
{
	necbasicregs regs;
 	unsigned short	sregs[4];

	unsigned short	ip;

	signed int	SignVal;
    unsigned int  AuxVal, OverVal, ZeroVal, CarryVal, ParityVal; /* 0 or non-0 valued flags */
	unsigned char	TF, IF, DF, MF; 	/* 0 or 1 valued flags */	/* OB[19.07.99] added Mode Flag V30 */
	unsigned int	int_vector;
	unsigned int	pending_irq;
	unsigned int	nmi_state;
	unsigned int	irq_state;
	int     (*irq_callback)(int irqline);
	
	unsigned int cpu_type;
	unsigned int prefix_base;
	signed char seg_prefix;
	int no_interrupt;
	int nec_ICount;

} nec_Regs;

enum {
	NEC_IP=1, NEC_AW, NEC_CW, NEC_DW, NEC_BW, NEC_SP, NEC_BP, NEC_IX, NEC_IY,
	NEC_FLAGS, NEC_ES, NEC_CS, NEC_SS, NEC_DS,
	NEC_VECTOR, NEC_PENDING, NEC_NMI_STATE, NEC_IRQ_STATE };

/* Public functions */

/*
#define v20_ICount nec_ICount
extern void v20_init(void);
extern void v20_reset(void *param);
extern void v20_exit(void);
extern int v20_execute(int cycles);
extern unsigned v20_get_context(void *dst);
extern void v20_set_context(void *src);
extern unsigned v20_get_reg(int regnum);
extern void v20_set_reg(int regnum, unsigned val);
extern void v20_set_irq_line(int irqline, int state);
extern void v20_set_irq_callback(int (*callback)(int irqline));
extern const char *v20_info(void *context, int regnum);
extern unsigned v20_dasm(char *buffer, unsigned pc);

#define v30_ICount nec_ICount
extern void v30_init(void);
extern void v30_reset(void *param);
extern void v30_exit(void);
extern int v30_execute(int cycles);
extern unsigned v30_get_context(void *dst);
extern void v30_set_context(void *src);
extern unsigned v30_get_reg(int regnum);
extern void v30_set_reg(int regnum, unsigned val);
extern void v30_set_irq_line(int irqline, int state);
extern void v30_set_irq_callback(int (*callback)(int irqline));
extern const char *v30_info(void *context, int regnum);
extern unsigned v30_dasm(char *buffer, unsigned pc);

#define v33_ICount nec_ICount
extern void v33_init(void);
extern void v33_reset(void *param);
extern void v33_exit(void);
extern int v33_execute(int cycles);
extern unsigned v33_get_context(void *dst);
extern void v33_set_context(void *src);
extern unsigned v33_get_reg(int regnum);
extern void v33_set_reg(int regnum, unsigned val);
extern void v33_set_irq_line(int irqline, int state);
extern void v33_set_irq_callback(int (*callback)(int irqline));
extern const char *v33_info(void *context, int regnum);
extern unsigned v33_dasm(char *buffer, unsigned pc);
*/

void nec_set_reg(int,unsigned);
int nec_execute(nec_Regs *pCPU,int cycles);	
unsigned nec_get_reg(int regnum);
void nec_reset (nec_Regs *pCPU);
void nec_int(unsigned int wektor);

#endif
