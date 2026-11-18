#include "libbb.h"
#include "help.h"
/* Do not expand path-binding macro while forming the applet entry name. */
#undef dirname
#define STR_(x) #x
#define STR(x) STR_(x)
#define CAT_(a,b) a##b
#define CAT(a,b) CAT_(a,b)
extern int CAT(APPLET,_main)(int,char **);
int upstream_main(int argc,char **argv){
 applet_name=STR(APPLET); xfunc_error_retval=!strcmp(applet_name,"diff")?2:1; GETOPT_RESET();
 if(argc==2&&!strcmp(argv[1],"--version")){puts("BusyBox 1.37.0 / Clarice " STR(APPLET));return 0;}
 if(argc==2&&!strcmp(argv[1],"--help")){puts(CAT(HELP_,APPLET));return 0;}
 /* Upstream option parsing and tr may mutate argv: never lend shell storage. */
 char **copy=xmalloc((argc+1)*sizeof(char *));for(int i=0;i<argc;i++)copy[i]=xstrdup(argv[i]);copy[argc]=NULL;
 return CAT(APPLET,_main)(argc,copy);
}
