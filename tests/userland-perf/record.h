#include <Files.h>
#include <string.h>
extern int resolve(const char*,char*);
extern int cli_native_create(const char*,unsigned long,unsigned long);
extern int open_path(const char*,short,short*);
extern int cli_native_write(short,const void*,long*);
extern int cli_native_close(short);
static void record(const char *path,const char *text){char native[256];short ref;if(resolve(path,native))return;cli_native_create(native,'TEXT','ttxt');if(open_path(path,2,&ref))return;long n=strlen(text);cli_native_write(ref,text,&n);cli_native_close(ref);FlushVol(0,0);}
