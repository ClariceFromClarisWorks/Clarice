/* FNV-1a, 68000 only. 0x01000193*h = (h<<24) + 403*h.
   403*h = ((3*h<<3)+h)<<4 + 3*h. No general multiply helper. */
.text
.even
.globl asm_fnv1a
asm_fnv1a:
 movem.l %d2-%d3,-(%sp)
 move.l 12(%sp),%d0
 move.l 16(%sp),%a0
 move.l 20(%sp),%d3
 beq 2f
1:
 moveq #0,%d1
 move.b (%a0)+,%d1
 eor.l %d1,%d0
 move.l %d0,%d1
 lsl.l #1,%d1
 add.l %d0,%d1
 move.l %d1,%d2
 lsl.l #3,%d2
 add.l %d0,%d2
 lsl.l #4,%d2
 add.l %d1,%d2
 lsl.l #8,%d0
 swap %d0
 clr.w %d0
 add.l %d2,%d0
 subq.l #1,%d3
 bne 1b
2:
 movem.l (%sp)+,%d2-%d3
 rts
