.text
/* Four pre-shifted Monaco 9 contribution planes; plain 68000, C ABI.
   Each plane has 95 64-byte glyph slots, each with eleven longwords.
   Existing caller clears boundary/tail bytes and guarantees even row base,
   64-byte stride, 4..84 glyphs divisible by four and bounded output. */
.globl asm_ascii_planes_at
asm_ascii_planes_at:
 movem.l %d2-%d7/%a2-%a6,-(%sp)
 move.l 48(%sp),%a0
 move.l 52(%sp),%a1
 move.l 56(%sp),%a2
 suba.l #2048,%a2
 move.l 60(%sp),%d6
 lsr.l #2,%d6
.Lplanes_group:
 moveq #0,%d0
 move.b (%a1)+,%d0
 lsl.w #6,%d0
 lea (%a2,%d0.w),%a3
 move.b (%a1)+,%d0
 andi.w #255,%d0
 lsl.w #6,%d0
 lea (%a2,%d0.w),%a4
 adda.l #6080,%a4
 move.b (%a1)+,%d0
 andi.w #255,%d0
 lsl.w #6,%d0
 lea (%a2,%d0.w),%a5
 adda.l #12160,%a5
 move.b (%a1)+,%d0
 andi.w #255,%d0
 lsl.w #6,%d0
 lea (%a2,%d0.w),%a6
 adda.l #18240,%a6
 moveq #10,%d5
 move.l %a0,%d0
 btst #0,%d0
 bne .Lplanes_odd
.macro planes_scan odd
.Lplanes_scan\@:
 move.l (%a3)+,%d0
 or.l (%a4)+,%d0
 or.l (%a5)+,%d0
 or.l (%a6)+,%d0
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
 dbra %d5,.Lplanes_scan\@
.endm
 planes_scan 0
 bra .Lplanes_next
.Lplanes_odd:
 planes_scan 1
.Lplanes_next:
 lea -701(%a0),%a0
 subq.w #1,%d6
 bne .Lplanes_group
 movem.l (%sp)+,%d2-%d7/%a2-%a6
 rts
