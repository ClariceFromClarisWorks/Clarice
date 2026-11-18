#include "libbb.h"
#include <Files.h>
extern int cli_resolve_info(const char *,char *,char *,CInfoPBRec *);
long bb_strtol(const char *s,char **e,int b){errno=0;return strtol(s,e,b);}
long xstrtoll(const char *s,int b){char *e;long n=bb_strtol(s,&e,b);if(errno||e==s||*e)bb_error_msg_and_die("invalid number: %s",s);return n;}
char *concat_path_file(const char *p,const char *n){return xasprintf("%s%s%s",p,(*p&&p[strlen(p)-1]!='/')?"/":"",n);}
/* Only file kind/length are consumed. No Unix identity shortcut or permissions. */
int addition_stat(const char *p,struct stat *s){CInfoPBRec pb;char native[512];int e=cli_resolve_info(p,native,0,&pb);if(e){errno=e==-43?ENOENT:EIO;return -1;}memset(s,0,sizeof(*s));s->st_mode=(pb.hFileInfo.ioFlAttrib&16)?S_IFDIR:S_IFREG;s->st_size=S_ISDIR(s->st_mode)?0:pb.hFileInfo.ioFlLgLen;return 0;}
void xstat(const char *p,struct stat *s){if(addition_stat(p,s))bb_simple_perror_msg_and_die(p);}
FILE *addition_diff_input(const char *p){
 if(!LONE_DASH(p))return xfopen(p,"r");
 const char *name=port_temp_new();if(!name)bb_simple_perror_msg_and_die("temporary file");
 FILE *f=xfopen(name,"w+");char b[4096];size_t n;
 while((n=fread(b,1,sizeof(b),stdin)))if(fwrite(b,1,n,f)!=n)bb_simple_perror_msg_and_die("temporary write");
 die_if_ferror(stdin,"stdin");if(fflush(f)||fseek(f,0,SEEK_SET))bb_simple_perror_msg_and_die("temporary seek");return f;
}
