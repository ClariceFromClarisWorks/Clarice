/* Investigation only: fixed ten-byte unsigned decimal field, no terminator.
 * Original Clarice experiment; no third-party assembly copied.
 * Pure 68000, odd output addresses allowed. Retro68 D0-D2/A0-A1 volatile.
 * Small values use DIVU; large values subtract decimal powers. No heap/cache.
 */
.text
.balign 2
.globl candidate_decimal_field
candidate_decimal_field:
 move.l %d3,-(%sp)
 move.l 8(%sp),%a0
 move.l 12(%sp),%d0
 cmp.l #65535,%d0
 bhi .Llarge
 move.l %a0,%a1
 adda.w #10,%a0
.Lsmall:
 divu.w #10,%d0
 move.l %d0,%d1
 swap %d1
 add.b #48,%d1
 move.b %d1,-(%a0)
 and.l #65535,%d0
 bne .Lsmall
.Lpad:
 cmpa.l %a1,%a0
 beq .Ldone
 move.b #32,-(%a0)
 bra .Lpad
.Llarge:
 lea .Lpowers(%pc),%a1
 moveq #0,%d3
.Lplace:
 move.l (%a1)+,%d1
 moveq #48,%d2
.Lsubtract:
 cmp.l %d1,%d0
 bcs .Ldigit
 sub.l %d1,%d0
 addq.b #1,%d2
 bra .Lsubtract
.Ldigit:
 cmp.b #48,%d2
 bne .Lnonzero
 tst.b %d3
 bne .Lemit
 cmp.l #1,%d1
 beq .Lemit
 moveq #32,%d2
 bra .Lemit
.Lnonzero:
 moveq #1,%d3
.Lemit:
 move.b %d2,(%a0)+
 cmp.l #1,%d1
 bne .Lplace
.Ldone:
 move.l (%sp)+,%d3
 rts
.balign 2
.Lpowers:
 .long 1000000000,100000000,10000000,1000000,100000,10000,1000,100,10,1
