
				;@ r0,r1,r2=temp regs.
	z80f		.req r3			;@
	z80a		.req r4			;@ Bits 0-23=0
	z80bc		.req r5			;@ Bits 0-15=0
	z80de		.req r6			;@ Bits 0-15=0
	z80hl		.req r7			;@ Bits 0-15=0
	z80cyc		.req r8
	z80pc		.req r9
	z80ptr		.req r10
	z80sp		.req r11		;@ Bits 0-15=0
	z80xy		.req lr			;@ Pointer to IX or IY reg
	addy		.req r12		;@ Keep this at r12 (scratch for APCS)

	.struct -(104*4)			;@ Changes section so make sure it's set before real code.
z80MemTbl:			.space 64*4
z80ReadTbl:			.space 8*4
z80WriteTbl:		.space 8*4
z80StateStart:
z80Regs:			.space 8*4
z80Regs2:			.space 5*4
z80IX:				.long 0
z80IY:				.long 0
z80I:				.byte 0
z80R:				.byte 0
z80IM:				.byte 0
z80Iff2:			.byte 0

z80IrqPin:			.byte 0
z80Iff1:			.byte 0
z80NmiPending:		.byte 0
z80ResetPin:		.space 1

z80NmiPin:			.byte 0
z80Out0:			.byte 0
z80Padding1:		.space 2
z80StateEnd:
z80LastBank:		.long 0
z80OldCycles:		.long 0		;@ Normal cycles (without flags)
z80NextTimeout:		.long 0
z80IMFunction:		.long 0
z80IrqVectorFunc:	.long 0
z80IrqAckFunc:		.long 0
z80Opz:				.space 256*4
z80PZST:			.space 256
z80Size:

;@----------------------------------------------------------------------------
