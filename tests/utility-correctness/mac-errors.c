extern void error(const char *,int);
int clarice_main(int argc,char **argv){(void)argc;(void)argv;error("missing fixture",-43);error("full fixture",-34);error("memory fixture",-108);error("unknown fixture",-32000);return 0;}
