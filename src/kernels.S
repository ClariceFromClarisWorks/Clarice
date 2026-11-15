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
.globl asm_copy_printable
/* Copy at most 132 bytes, stopping before the first byte below 32.
   High MacRoman bytes are printable. Byte accesses permit odd source/dest.
   Return the copied count; do not read beyond the supplied bound. */
asm_copy_printable:
 move.l 4(%sp),%a0
 move.l 8(%sp),%a1
 move.l 12(%sp),%d1
 tst.l %d1
 ble 2f
 subq.w #1,%d1
1: cmpi.b #32,(%a1)
 bcs 2f
 move.b (%a1)+,(%a0)+
 dbra %d1,1b
2: move.l %a1,%d0
 sub.l 8(%sp),%d0
 rts
.globl asm_clear_terminal_row
/* Terminal rows are even-aligned and exactly 132 bytes. */
asm_clear_terminal_row:
 move.l 4(%sp),%a0
 moveq #0,%d0
 .rept 33
 move.l %d0,(%a0)+
 .endr
 rts
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
.globl asm_ascii_cells
asm_ascii_cells:
 cmpi.l #6,24(%sp)
 bne .Lascii_generic
 cmpi.l #11,20(%sp)
 bne .Lascii_generic
 cmpi.l #4,16(%sp)
 bcc .Lascii_four
.Lascii_generic:
 move.l #1024,%d0
 bra .Lcells_enter
.globl asm_cells
asm_cells:
 moveq #0,%d0
.Lcells_enter:
 moveq #0,%d1
 bra .Lcells_offset
.globl asm_ascii_cells_at
asm_ascii_cells_at:
 move.l #1024,%d0
 move.l 28(%sp),%d1
.Lcells_offset:
 movem.l %d2-%d7/%a2-%a5,-(%sp)
 move.l 44(%sp),%a0
 move.l 48(%sp),%a1
 move.l 52(%sp),%a2
 suba.l %d0,%a2
 move.l 56(%sp),%d7
 move.l 60(%sp),%d6
 move.l 64(%sp),%d5
 move.l %d1,%d4
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
 /* A fitting glyph must not touch the next byte, even with a zero OR:
    the last scanline may end exactly at the staging-buffer boundary. */
 cmp.w %d5,%d3
 bge 3f
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

/* Four six-pixel cells form three complete bytes. Compose those bytes once,
   avoiding per-cell variable shifts and read/modify/write of overlapping pixels.
   Exact Monaco geometry only; remainder uses the bounded generic compositor. */
.Lascii_four:
 movem.l %d2-%d7/%a2-%a6,-(%sp)
 move.l 48(%sp),%a0
 move.l 52(%sp),%a1
 move.l 56(%sp),%a2
 suba.l #1024,%a2
 move.l 60(%sp),%d7
 move.l %d7,%d6
 lsr.l #2,%d6
 andi.w #3,%d7
.Lascii_group:
 moveq #0,%d0
 move.b (%a1)+,%d0
 lsl.w #5,%d0
 lea (%a2,%d0.w),%a3
 move.b (%a1)+,%d0
 andi.w #255,%d0
 lsl.w #5,%d0
 lea (%a2,%d0.w),%a4
 move.b (%a1)+,%d0
 andi.w #255,%d0
 lsl.w #5,%d0
 lea (%a2,%d0.w),%a5
 move.b (%a1)+,%d0
 andi.w #255,%d0
 lsl.w #5,%d0
 lea (%a2,%d0.w),%a6
 moveq #10,%d5
.Lascii_scan:
 move.b (%a3)+,%d0
 move.b (%a4)+,%d1
 move.b %d1,%d2
 lsr.b #6,%d2
 or.b %d2,%d0
 move.b %d0,(%a0)
 lsl.b #2,%d1
 move.b (%a5)+,%d2
 move.b %d2,%d3
 lsr.b #4,%d3
 or.b %d3,%d1
 move.b %d1,1(%a0)
 lsl.b #4,%d2
 move.b (%a6)+,%d3
 lsr.b #2,%d3
 or.b %d3,%d2
 move.b %d2,2(%a0)
 lea 64(%a0),%a0
 dbra %d5,.Lascii_scan
 lea -701(%a0),%a0
 subq.w #1,%d6
 bne .Lascii_group
 tst.w %d7
 beq .Lascii_done
 move.l #6,-(%sp)
 move.l #11,-(%sp)
 move.l %d7,-(%sp)
 pea 1024(%a2)
 move.l %a1,-(%sp)
 move.l %a0,-(%sp)
 jsr asm_ascii_cells
 lea 24(%sp),%sp
