/* SEControl bounded input, GPL-3.0-or-later.
 * Mouse-state technique adapted from MiniVNC, Copyright 2022-2024 Marcio Teixeira.
 * See vendor/LICENSE-MiniVNC.txt. No VNC protocol or trap patches.
 */
#include <Multiverse.h>
typedef struct MouseStep { short x,y,down; unsigned short tick; } MouseStep;
typedef struct MousePlan {
 TMTask task; /* first: Time Manager supplies its address in A1 on System 6.0.3+ */
 volatile short active,cancel,down,index,count,error;
 unsigned long started,completed;
 MouseStep steps[34];
} MousePlan;
extern void hil_timer(void);
extern void hil_tick(MousePlan *);
extern void hil_release(MousePlan *);
extern OSErr hil_post(short,long,short,short,short);
