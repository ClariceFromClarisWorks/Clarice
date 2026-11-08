/* Guest-only diagnostic; not installed as a user utility. */
extern unsigned long cli_ticks(void);
extern int app_yield(void);
extern void app_activity_interactive(int);
int clarice_main(int argc,char **argv){
 (void)argv;app_activity_interactive(argc>1);
 unsigned long start=cli_ticks();
 while(cli_ticks()-start<900)if(app_yield())return 130;
 return 0;
}
