#ifndef ARMZ80_HEADER
#define ARMZ80_HEADER

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	u8 *memTbl[64];
	u8 (*readTbl[8])(void);
	void (*writeTbl[8])(u8);

	u32 regs[8];
	u32 regs2[5];
	u32 IX;
	u32 IY;
	u8 I;
	u8 R;
	u8 IM;
	u8 Iff2;
	u8 IrqPin;
	u8 Padding0[1];
	u8 Iff1;
	u8 NmiPending;
	u8 NmiPin;
	u8 Out0;
	u8 Padding1[2];
	u8 *LastBank;
	void (*IMFunction)(void);
	u8 (*IrqVectorFunc)(void);
	void (*IrqAckFunc)(void);
	void (*Opz[256])(void);
	u8 PZST[256];

} ARMZ80Core;

extern ARMZ80Core Z80OpTable;

void Z80Reset(ARMZ80Core *cpu, int type);

/**
 * Saves the state of the cpu to the destination.
 * @param  *destination: Where to save the state.
 * @param  *cpu: The ARMZ80Core cpu to save.
 * @return The size of the state.
 */
int Z80SaveState(void *destination, const ARMZ80Core *cpu);

/**
 * Loads the state of the cpu from the source.
 * @param  *cpu: The ARMZ80Core cpu to load a state into.
 * @param  *source: Where to load the state from.
 * @return The size of the state.
 */
int Z80LoadState(ARMZ80Core *cpu, const void *source);

/**
 * Gets the state size of an ARMZ80Core state.
 * @return The size of the state.
 */
int Z80GetStateSize(void);

/**
 * Redirect/patch an opcode to a new function.
 * @param  opcode: Which opcode to redirect.
 * @param  *function: Pointer to new function .
 */
void Z80RedirectOpcode(int opcode, void (*function)(void));

void Z80SetIRQPin(bool state, ARMZ80Core *cpu);
void Z80SetNMIPin(bool state, ARMZ80Core *cpu);
void Z80SetResetPin(bool state, ARMZ80Core *cpu);

void Z80RestoreAndRunXCycles(int cycles);
void Z80RunXCycles(int cycles);

#ifdef __cplusplus
}
#endif

#endif // ARMZ80_HEADER
