/* Adapter-level malformed names and disk failure probes; ASan/UBSan host only. */
#define main adapter_main
#include "../../src/commands/kermit.c"
#undef main
#include <assert.h>
int main(void){
 assert(valid_name((UCHAR *)"data.bin"));
 const char *bad[]={"",".","..","../escape","volume:file","a/b","a\\b","bad\nname",0};
 for(int i=0;bad[i];i++)assert(!valid_name((const UCHAR *)bad[i]));
 char longname[70];memset(longname,'a',sizeof longname);longname[69]=0;assert(!valid_name((UCHAR *)longname));
 file_fd=-1;assert(write_file(&protocol,(UCHAR *)"x",1)==X_ERROR&&io_failed);
 assert(close_file(&protocol,0,2)==X_ERROR&&!completed);
 puts("PASS: invalid names rejected; failed writes cannot commit");return 0;
}