.Lascii_done:
 movem.l (%sp)+,%d2-%d7/%a2-%a6
 rts

/* Four-glyph groups emitted at a 1..7 bit offset. C bounds the count and
   clears the first/carry-tail bytes. Overlapping stores preserve prior groups;
   every word/long access is even, including odd-byte destination groups. */
.globl asm_ascii_groups_at
asm_ascii_groups_at:
 movem.l %d2-%d7/%a2-%a6,-(%sp)
 move.l 48(%sp),%a0
 move.l 52(%sp),%a1
 move.l 56(%sp),%a2
 suba.l #1024,%a2
 move.l 60(%sp),%d6
 lsr.l #2,%d6
 moveq #6,%d4
 sub.l 64(%sp),%d4
.Ldirect_group:
 moveq #0,%d0
 move.b (%a1)+,%d0
 lsl.w #5,%d0
 lea (%a2,%d0.w),%a3
 move.b (%a1)+,%d0
 andi.w #255,%d0
 lsl.w #5,%d0
 lea (%a2,%d0.w),%a4
 move.b (%a1)+,%d0
 andi.w #255,%d0
 lsl.w #5,%d0
 lea (%a2,%d0.w),%a5
 move.b (%a1)+,%d0
 andi.w #255,%d0
 lsl.w #5,%d0
 lea (%a2,%d0.w),%a6
 moveq #10,%d5
 move.l %a0,%d0
 btst #0,%d0
 bne .Ldirect_odd
.macro direct_scan odd
.Ldirect_scan\@:
 moveq #0,%d0
 move.b (%a3)+,%d0
 lsl.l #6,%d0
 or.b (%a4)+,%d0
 lsl.l #6,%d0
 or.b (%a5)+,%d0
 lsl.l #6,%d0
 or.b (%a6)+,%d0
 tst.w %d4
 bmi .Ldirect_right\@
 lsl.l %d4,%d0
 bra .Ldirect_store\@
.Ldirect_right\@:
 lsr.l #1,%d0
.Ldirect_store\@:
 moveq #0,%d1
.if \odd
 move.w -1(%a0),%d1
 swap %d1
 move.l %d0,%d2
 lsr.l #8,%d2
 or.l %d1,%d2
 move.l %d2,-1(%a0)
 move.b %d0,3(%a0)
.else
 move.b (%a0),%d1
 ror.l #8,%d1
 or.l %d1,%d0
 move.l %d0,(%a0)
.endif
 lea 64(%a0),%a0
 dbra %d5,.Ldirect_scan\@
.endm
 direct_scan 0
 bra .Ldirect_next
.Ldirect_odd:
 direct_scan 1
.Ldirect_next:
 lea -701(%a0),%a0
 subq.w #1,%d6
 bne .Ldirect_group
 movem.l (%sp)+,%d2-%d7/%a2-%a6
 rts

/* In-place right shift, end points at the last even word of the first row.
   C validates 1..32 words, 1..64 rows, 1..15 bits and all 64-byte row bounds.
   A single 32-bit shift combines adjacent words; backwards writes preserve
   unread input. The first word never reads before the validated span. */
.globl asm_align_row_bits
asm_align_row_bits:
 movem.l %d2-%d5/%a2,-(%sp)
 move.l 24(%sp),%a2
 move.l 28(%sp),%d2
 move.l 32(%sp),%d3
 move.l 36(%sp),%d4
 subq.w #1,%d2
 subq.w #1,%d3
.Lalign_row:
 move.l %a2,%a0
 lea 2(%a2),%a1
 move.w (%a0),%d0
 swap %d0
 move.w %d2,%d5
 beq .Lalign_first
 subq.w #1,%d5
.Lalign_word:
 move.w -(%a0),%d0
 swap %d0
 move.l %d0,%d1
 lsr.l %d4,%d1
 move.w %d1,-(%a1)
 dbra %d5,.Lalign_word
.Lalign_first:
 swap %d0
 lsr.w %d4,%d0
 move.w %d0,-(%a1)
 lea 64(%a2),%a2
 dbra %d3,.Lalign_row
 movem.l (%sp)+,%d2-%d5/%a2
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

/* Equality only, for even-aligned 132-byte terminal model rows.
   General libc memcmp uses byte loops at 2-mod-4 addresses on 68000.
   Check every byte including the cursor marker, without reading past the row. */
.globl asm_terminal_row_changed
asm_terminal_row_changed:
 move.l 4(%sp),%a0
 move.l 8(%sp),%a1
 .rept 33
 cmpm.l (%a0)+,(%a1)+
 bne 1f
 .endr
 moveq #0,%d0
 rts
1: moveq #1,%d0
 rts
