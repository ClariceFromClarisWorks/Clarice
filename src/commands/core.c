/* Clarice core utilities: independently loaded 68000 command implementation. */
#include "../sys6.h"
#include <Files.h>
#include <Errors.h>
#include <OSUtils.h>
#include <string.h>
#include <stdio.h>
extern short output_ref,input_ref;
extern int output_error,script_depth;
extern void emit(const char *,long),say(const char *),errsay(const char *),error(const char *,int);
extern int resolve_ex(const char *,char *,char *,int),resolve(const char *,char *);
extern int info(const char *,CInfoPBRec *),is_link(const CInfoPBRec *);
extern void parent_path(char *);
extern int volume_name(int,char *),encode_name(const char *,char *,size_t),same_name(const char *,const char *);
extern int open_path(const char *,short,short *);
extern long cli_read(int,void *,unsigned long);
void format_created(char [17],unsigned long);
void format_decimal32(char *,unsigned long);
#include "../listing.inc"
static int cat_file(const char *s){char b[2048];short ref=-1;int e=0,status=0,standard=!strcmp(s,"-");if(!standard)e=open_path(s,fsRdPerm,&ref);if(e){error("cat",e);return 1;}
 for(;;){long n=sizeof b;if(standard){n=cli_read(0,b,sizeof b);e=n<0?ioErr:0;if(n<0)n=0;}else e=FSRead(ref,&n,b);
 if(n)emit(b,n);if(app_yield()){status=130;break;}if(output_error){status=1;break;}if(e==eofErr||(!e&&!n))break;if(e){error("cat read",e);status=1;break;}}
 if(!standard)FSClose(ref);return status;}

int clarice_main(int argc,char **av){int i,status=0;const char *name=strrchr(av[0],'/');name=name?name+1:av[0];
 if(!strcmp(name,"ls"))return ls_command(argc,av);
 if(!strcmp(name,"df"))return df_command(argc,av);
 if(!strcmp(name,"cat")){int files=0,options=1;for(i=1;i<argc;i++){if(options&&!strcmp(av[i],"--")){options=0;continue;}if(options&&!strcmp(av[i],"-u"))continue;if(options&&av[i][0]=='-'&&av[i][1]){errsay("usage: cat [-u] [--] [file ...]\n");return 2;}files++;int e=cat_file(av[i]);if(e==130)return e;if(e)status=e;}return files?status:cat_file("-");}
 if(!strcmp(name,"benchls")||!strcmp(name,"benchscan")){char report[320];unsigned long totals[3],first[3],entries=0,p0,e0,p1,e1;app_profile(&p0,&e0);for(i=0;i<3;i++){status=list(argc>1?av[1]:".",!strcmp(name,"benchscan")?-1:1,1,0,0,1);totals[i]=list_ticks;first[i]=list_first;entries=list_entries;if(status)break;}if(!status){app_profile(&p1,&e1);snprintf(report,sizeof report,"ls -la benchmark: %lu entries, 3 runs\nFirst entry ticks: %lu %lu %lu\nTotal ticks: %lu %lu %lu (60 ticks/sec)\n",entries,first[0],first[1],first[2],totals[0],totals[1],totals[2]);say(report);snprintf(report,sizeof report,"Three-run paint %lu ticks; events %lu ticks\n",p1-p0,e1-e0);say(report);}}

 else return 127;
 return status;
}
