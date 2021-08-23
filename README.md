# ARMZ80
--------------------------------------------------------------------------------
This is a Zilog Z80 cpu emulator for ARM32 architechtures.

--------------------------------------------------------------------------------
How to use:
--------------------------------------------------------------------------------
One can not simply use the CPU core in a C environment.
Register 3 (r3) is used as a cpu flag holder, so it must be preserved if calling
C code. Register 12 (r12 a.k.a addy) is used as address in read/write operations.

You can use defines to alter the behaviour of the emulator.
"Z80_LARGE_MAP" use a larger memory map for finer control of memory layout.
"Z80_FAST" inserts more of the mem handling in the opcode, for a speed up (YMMV).
"Z80_USE_FAST_MEM" use fast memory on GBA and NDS for the cpu core.
--------------------------------------------------------------------------------
Credits:
--------------------------------------------------------------------------------
Thanks to:
Reesy & Dwedit for help and inspiration with a lot of things.


--------------------------------------
Fredrik Ahlström
Twitter @TheRealFluBBa
http://www.github.com/FluBBaOfWard
--------------------------------------
