/* Diagnostic-only ncurses flush measurement, excluded from normal ports. */
extern unsigned long cli_ticks(void);
extern int __real_doupdate(void);
extern void app_nano_refresh(unsigned long);
int __wrap_doupdate(void){unsigned long t=cli_ticks();int r=__real_doupdate();app_nano_refresh(cli_ticks()-t);return r;}
