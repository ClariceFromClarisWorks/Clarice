/* GPL-3.0-or-later. Preserve the interrupted application's registers. */
 .text
 .even
 .globl hil_timer
hil_timer:
 movem.l %d0-%d7/%a0-%a6,-(%sp)
 move.l %a1,-(%sp)
 jsr hil_tick
 addq.l #4,%sp
 movem.l (%sp)+,%d0-%d7/%a0-%a6
 rts
