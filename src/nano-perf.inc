/* Opt-in measurements only; no allocations or production resident state. */
#ifdef CLARICE_NANO_PERF
static unsigned long nano_draws,nano_rows,nano_text_ticks,nano_copy_ticks,nano_draw_ticks;
static unsigned long nano_marks[13],nano_key_tick,nano_keys,nano_key_sum,nano_key_max,nano_heap;
void app_nano_mark(int n){if(n>=0&&n<13&&!nano_marks[n])nano_marks[n]=TickCount();}
static void nano_submit(const char *s){if(strncmp(s,"nano",4)||(s[4]&&s[4]!=' '))return;memset(nano_marks,0,sizeof nano_marks);nano_draws=nano_rows=nano_text_ticks=nano_copy_ticks=nano_draw_ticks=0;nano_key_tick=nano_keys=nano_key_sum=nano_key_max=0;nano_marks[0]=TickCount();nano_heap=FreeMem();}
static void nano_input(void){if(nano_marks[8]&&!nano_key_tick)nano_key_tick=TickCount();}
static void nano_painted(void){if(nano_key_tick){unsigned long n=TickCount()-nano_key_tick;nano_keys++;nano_key_sum+=n;if(n>nano_key_max)nano_key_max=n;nano_key_tick=0;}}
void app_nano_stats(char *b,unsigned long n){snprintf(b,n,"Nano ticks: %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu; input batches=%lu sum=%lu max=%lu; initial-heap=%lu\n",nano_marks[0],nano_marks[1],nano_marks[2],nano_marks[3],nano_marks[4],nano_marks[5],nano_marks[6],nano_marks[7],nano_marks[8],nano_marks[9],nano_marks[10],nano_marks[11],nano_marks[12],nano_keys,nano_key_sum,nano_key_max,nano_heap);unsigned long used=strlen(b);if(used<n)snprintf(b+used,n-used,"Editor startup: draws=%lu rows=%lu text=%lu copy=%lu total=%lu ticks\n",nano_draws,nano_rows,nano_text_ticks,nano_copy_ticks,nano_draw_ticks);}
#else
#define nano_submit(s) ((void)0)
#define nano_input() ((void)0)
#define nano_painted() ((void)0)
#endif
