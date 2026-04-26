# ARMZ80 V1.9.11

This is a Zilog Z80 cpu emulator for ARM32 architechtures.

## How to use

One can not simply use the CPU core in a C environment.
Register 3 (r3) is used as a cpu flag holder, so it must be preserved if calling
C code. Register 12 (r12 a.k.a addy) is used as address in read/write operations.
You must also implement Z80In/Z80InBC & Z80Out/Z80OutBC. 

You can use defines to alter the behaviour of the emulator.
"Z80_LARGE_MAP" use a larger memory map for finer control of memory layout.
"Z80_FAST" inserts more of the mem handling in the opcode for a speed up (YMMV).
"Z80_DIRECT_MEM" skip the mem read jump table & just use the mem pointer table.
"Z80_INV_MEM_TBL" to use an inverted table for even faster mem reads.
"Z80_USE_FAST_MEM" use fast memory on GBA and NDS for the cpu core.

## Projects that use this cpu core

* <https://github.com/FluBBaOfWard/BlackTigerDS>
* <https://github.com/FluBBaOfWard/BlackTigerGBA>
* <https://github.com/FluBBaOfWard/GhostsNGoblinsDS>
* <https://github.com/FluBBaOfWard/GreenBeretGBA>
* <https://github.com/FluBBaOfWard/K80DS>
* <https://github.com/FluBBaOfWard/NGPDS>
* <https://github.com/FluBBaOfWard/NGPGBA>
* <https://github.com/FluBBaOfWard/PunchOutDS>
* <https://github.com/FluBBaOfWard/S8DS>
* <https://github.com/FluBBaOfWard/S8GBA>

## Credits

Thanks to:
Reesy & Dwedit for help and inspiration with a lot of things.

Fredrik Ahlström

<https://bsky.app/profile/therealflubba.bsky.social>

<https://www.github.com/FluBBaOfWard>

X/Twitter @TheRealFluBBa
