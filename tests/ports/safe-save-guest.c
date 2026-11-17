/* Real guest service exercise; use only a disposable file. */
#include "compat.h"
extern void *cli_replace_begin(const char *,char *,unsigned long);
extern int cli_replace_commit(void *);
extern void cli_replace_abort(void *);
static int contents(const char *name,const char *expected){char b[64]={0};FILE *f=fopen(name,"rb");if(!f)return 0;size_t n=fread(b,1,sizeof b-1,f);int e=fclose(f);return !e&&n==strlen(expected)&&!strcmp(b,expected);}
int upstream_main(int argc,char **argv){
 if(argc!=2)return 10;
 char stage[520];
 for(int i=0;i<20;i++){
  void *s=cli_replace_begin(argv[1],stage,sizeof stage);if(!s)return 11;
  FILE *f=fopen(stage,"wb");if(!f){cli_replace_abort(s);return 12;}
  if(fputs("safe replacement\n",f)<0||fclose(f)){cli_replace_abort(s);return 13;}
  if(cli_replace_commit(s)||!contents(argv[1],"safe replacement\n"))return 14;
 }
 void *s=cli_replace_begin(argv[1],stage,sizeof stage);if(!s)return 15;
 FILE *f=fopen(stage,"wb");if(!f){cli_replace_abort(s);return 16;}
 fputs("incomplete",f);fclose(f);cli_replace_abort(s);
 if(!contents(argv[1],"safe replacement\n"))return 17;
 /* Invocation cleanup must discard an uncommitted transaction too. */
 s=cli_replace_begin(argv[1],stage,sizeof stage);if(!s)return 18;
 puts("PASS 20 saves, exact contents, partial-save abort; pending transaction for invocation cleanup");return 0;
}
