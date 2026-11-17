/* Native 32-bit regression for the shared progress formatter. */
#include <stdio.h>
#include <string.h>
static unsigned long progress_clock(void){return 0;}
static int progress_tty(void){return 0;}
static int progress_columns(void){return 80;}
static void progress_row(const char *s,int done){(void)s;(void)done;}
#include "../../src/commands/progress.h"
extern void say(const char *);
int clarice_main(int argc,char **argv){(void)argc;(void)argv;
 static const unsigned long values[]={0,1023,1024,1048576,0x60000000UL,0x70000000UL,0x7fffffffUL,0xffffffffUL};
 static const char *want[]={"0 B","1023 B","1.00 KiB","1.00 MiB","1.50 GiB","1.75 GiB","2.00 GiB","4.00 GiB"};
 char got[24],line[96];int failed=0;
 for(unsigned i=0;i<sizeof values/sizeof *values;i++){
  progress_size(got,values[i]);snprintf(line,sizeof line,"%lu -> %s%s\n",values[i],got,strcmp(got,want[i])?" FAIL":"");say(line);failed|=strcmp(got,want[i])!=0;
 }
 say(failed?"SIZE32 FAIL\n":"SIZE32 PASS\n");return failed;
}
