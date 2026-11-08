/* Retro68 C ABI probe: one stack argument, d0 result, no saved-register use. */
.text
.even
.globl sys6_abi_probe
sys6_abi_probe:
    move.l 4(%sp),%d0
    addq.l #1,%d0
    rts
