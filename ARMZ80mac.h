
#include "ARMZ80.i"
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
#ifdef Z80_LARGE_MAP
	.equ MEM_BANK_MASK, 0xFC00	;@ Granularity of bank switching
	.equ MEM_BANK_SHIFT, 8		;@ Shift of adr to get bank
#else
	.equ MEM_BANK_MASK, 0xE000	;@ Granularity of bank switching
	.equ MEM_BANK_SHIFT, 11		;@ Shift of adr to get bank
#endif


	.macro getPzsTbl reg
	add \reg,z80optbl,#z80PZST
	.endm

	.macro loadLastBank reg
	ldr \reg,[z80optbl,#z80LastBank]
	.endm

	.macro storeLastBank reg
	str \reg,[z80optbl,#z80LastBank]
	.endm

	.macro encodePC				;@ Translate z80pc from Z80 PC to rom offset
#ifdef Z80_FAST
	and r0,z80pc,#MEM_BANK_MASK
	add r2,z80optbl,#z80MemTbl
	ldr r0,[r2,r0,lsr#MEM_BANK_SHIFT]
	storeLastBank r0
	add z80pc,z80pc,r0
#else
	bl translateZ80PCToOffset	;@ In=z80pc, out=z80pc
#endif
	.endm

	.macro reEncodePC			;@ Translate z80pc from Z80 PC to rom offset
	loadLastBank r0
	sub z80pc,z80pc,r0
	encodePC
	.endm

	.macro encodeFLG			;@ Pack Z80 flags into r0
	and r0,z80f,#PSR_H|PSR_Y
	and r1,z80f,#PSR_S|PSR_Z
	orr r0,r0,r1,lsl#4
	movs r1,z80f,lsl#31
	orrmi r0,r0,#VF
	and r1,z80f,#PSR_n
	adc r0,r0,r1,lsr#6			;@ NF & CF
	tst z80f,#PSR_X
	orrne r0,r0,#XF
	.endm

	.macro decodeFLG			;@ Unpack Z80 flags from r0
	and z80f,r0,#HF|YF
	tst r0,#XF
	orrne z80f,z80f,#PSR_X
	tst r0,#CF
	orrne z80f,z80f,#PSR_C
	and r1,r0,#SF|ZF
	movs r0,r0,lsl#30
	adc z80f,z80f,r1,lsr#4		;@ Also sets V/P Flag.
	orrmi z80f,z80f,#PSR_n
	.endm


	.macro getNextOpcode
	ldrb r0,[z80pc],#1
	.endm

	.macro executeOpcode count
	subs cycles,cycles,#(\count)*CYCLE
	ldrpl pc,[z80optbl,r0,lsl#2]
	b outOfCycles
	.endm

/*
	.macro fetch count
	subs cycles,cycles,#(\count)*CYCLE
	b fetchDebug
	.endm
*/
	.macro fetch count
	getNextOpcode
	executeOpcode \count
	.endm
/*	.macro fetch count
	subs cycles,cycles,#(\count)*CYCLE
	ldrplb r0,[z80pc],#1
	ldrpl pc,[z80optbl,r0,lsl#2]
	ldr pc,[z80optbl,#z80NextTimeout]
	.endm
*/
	.macro fetchForce
	getNextOpcode
	ldr pc,[z80optbl,r0,lsl#2]
	.endm

	.macro eatCycles count
	sub cycles,cycles,#(\count)*CYCLE
	.endm

	.macro readMem8
#ifdef Z80_FAST
	and r1,addy,#0xE000
	add r2,z80optbl,#z80ReadTbl
	mov lr,pc
	ldr pc,[r2,r1,lsr#11]		;@ In: addy,r0=val(bits 8-31=?)
0:
#else
	bl memRead8
#endif
	.endm

	.macro readMem8BC
	mov addy,z80bc,lsr#16
	readMem8
	.endm

	.macro readMem8DE
	mov addy,z80de,lsr#16
	readMem8
	.endm

	.macro readMem8HL
#ifdef Z80_FAST
	mov addy,z80hl,lsr#16
	readMem8
#else
	bl memRead8HL
#endif
	.endm

	.macro readMem16 reg
	readMem8
	mov \reg,r0,lsl#16
	add addy,addy,#1
	readMem8
	orr \reg,\reg,r0,lsl#24
	.endm

	.macro writeMem8
#ifdef Z80_FAST
	and r1,addy,#0xE000
	add r2,z80optbl,#z80WriteTbl
	mov lr,pc
	ldr pc,[r2,r1,lsr#11]		;@ In: addy,r0=val(bits 8-31=?)
0:
#else
	bl memWrite8
#endif
	.endm

	.macro writeMem8e adr
#ifdef Z80_FAST
	and r1,addy,#0xE000
	add r2,z80optbl,#z80WriteTbl
	adr lr,\adr
	ldr pc,[r2,r1,lsr#11]		;@ In: addy,r0=val(bits 8-31=?)
#else
	adr lr,\adr
	b memWrite8
#endif
	.endm

	.macro writeMem8BC
	mov addy,z80bc,lsr#16
	writeMem8
	.endm

	.macro writeMem8DE
#ifdef Z80_FAST
	mov addy,z80de,lsr#16
	writeMem8
#else
	bl memWrite8DE
#endif
	.endm

	.macro writeMem8HL
	mov addy,z80hl,lsr#16
	writeMem8
	.endm

	.macro writeMem8HLe adr
	mov addy,z80hl,lsr#16
	writeMem8e \adr
	.endm

	.macro writeMem8HLminus adr
	mov addy,z80hl,lsr#16
	sub z80hl,z80hl,#0x00010000
	writeMem8e \adr
	.endm

	.macro writeMem8HLplus adr
	mov addy,z80hl,lsr#16
	add z80hl,z80hl,#0x00010000
	writeMem8e \adr
	.endm

	.macro writeMem16 reg
	mov r0,\reg,lsr#16
	writeMem8
	add addy,addy,#1
	mov r0,\reg,lsr#24
	writeMem8
	.endm

	.macro copyMem8HL_DE
	readMem8HL
	writeMem8DE
	.endm

	.macro calcIXd
	ldrsb r1,[z80pc],#1
	ldr addy,[z80xy]
	add addy,addy,r1,lsl#16
	mov addy,addy,lsr#16
	.endm
;@----------------------------------------------------------------------------

#ifdef Z80_FAST
	.macro push16				;@ Push r0
	sub z80sp,z80sp,#0x00020000
	and r1,z80sp,#MEM_BANK_MASK<<16
	add r2,z80optbl,#z80MemTbl
	ldr r2,[r2,r1,lsr#MEM_BANK_SHIFT+16]
	strb r0,[r2,z80sp,lsr#16]
	add r1,z80sp,#0x00010000
	mov r0,r0,lsr#8
	strb r0,[r2,r1,lsr#16]
	.endm						;@ r1,r2=?

	.macro push16Reg reg		;@ Push BC,DE,HL
	sub z80sp,z80sp,#0x00020000
	and r1,z80sp,#MEM_BANK_MASK<<16
	add r2,z80optbl,#z80MemTbl
	ldr r2,[r2,r1,lsr#MEM_BANK_SHIFT+16]
	mov r0,\reg,lsr#16
	strb r0,[r2,z80sp,lsr#16]
	add r1,z80sp,#0x00010000
	mov r0,\reg,lsr#24
	strb r0,[r2,r1,lsr#16]
	.endm						;@ r1,r2=?

	.macro pop16Reg reg			;@ Pop BC,DE,HL
	and r0,z80sp,#MEM_BANK_MASK<<16
	add r1,z80optbl,#z80MemTbl
	ldr r1,[r1,r0,lsr#MEM_BANK_SHIFT+16]
	ldrb \reg,[r1,z80sp,lsr#16]
	add z80sp,z80sp,#0x00010000
	ldrb r1,[r1,z80sp,lsr#16]
	add z80sp,z80sp,#0x00010000
	orr \reg,\reg,r1,lsl#8
	mov \reg,\reg,lsl#16
	.endm						;@ r0,r1=?

	.macro pop16AF				;@ Pop AF
	and r0,z80sp,#MEM_BANK_MASK<<16
	add r1,z80optbl,#z80MemTbl
	ldr r1,[r1,r0,lsr#MEM_BANK_SHIFT+16]
	ldrb r0,[r1,z80sp,lsr#16]
	add z80sp,z80sp,#0x00010000
	ldrb z80a,[r1,z80sp,lsr#16]
	add z80sp,z80sp,#0x00010000
	mov z80a,z80a,lsl#24
	.endm						;@ r0=flags,r1=?

	.macro pop16PC				;@ Pop PC
	and r0,z80sp,#MEM_BANK_MASK<<16
	add r1,z80optbl,#z80MemTbl
	ldr r1,[r1,r0,lsr#MEM_BANK_SHIFT+16]
	ldrb z80pc,[r1,z80sp,lsr#16]
	add z80sp,z80sp,#0x00010000
	ldrb r0,[r1,z80sp,lsr#16]
	add z80sp,z80sp,#0x00010000
	orr z80pc,z80pc,r0,lsl#8
	.endm						;@ r0,r1=?

	.macro pop16IX				;@ Pop IX
	and r0,z80sp,#MEM_BANK_MASK<<16
	add r1,z80optbl,#z80MemTbl
	ldr r1,[r1,r0,lsr#MEM_BANK_SHIFT+16]
	ldrb r0,[r1,z80sp,lsr#16]
	add z80sp,z80sp,#0x00010000
	ldrb r1,[r1,z80sp,lsr#16]
	add z80sp,z80sp,#0x00010000
	orr r0,r0,r1,lsl#8
	strh r0,[z80xy,#2]
	.endm						;@ r0,r1=?
#else // #ifdef Z80_FAST

	.macro push8
	sub z80sp,z80sp,#0x00010000
	mov addy,z80sp,lsr#16
	writeMem8
	.endm

	.macro pop8
	mov addy,z80sp,lsr#16
	readMem8
	add z80sp,z80sp,#0x00010000
	.endm

	.macro push16				;@ Push r0
	stmfd sp!,{r0}
	mov r0,r0,lsr#8
	push8
	ldmfd sp!,{r0}
	push8
	.endm						;@ r1,r2=?

	.macro push16Reg reg		;@ Push BC,DE,HL
	mov r0,\reg,lsr#24
	push8
	mov r0,\reg,lsr#16
	push8
	.endm						;@ r1,r2=?

	.macro pop16Reg reg			;@ Pop BC,DE,HL
	pop8
	mov \reg,r0,lsl#16
	pop8
	orr \reg,\reg,r0,lsl#24
	.endm						;@ r0,r1=?

	.macro pop16AF				;@ Pop AF
	pop8
	mov z80f,r0
	pop8
	mov z80a,r0,lsl#24
	mov r0,z80f
	.endm						;@ r0=flags,r1=?

	.macro pop16PC				;@ Pop PC
	pop8
	mov z80pc,r0
	pop8
	orr z80pc,z80pc,r0,lsl#8
	.endm						;@ r0,r1=?

	.macro pop16IX				;@ Pop IX
	stmfd sp!,{z80xy}
	pop8
	stmfd sp!,{r0}
	pop8
	ldmfd sp!,{r1,z80xy}
	orr r0,r1,r0,lsl#8
	strh r0,[z80xy,#2]
	.endm						;@ r0,r1=?
#endif // #ifdef Z80_FAST
;@----------------------------------------------------------------------------

	.macro opADC
	movs z80f,z80f,lsr#2		;@ Get C
	subcs r0,r0,#0x100
	eor z80f,r0,z80a,lsr#24		;@ Prepare for check of half carry
	adcs z80a,z80a,r0,ror#8
	mrs r0,cpsr					;@ S,Z,V&C
	eor z80f,z80f,z80a,lsr#24
	and z80f,z80f,#PSR_H		;@ H, correct
	orr z80f,z80f,r0,lsr#28
	.endm

	.macro opADCA
	movs z80f,z80f,lsr#2		;@ Get C
	orrcs z80a,z80a,#0x00800000
	adds z80a,z80a,z80a
	mrs z80f,cpsr				;@ S,Z,V&C
	mov z80f,z80f,lsr#28
	tst z80a,#0x10000000		;@ H, correct
	orrne z80f,z80f,#PSR_H
	fetch 4
	.endm

	.macro opADCH reg
	mov r0,\reg,lsr#24
	opADC
	fetch 4
	.endm

	.macro opADCL reg
	movs z80f,z80f,lsr#2		;@ Get C
	adc r0,\reg,\reg,lsr#15
	orrcs z80a,z80a,#0x00800000
	mov r1,z80a,lsl#4			;@ Prepare for check of half carry
	adds z80a,z80a,r0,lsl#23
	mrs z80f,cpsr				;@ S,Z,V&C
	mov z80f,z80f,lsr#28
	cmn r1,r0,lsl#27
	orrcs z80f,z80f,#PSR_H		;@ H, correct
	fetch 4
	.endm

	.macro opADCb
	opADC
	.endm
;@---------------------------------------

	.macro opADD reg s
	mov r1,z80a,lsl#4			;@ Prepare for check of half carry
	adds z80a,z80a,\reg,lsl#(\s)
	mrs z80f,cpsr				;@ S,Z,V&C
	mov z80f,z80f,lsr#28
	cmn r1,\reg,lsl#(\s)+4
	orrcs z80f,z80f,#PSR_H
	.endm

	.macro opADDA
	adds z80a,z80a,z80a
	mrs z80f,cpsr				;@ S,Z,V&C
	mov z80f,z80f,lsr#28
	tst z80a,#0x10000000		;@ H, correct
	orrne z80f,z80f,#PSR_H
	fetch 4
	.endm

	.macro opADDH reg
	and r0,\reg,#0xFF000000
	opADD r0,0
	fetch 4
	.endm

	.macro opADDL reg
	opADD \reg,8
	fetch 4
	.endm

	.macro opADDb 
	opADD r0,24
	.endm
;@---------------------------------------

	.macro opADC16 reg
	movs z80f,z80f,lsr#2		;@ Get C
	adc r0,z80a,\reg,lsr#15
	orrcs z80hl,z80hl,#0x00008000
	mov r1,z80hl,lsl#4
	adds z80hl,z80hl,r0,lsl#15
	mrs z80f,cpsr				;@ S,Z,V&C
	mov z80f,z80f,lsr#28
	cmn r1,r0,lsl#19
	orrcs z80f,z80f,#PSR_H
	fetch 15
	.endm

	.macro opADC16HL
	movs z80f,z80f,lsr#2		;@ Get C
	orrcs z80hl,z80hl,#0x00008000
	adds z80hl,z80hl,z80hl
	mrs z80f,cpsr				;@ S,Z,V&C
	mov z80f,z80f,lsr#28
	tst z80hl,#0x10000000		;@ H, correct.
	orrne z80f,z80f,#PSR_H
	fetch 15
	.endm

	.macro opADD16 reg1 reg2
	mov r1,\reg1,lsl#4			;@ Prepare for check of half carry
	adds \reg1,\reg1,\reg2
	bic z80f,z80f,#PSR_C+PSR_H+PSR_n
	orrcs z80f,z80f,#PSR_C
	cmn r1,\reg2,lsl#4
	orrcs z80f,z80f,#PSR_H
	.endm

	.macro opADD16_2 reg
	adds \reg,\reg,\reg
	bic z80f,z80f,#PSR_C+PSR_H+PSR_n
	orrcs z80f,z80f,#PSR_C
	tst \reg,#0x10000000		;@ H, correct.
	orrne z80f,z80f,#PSR_H
	.endm
;@---------------------------------------

	.macro opAND reg s
	and z80a,z80a,\reg,lsl#(\s)
	getPzsTbl r0
	ldrb z80f,[r0,z80a,lsr#24]	;@ Get PZS
	orr z80f,z80f,#PSR_H		;@ Set PSR_H
	.endm

	.macro opANDA
	getPzsTbl r0
	ldrb z80f,[r0,z80a,lsr#24]	;@ Get PZS
	orr z80f,z80f,#PSR_H		;@ Set PSR_H
	fetch 4
	.endm

	.macro opANDH reg
	opAND \reg,0
	fetch 4
	.endm

	.macro opANDL reg
	opAND \reg,8
	fetch 4
	.endm

	.macro opANDb
	opAND r0,24
	.endm
;@---------------------------------------

	.macro opBIT reg
	and z80f,z80f,#PSR_C			;@ Keep C
	orr z80f,z80f,#PSR_H			;@ Set H
	mov r0,r0,lsr#3
	tst \reg,r1,lsl r0				;@ r0 0x08-0x0F
	orreq z80f,z80f,#PSR_Z|PSR_P	;@ Z & P
	fetch 8
	.endm

	.macro opBITH reg
	mov r1,#0x00010000
	opBIT \reg
	.endm

	.macro opBITL reg
	mov r1,#0x00000100
	opBIT \reg
	.endm

	.macro opBIT7H reg
	and z80f,z80f,#PSR_C				;@ Keep C
	tst \reg,#0x80000000				;@ Bit 7
	orreq z80f,z80f,#PSR_H+PSR_Z+PSR_P	;@ H,Z & P
	orrne z80f,z80f,#PSR_H+PSR_S		;@ H & sign on "BIT 7,x"
	fetch 8
	.endm

	.macro opBIT7L reg
	and z80f,z80f,#PSR_C				;@ Keep C
	tst \reg,#0x00800000				;@ Bit 7
	orreq z80f,z80f,#PSR_H+PSR_Z+PSR_P	;@ H,Z & P
	orrne z80f,z80f,#PSR_H+PSR_S		;@ H & sign on "BIT 7,x"
	fetch 8
	.endm

	.macro opBITmem x
	readMem8
	and z80f,z80f,#PSR_C				;@ Keep C
	orr z80f,z80f,#PSR_H				;@ Set H
	tst r0,#1<<(\x)						;@ Bit x
	orreq z80f,z80f,#PSR_Z|PSR_P		;@ Z & P
	fetch 12
	.endm

	.macro opBIT7mem
	readMem8
	and z80f,z80f,#PSR_C				;@ Keep C
	tst r0,#0x80						;@ Bit x
	orreq z80f,z80f,#PSR_H+PSR_Z+PSR_P	;@ H,Z & P
	orrne z80f,z80f,#PSR_H+PSR_S		;@ H & sign on "BIT 7,x"
	fetch 12
	.endm
;@---------------------------------------

	.macro opCP reg s
	mov r1,z80a,lsl#4			;@ Prepare for check of half carry
	cmp z80a,\reg,lsl#(\s)
	mrs z80f,cpsr				;@ S,Z,V&C
	mov z80f,z80f,lsr#28
	eor z80f,z80f,#PSR_C|PSR_n	;@ Invert C and set n
	cmp r1,\reg,lsl#(\s)+4
	orrcc z80f,z80f,#PSR_H
	.endm

	.macro opCPA
	mov z80f,#PSR_Z|PSR_n		;@ Set Z & n
	fetch 4
	.endm

	.macro opCPH reg
	and r0,\reg,#0xFF000000
	opCP r0,0
	fetch 4
	.endm

	.macro opCPL reg
	opCP \reg,8
	fetch 4
	.endm

	.macro opCPb
	opCP r0,24
	.endm
;@---------------------------------------

	.macro opDEC8 reg								;@ For A and memory
	orr z80f,z80f,#PSR_n+PSR_H+PSR_S+PSR_V+PSR_Z	;@ Save carry & set n
	tst \reg,#0x0f000000
	bicne z80f,z80f,#PSR_H
	subs \reg,\reg,#0x01000000
	bicpl z80f,z80f,#PSR_S
	bicvc z80f,z80f,#PSR_V
	bicne z80f,z80f,#PSR_Z
	.endm

	.macro opDEC8H reg								;@ For B, D & H
	orr z80f,z80f,#PSR_n+PSR_H+PSR_S+PSR_V+PSR_Z	;@ Save carry & set n
	tst \reg,#0x0f000000
	bicne z80f,z80f,#PSR_H
	subs \reg,\reg,#0x01000000
	bicpl z80f,z80f,#PSR_S
	bicvc z80f,z80f,#PSR_V
	tst \reg,#0xff000000		;@ Z
	bicne z80f,z80f,#PSR_Z
	.endm

	.macro opDEC8L reg			;@ For C, E & L
	mov \reg,\reg,ror#24
	opDEC8H \reg
	mov \reg,\reg,ror#8
	.endm

	.macro opDEC8b				;@ For memory
	mov r0,r0,lsl#24
	opDEC8 r0
	mov r0,r0,lsr#24
	.endm

	.macro opDEC16 reg
	sub \reg,\reg,#0x00010000
	.endm
;@---------------------------------------

	.macro opINC8 reg
	and z80f,z80f,#PSR_C		;@ Save carry, clear n
	adds \reg,\reg,#0x01000000
	orrmi z80f,z80f,#PSR_S
	orrvs z80f,z80f,#PSR_V
	orrcs z80f,z80f,#PSR_Z		;@ When going from 0xFF to 0x00 carry is set.
	tst \reg,#0x0f000000		;@ h
	orreq z80f,z80f,#PSR_H
	.endm

	.macro opINC8H reg			;@ For B, D & H
	opINC8 \reg
	.endm

	.macro opINC8L reg			;@ For C, E & L
	mov \reg,\reg,ror#24
	opINC8 \reg
	mov \reg,\reg,ror#8
	.endm

	.macro opINC8b				;@ For memory
	mov r0,r0,lsl#24
	opINC8 r0
	mov r0,r0,lsr#24
	.endm

	.macro opINC16 reg
	add \reg,\reg,#0x00010000
	.endm
;@---------------------------------------

	.macro opINrC
	mov addy,z80bc,lsr#16
	bl Z80In
	getPzsTbl r1
	ldrb r1,[r1,r0]				;@ Get PZS
	and z80f,z80f,#PSR_C		;@ Keep C
	orr z80f,z80f,r1
	.endm
;@---------------------------------------

	.macro opLDIM16
	ldrb r0,[z80pc],#1
	ldrb r1,[z80pc],#1
	orr r0,r0,r1,lsl#8
	.endm

	.macro opLDIM8H reg
	ldrb r0,[z80pc],#1
	and \reg,\reg,#0x00ff0000
	orr \reg,\reg,r0,lsl#24
	.endm

	.macro opLDIM8L reg
	ldrb r0,[z80pc],#1
	and \reg,\reg,#0xff000000
	orr \reg,\reg,r0,lsl#16
	.endm
;@---------------------------------------

	.macro opOR reg s
	orr z80a,z80a,\reg,lsl#(\s)
	getPzsTbl r1
	ldrb z80f,[r1,z80a,lsr#24]	;@ Get PZS
	.endm

	.macro opORA
	getPzsTbl r1
	ldrb z80f,[r1,z80a,lsr#24]	;@ Get PZS
	fetch 4
	.endm

	.macro opORH reg
	and r0,\reg,#0xFF000000
	opOR r0,0
	fetch 4
	.endm

	.macro opORL reg
	opOR \reg,8
	fetch 4
	.endm

	.macro opORb
	opOR r0,24
	.endm
;@---------------------------------------

	.macro opOUTCr
	mov addy,z80bc,lsr#16
	bl Z80Out
	fetch 12
	.endm

	.macro opOUTCrH reg
	mov r0,\reg,lsr#24
	opOUTCr
	.endm

	.macro opOUTCrL reg
	mov r0,\reg,lsr#16
;@	and r0,r0,#0xFF
	opOUTCr
	.endm
;@---------------------------------------

	.macro opRES reg
	mov r0,r0,lsr#3
	bic \reg,\reg,r1,lsl r0		;@ r0 0x10-0x17
	.endm

	.macro opRESH reg
	mov r1,#0x00000100
	opRES \reg
	fetch 8
	.endm

	.macro opRESL reg
	mov r1,#0x00000001
	opRES \reg
	fetch 8
	.endm

	.macro opRESmem x
	readMem8
	bic r0,r0,#1<<(\x)			;@ Bit ?
	writeMem8
	fetch 15
	.endm
;@---------------------------------------

	.macro opRL reg1 reg2 s
	movs \reg1,\reg2,lsl#(\s)
	tst z80f,#PSR_C				;@ Doesn't affect ARM carry, as long as the immediate value is < 0x100. Watch out!
	orrne \reg1,\reg1,#0x01000000
;@	and r2,z80f,#PSR_C
;@	orr \reg1,\reg1,r2,lsl#23
	getPzsTbl r1
	ldrb z80f,[r1,\reg1,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	.endm

	.macro opRLA
	opRL z80a,z80a,1
	fetch 8
	.endm

	.macro opRLH reg
	and r0,\reg,#0xFF000000		;@ Mask high to r0
	adds \reg,\reg,r0
	tst z80f,#PSR_C				;@ Doesn't affect ARM carry, as long as the immediate value is < 0x100. Watch out!
	orrne \reg,\reg,#0x01000000
	getPzsTbl r1
	ldrb z80f,[r1,\reg,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	fetch 8
	.endm

	.macro opRLL reg
	opRL r0,\reg,9
	and \reg,\reg,#0xFF000000	;@ Mask out high
	orr \reg,\reg,r0,lsr#8
	fetch 8
	.endm

	.macro opRLb
	opRL r0,r0,25
	mov r0,r0,lsr#24
	.endm
;@---------------------------------------

	.macro opRLC reg1 reg2 s
	movs \reg1,\reg2,lsl#(\s)
	orrcs \reg1,\reg1,#0x01000000
	getPzsTbl r1
	ldrb z80f,[r1,\reg1,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	.endm

	.macro opRLCA
	opRLC z80a,z80a,1
	fetch 8
	.endm

	.macro opRLCH reg
	and r0,\reg,#0xFF000000		;@ Mask high to r0
	adds \reg,\reg,r0
	orrcs \reg,\reg,#0x01000000
	getPzsTbl r1
	ldrb z80f,[r1,\reg,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	fetch 8
	.endm

	.macro opRLCL reg
	opRLC r0,\reg,9
	and \reg,\reg,#0xFF000000	;@ Mask out high
	orr \reg,\reg,r0,lsr#8
	fetch 8
	.endm

	.macro opRLCb
	opRLC r0,r0,25
	mov r0,r0,lsr#24
	.endm
;@---------------------------------------

	.macro opRR reg1 reg2 s
	movs \reg1,\reg2,lsr#(\s)
	tst z80f,#PSR_C				;@ Doesn't affect ARM carry, as long as the immediate value is < 0x100. Watch out!
	orrne \reg1,\reg1,#0x00000080
;@	and r1,z80f,#PSR_C
;@	orr \reg1,\reg1,r1,lsl#6
	getPzsTbl r1
	ldrb z80f,[r1,\reg1]		;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	.endm

	.macro opRRA
	orr z80a,z80a,z80f,lsr#1	;@ Get C
	movs z80a,z80a,ror#25
	mov z80a,z80a,lsl#24
	getPzsTbl r1
	ldrb z80f,[r1,z80a,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	fetch 8
	.endm

	.macro opRRH reg
	orr r0,\reg,z80f,lsr#1		;@ Get C
	movs r0,r0,ror#25
	and \reg,\reg,#0x00FF0000	;@ Mask out low
	orr \reg,\reg,r0,lsl#24
	getPzsTbl r1
	ldrb z80f,[r1,\reg,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	fetch 8
	.endm

	.macro opRRL reg
	and r0,\reg,#0x00FF0000		;@ Mask out low to r0
	opRR r0,r0,17
	and \reg,\reg,#0xFF000000	;@ Mask out high
	orr \reg,\reg,r0,lsl#16
	fetch 8
	.endm

	.macro opRRb
	opRR r0,r0,1
	.endm
;@---------------------------------------

	.macro opRRC reg1 reg2 s
	movs \reg1,\reg2,lsr#(\s)
	orrcs \reg1,\reg1,#0x00000080
	getPzsTbl r1
	ldrb z80f,[r1,\reg1]		;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	.endm

	.macro opRRCA
	opRRC z80a,z80a,25
	mov z80a,z80a,lsl#24
	fetch 8
	.endm

	.macro opRRCH reg
	movs r0,\reg,lsr#25
	addcs r0,r0,#0x00000081
	sub \reg,\reg,r0,lsl#24
	getPzsTbl r1
	ldrb z80f,[r1,\reg,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	fetch 8
	.endm

	.macro opRRCL reg
	and r0,\reg,#0x00FF0000		;@ Mask low to r0
	opRRC r0,r0,17
	and \reg,\reg,#0xFF000000	;@ Mask out high
	orr \reg,\reg,r0,lsl#16
	fetch 8
	.endm

	.macro opRRCb
	opRRC r0,r0,1
	.endm
;@---------------------------------------

	.macro opSBC
	and z80f,z80f,#PSR_C
	subs r0,r0,z80f,lsl#7		;@ Fix up r0 and set correct C.
	eor z80f,r0,z80a,lsr#24		;@ Prepare for check of H
	sbcs z80a,z80a,r0,ror#8
	mrs r0,cpsr
	eor z80f,z80f,z80a,lsr#24
	and z80f,z80f,#PSR_H		;@ H, correct
	orr z80f,z80f,r0,lsr#28		;@ S,Z,V&C
	eor z80f,z80f,#PSR_C|PSR_n	;@ Invert C and set n.
	.endm

	.macro opSBCA
	ands z80a,z80f,#PSR_C		;@ Clear z80a, check C
	movne z80a,#0xFF000000
	moveq z80f,#PSR_n+PSR_Z
	movne z80f,#PSR_n+PSR_S+PSR_C+PSR_H
	fetch 4
	.endm

	.macro opSBCH reg
	mov r0,\reg,lsr#24
	opSBC
	fetch 4
	.endm

	.macro opSBCL reg
	mov r0,\reg,lsl#8
	eor z80f,z80f,#PSR_C		;@ Invert C
	movs z80f,z80f,lsr#2		;@ Get C
	sbccc r0,r0,#0xFF000000
	mov r1,z80a,lsl#4			;@ Prepare for check of H
	sbcs z80a,z80a,r0
	mrs z80f,cpsr				;@ S,Z,V&C
	mov z80f,z80f,lsr#28
	eor z80f,z80f,#PSR_C|PSR_n	;@ Invert C and set n.
	cmp r1,r0,lsl#4
	orrcc z80f,z80f,#PSR_H		;@ H, correct
	fetch 4
	.endm

	.macro opSBCb
	opSBC
	.endm
;@---------------------------------------

	.macro opSBC16 reg
	and z80f,z80f,#PSR_C
	subs r1,z80f,z80f,lsl#1		;@ Fix up r1 and set correct C.
	orr r0,\reg,r1,lsr#16
	mov r1,z80hl,lsl#4			;@ Prepare for check of H
	sbcs z80hl,z80hl,r0
	mrs z80f,cpsr				;@ S,Z,V&C
	mov z80f,z80f,lsr#28
	eor z80f,z80f,#PSR_C|PSR_n	;@ Invert C and set n.
	cmp r1,r0,lsl#4
	orrcc z80f,z80f,#PSR_H
	fetch 15
	.endm

	.macro opSBC16HL
	ands z80hl,z80f,z80f,lsl#31	;@ Clear z80hl, check C
	subcs z80hl,z80hl,#0x00010000
	movcc z80f,#PSR_n+PSR_Z
	movcs z80f,#PSR_n+PSR_S+PSR_C+PSR_H
	fetch 15
	.endm
;@---------------------------------------

	.macro opSET reg
	mov r0,r0,lsr#3
	and r0,r0,#7
	orr \reg,\reg,r1,lsl r0		;@ r0 0-7
	.endm

	.macro opSETH reg
	mov r1,#0x01000000
	opSET \reg
	fetch 8
	.endm

	.macro opSETL reg
	mov r1,#0x00010000
	opSET \reg
	fetch 8
	.endm

	.macro opSETmem x
	readMem8
	orr r0,r0,#1<<(\x)			;@ Bit ?
	writeMem8
	fetch 15
	.endm
;@---------------------------------------

	.macro opSLA reg1 reg2 s
	movs \reg1,\reg2,lsl#(\s)
	getPzsTbl r1
	ldrb z80f,[r1,\reg1,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	.endm

	.macro opSLAA
	opSLA z80a,z80a,1
	fetch 8
	.endm

	.macro opSLAH reg
	and r0,\reg,#0xFF000000		;@ Mask high to r0
	adds \reg,\reg,r0
	getPzsTbl r1
	ldrb z80f,[r1,\reg,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	fetch 8
	.endm

	.macro opSLAL reg
	opSLA r0,\reg,9
	and \reg,\reg,#0xFF000000	;@ Mask out high
	orr \reg,\reg,r0,lsr#8
	fetch 8
	.endm

	.macro opSLAb
	opSLA r0,r0,25
	mov r0,r0,lsr#24
	.endm
;@---------------------------------------

	.macro opSLL reg1 reg2 s
	movs \reg1,\reg2,lsl#(\s)
	orr \reg1,\reg1,#0x01000000
	getPzsTbl r1
	ldrb z80f,[r1,\reg1,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	.endm

	.macro opSLLA
	opSLL z80a,z80a,1
	fetch 8
	.endm

	.macro opSLLH reg
	mvn r0,\reg,lsr#24
	subs \reg,\reg,r0,lsl#24
	getPzsTbl r1
	ldrb z80f,[r1,\reg,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	fetch 8
	.endm

	.macro opSLLL reg
	opSLL r0,\reg,9
	and \reg,\reg,#0xFF000000	;@ Mask out high
	orr \reg,\reg,r0,lsr#8
	fetch 8
	.endm

	.macro opSLLb
	opSLL r0,r0,25
	mov r0,r0,lsr#24
	.endm
;@---------------------------------------

	.macro opSRA reg1 reg2
	movs \reg1,\reg2,asr#25
	and \reg1,\reg1,#0xFF
	getPzsTbl r1
	ldrb z80f,[r1,\reg1]		;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	.endm

	.macro opSRAA
	movs z80a,z80a,asr#25
	mov z80a,z80a,lsl#24
	getPzsTbl r1
	ldrb z80f,[r1,z80a,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	fetch 8
	.endm

	.macro opSRAH reg
	movs r0,\reg,asr#25
	and \reg,\reg,#0x00FF0000	;@ Mask out low
	orr \reg,\reg,r0,lsl#24
	getPzsTbl r1
	ldrb z80f,[r1,\reg,lsr#24]	;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	fetch 8
	.endm

	.macro opSRAL reg
	mov r0,\reg,lsl#8
	opSRA r0,r0
	and \reg,\reg,#0xFF000000	;@ Mask out high
	orr \reg,\reg,r0,lsl#16
	fetch 8
	.endm

	.macro opSRAb
	and r1,r0,#0x80
	orrs r0,r1,r0,lsr#1
	getPzsTbl r1
	ldrb z80f,[r1,r0]			;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	.endm
;@---------------------------------------

	.macro opSRL reg1 reg2 s
	movs \reg1,\reg2,lsr#(\s)
	getPzsTbl r1
	ldrb z80f,[r1,\reg1]		;@ Get PZS
	orrcs z80f,z80f,#PSR_C
	.endm

	.macro opSRLA
	opSRL z80a,z80a,25
	mov z80a,z80a,lsl#24
	fetch 8
	.endm

	.macro opSRLH reg
	opSRL r0,\reg,25
	and \reg,\reg,#0x00FF0000	;@ Mask out low
	orr \reg,\reg,r0,lsl#24
	fetch 8
	.endm

	.macro opSRLL reg
	mov r0,\reg,lsl#8
	opSRL r0,r0,25
	and \reg,\reg,#0xFF000000	;@ Mask out high
	orr \reg,\reg,r0,lsl#16
	fetch 8
	.endm

	.macro opSRLb
	opSRL r0,r0,1
	.endm
;@---------------------------------------

	.macro opSUB reg s
	mov r1,z80a,lsl#4 			;@ Prepare for check of half carry
	subs z80a,z80a,\reg,lsl#(\s)
	mrs z80f,cpsr				;@ S,Z,V&C
	mov z80f,z80f,lsr#28
	eor z80f,z80f,#PSR_C|PSR_n	;@ Invert C and set n
	cmp r1,\reg,lsl#(\s)+4
	orrcc z80f,z80f,#PSR_H
	.endm

	.macro opSUBA
	mov z80a,#0
	mov z80f,#PSR_Z|PSR_n		;@ Set Z & n
	fetch 4
	.endm

	.macro opSUBH reg
	and r0,\reg,#0xFF000000
	opSUB r0,0
	fetch 4
	.endm

	.macro opSUBL reg
	opSUB \reg,8
	fetch 4
	.endm

	.macro opSUBb
	opSUB r0,24
	.endm
;@---------------------------------------

	.macro opXOR reg s
	eor z80a,z80a,\reg,lsl#(\s)
	getPzsTbl r0
	ldrb z80f,[r0,z80a,lsr#24]	;@ Get PZS
	.endm

	.macro opXORA
	mov z80a,#0					;@ Clear A.
	mov z80f,#PSR_Z|PSR_P		;@ Z & P
	fetch 4
	.endm

	.macro opXORH reg
	and r0,\reg,#0xFF000000
	opXOR r0,0
	fetch 4
	.endm

	.macro opXORL reg
	opXOR \reg,8
	fetch 4
	.endm

	.macro opXORb
	opXOR r0,24
	.endm
;@---------------------------------------
