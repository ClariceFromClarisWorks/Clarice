/* Actual runtime and terminal environment adapter, run in the guest. */
#include "ncurses-compat.h"
int upstream_main(int argc,char **argv){(void)argc;(void)argv;
 if(strcmp(getenv("TERM"),"clarice"))return 10;
 if(getenv("PRIVATE"))return 11;
 for(int i=0;i<40;i++){char n[16];snprintf(n,sizeof n,"E%d",i);const char *p=getenv(n);if(!p||strlen(p)!=2048)return 12;}
 const char *last=getenv("E39");if(setenv("COPY",last,1)||strlen(getenv("COPY"))!=2048)return 13;
 if(unsetenv("E0")||getenv("E0"))return 14;
 puts("PASS editor 40 exported 2048-byte values, TERM override, private exclusion, set/unset");return 0;
}
