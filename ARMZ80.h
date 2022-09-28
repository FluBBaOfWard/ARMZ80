#ifndef ARMZ80_HEADER
#define ARMZ80_HEADER

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	u32 z80MemTbl[64];
	u32 z80ReadTbl[8];
	u32 z80WriteTbl[8];

	u32 z80Regs[8];
	u32 z80Regs2[5];
	u32 z80IX;
	u32 z80IY;
	u8 z80I;
	u8 z80R;
	u8 z80IM;
	u8 z80Iff2;
	u8 z80IrqPin;
	u8 z80Padding0[1];
	u8 z80Iff1;
	u8 z80NmiPending;
	u8 z80NmiPin;
	u8 z80Out0;
	u8 z80Padding1[2];
	u32 z80LastBank;
	u32 z80OldCycles;
	u32 *z80NextTimeout_;
	u32 *z80NextTimeout;
	u32 *z80IMFunction;
	u32 *z80IrqVectorFunc;
	u32 *z80IrqAckFunc;
	u32 z80Opz[256];
	u8 z80PZST[256];

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
void Z80RedirectOpcode(int opcode, void *function);

void Z80SetIRQPin(bool set);
void Z80SetNMIPin(bool set);
void Z80RestoreAndRunXCycles(int cycles);
void Z80RunXCycles(int cycles);

#ifdef __cplusplus
}
#endif

#endif // ARMZ80_HEADER
