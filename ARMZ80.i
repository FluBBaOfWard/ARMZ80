#if !__ASSEMBLER__
	#error This header file is only for use in assembly files!
#endif

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

;@----------------------------------------------------------------------------
							;@ ARM flags
	.equ PSR_S, 0x00000008		;@ Sign (negative)
	.equ PSR_Z, 0x00000004		;@ Zero
	.equ PSR_C, 0x00000002		;@ Carry
	.equ PSR_V, 0x00000001		;@ Overflow/Parity
	.equ PSR_P, 0x00000001		;@ Overflow/Parity

	.equ PSR_n, 0x00000080		;@ Was the last opcode add or sub?
	.equ PSR_X, 0x00000040		;@ z80_X (unused)
	.equ PSR_Y, 0x00000020		;@ z80_Y (unused)
	.equ PSR_H, 0x00000010		;@ Half carry

							;@ Z80 flags
	.equ SF, 0x80				;@ Sign (negative)
	.equ ZF, 0x40				;@ Zero
	.equ YF, 0x20				;@ z80_Y (unused)
	.equ HF, 0x10				;@ Half carry
	.equ XF, 0x08				;@ z80_X (unused)
	.equ PF, 0x04				;@ Overflow/Parity
	.equ VF, 0x04				;@ Overflow/Parity
	.equ NF, 0x02				;@ Was the last opcode add or sub?
	.equ CF, 0x01				;@ Carry

;@----------------------------------------------------------------------------
	.equ CYC_SHIFT, 8
	.equ CYCLE, 1<<CYC_SHIFT	;@ One cycle
	.equ CYC_MASK, CYCLE-1		;@ Mask
;@----------------------------------------------------------------------------
	.struct -(102*4)			;@ Changes section so make sure it's set before real code.
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
z80ResetPin:		.byte 0

z80NmiPin:			.byte 0
z80Out0:			.byte 0
z80Padding1:		.space 2
z80StateEnd:
z80LastBank:		.long 0
z80IMFunction:		.long 0
z80IrqVectorFunc:	.long 0
z80IrqAckFunc:		.long 0
z80Opz:				.space 256*4
z80PZST:			.space 256
z80Size:

;@----------------------------------------------------------------------------
