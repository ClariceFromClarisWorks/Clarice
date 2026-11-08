/* Baseline 68000; C stack ABI, caller-saved d0/d1/a0/a1 only. */
.text
.even
.globl asm_length
asm_length:
 move.l 4(%sp),%a0
 moveq #0,%d0
1: tst.b (%a0)+
 beq 2f
 addq.l #1,%d0
 bra 1b
2: rts
.globl asm_glyph
asm_glyph:
 move.l 4(%sp),%a0
 move.l 8(%sp),%a1
 move.l 12(%sp),%d0
 move.l 16(%sp),%d1
 beq 2f
 subq.l #1,%d1
1: move.b (%a1)+,(%a0)
 adda.l %d0,%a0
 dbra %d1,1b
2: rts
.globl asm_contains
asm_contains:
 movem.l %a2-%a3,-(%sp)
 move.l 12(%sp),%a0
 move.l 16(%sp),%a1
1: move.l %a0,%a2
 move.l %a1,%a3
2: move.b (%a3)+,%d0
 beq 4f
 cmp.b (%a2)+,%d0
 bne 3f
 bra 2b
3: tst.b (%a0)+
 bne 1b
 moveq #0,%d0
 bra 5f
4: moveq #1,%d0
5: movem.l (%sp)+,%a2-%a3
 rts
.globl sys6_memtop
sys6_memtop:
 move.l 0x108,%d0
 rts

/* Packed monochrome cell compositor, 68000 C ABI. Glyph stride is 32. */
.globl asm_cells
asm_cells:
 movem.l %d2-%d7/%a2-%a5,-(%sp)
 move.l 44(%sp),%a0
 move.l 48(%sp),%a1
 move.l 52(%sp),%a2
 move.l 56(%sp),%d7
 move.l 60(%sp),%d6
 move.l 64(%sp),%d5
 moveq #0,%d4
 tst.l %d7
 beq 4f
1: moveq #0,%d0
 move.b (%a1)+,%d0
 lsl.l #5,%d0
 lea (%a2,%d0.l),%a3
 move.l %d4,%d2
 andi.l #7,%d2
 moveq #8,%d3
 sub.l %d2,%d3
 move.l %d4,%d0
 lsr.l #3,%d0
 lea (%a0,%d0.l),%a4
 move.l %d6,%d1
 subq.w #1,%d1
2: moveq #0,%d0
 move.b (%a3)+,%d0
 move.l %d0,%a5
 lsr.b %d2,%d0
 or.b %d0,(%a4)
 tst.w %d2
 beq 3f
 move.l %a5,%d0
 lsl.b %d3,%d0
 or.b %d0,1(%a4)
3: lea 64(%a4),%a4
 dbra %d1,2b
 add.l %d5,%d4
 subq.l #1,%d7
 bne 1b
4: movem.l (%sp)+,%d2-%d7/%a2-%a5
 rts

/* Even-aligned row transfer. Width can be any positive number of bytes. */
.globl asm_copy_rows
asm_copy_rows:
 movem.l %d2-%d5/%a2-%a3,-(%sp)
 move.l 28(%sp),%a0
 move.l 32(%sp),%a1
 move.l 36(%sp),%d2
 move.l 40(%sp),%d3
 move.l 44(%sp),%d4
 move.l 48(%sp),%d5
 tst.l %d3
 beq 5f
1: move.l %a0,%a2
 move.l %a1,%a3
 move.l %d2,%d0
 lsr.l #5,%d0
 beq 7f
 subq.w #1,%d0
8:
 .rept 8
 move.l (%a3)+,(%a2)+
 .endr
 dbra %d0,8b
7: move.l %d2,%d0
 andi.l #28,%d0
 lsr.l #2,%d0
 beq 3f
 subq.w #1,%d0
2: move.l (%a3)+,(%a2)+
 dbra %d0,2b
3: btst #1,%d2
 beq 4f
 move.w (%a3)+,(%a2)+
4: btst #0,%d2
 beq 6f
 move.b (%a3),(%a2)
6: adda.l %d4,%a0
 adda.l %d5,%a1
 subq.l #1,%d3
 bne 1b
5: movem.l (%sp)+,%d2-%d5/%a2-%a3
 rts
