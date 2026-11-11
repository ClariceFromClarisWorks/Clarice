/* Independently compiled fixture. No terminal link or resource rebuild required. */
extern long cli_write(int,const void *,unsigned long);
#ifndef PROBE_VERSION
#define PROBE_VERSION 2
#endif
int clarice_main(int argc,char **argv){
 (void)argc;(void)argv;
#if PROBE_VERSION == 1
 static const char text[]="independent command v1\n";
#else
 static const char text[]="independent command v2\n";
#endif
 return cli_write(1,text,sizeof text-1)==sizeof text-1?0:1;
}
