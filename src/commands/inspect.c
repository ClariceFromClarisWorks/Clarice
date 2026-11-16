/* Native HFS traversal and inspection, with demand-allocated traversal and no link following. */
#include "mac-files.h"
#include <errno.h>
static char **predicates;static int predicate_count;static int maxdepth=2147483647,all,summarize,human,unit=512,brief;
static int match(const char *p,const char *s){const char *star=NULL,*retry=NULL;while(*s){if(*p=='?'||*p==*s){p++;s++;}else if(*p=='*'){star=p++;retry=s;}else if(star){p=star+1;s=++retry;}else return 0;}while(*p=='*')p++;return !*p;}
/* Borrow validated invocation arguments; every predicate is conjunctive.
   No retained matching state or separately allocated expression tree. */
static int selected(const char *path,const char *base,int directory,int link){
 for(int i=0;i<predicate_count;i++){
  const char *op=predicates[i];if(!strcmp(op,"-print"))continue;
  const char *value=predicates[++i];
  if(!strcmp(op,"-name")&&!match(value,base))return 0;
  if(!strcmp(op,"-path")&&!match(value,path))return 0;
  if(!strcmp(op,"-type")&&*value!=(link?'l':directory?'d':'f'))return 0;
 }
 return 1;
}
/* Iterative depth-first traversal: one reclaimable frame per active directory.
   No recursion, directory-wide result array, or arbitrary depth ceiling. */
typedef struct WalkFrame {struct WalkFrame *parent;char path[PATH_CAP],native[PATH_CAP];unsigned long long size;int depth,index,directory;} WalkFrame;
static WalkFrame *walk_frame(const char *path,int depth,WalkFrame *parent,int du,int *status){
 WalkFrame *f=cli_alloc(sizeof *f);CInfoPBRec pb;char canonical[PATH_CAP];
 if(!f){error(path,memFullErr);*status=1;return NULL;}
 memset(f,0,sizeof *f);f->parent=parent;f->depth=depth;f->index=1;
 if(strlen(path)>=PATH_CAP){error(path,bdNamErr);*status=1;cli_free(f);return NULL;}
 strcpy(f->path,path);int e=resolve_ex(path,f->native,canonical,0);if(!e)e=info(f->native,&pb);
 if(e){error(path,e);*status=1;cli_free(f);return NULL;}
 f->directory=isdir(&pb);int link=is_link(&pb);
 f->size=f->directory?0:(unsigned long)pb.hFileInfo.ioFlPyLen+(unsigned long long)(unsigned long)pb.hFileInfo.ioFlRPyLen;
 const char *base=strrchr(canonical,'/');base=base&&base[1]?base+1:canonical;
 if(!du&&selected(path,base,f->directory,link)){say(path);say("\n");}
 return f;
}
static int walk(const char *path,int depth,int du,unsigned long long *total){
 int status=0;*total=0;WalkFrame *f=walk_frame(path,depth,NULL,du,&status);
 while(f){
  if(app_yield()){status=130;break;}if(output_error){status=1;break;}
  if(f->directory&&f->depth<maxdepth){
   /* The native catalog index is signed 16-bit; fail rather than wrap. */
   if(f->index>32767){error(f->path,paramErr);status=1;break;}
   CInfoPBRec pb;char leaf[32],enc[100],child[PATH_CAP];int e=cli_native_nth(f->native,f->index,leaf,&pb);
   if(!e){
    f->index++;e=encode_name(leaf,enc,sizeof enc);if(!e)e=join_path(f->path,enc,child);
    if(e){error(f->path,e);status=1;}else{WalkFrame *next=walk_frame(child,f->depth+1,f,du,&status);if(next){f=next;continue;}}
   }else if(e!=fnfErr){error(f->path,e);status=1;}
   else goto finish;
   if(!e)continue;
  }
finish:
  if(du&&((f->directory&&(!summarize||!f->depth))||all||!f->depth)){char b[64];if(human)format_size(b,f->size,1);else snprintf(b,sizeof b,"%lu",(unsigned long)((f->size+unit-1)/unit));say(b);say("\t");say(f->path);say("\n");}
  WalkFrame *parent=f->parent;if(parent)parent->size+=f->size;else *total=f->size;cli_free(f);f=parent;
 }
 while(f){WalkFrame *parent=f->parent;cli_free(f);f=parent;}
 return output_error?1:status;
}
static void code4(unsigned long x,char *p){for(int i=0;i<4;i++){unsigned char c=x>>(24-8*i);p[i]=c>=32&&c<127?c:'.';}p[4]=0;}
static int describe(const char *path){char native[PATH_CAP],canonical[PATH_CAP],target[PATH_CAP],type[5],creator[5],buf[160];CInfoPBRec pb;int e=resolve_ex(path,native,canonical,0);if(!e)e=info(native,&pb);if(e){error(path,e);return 1;}if(!brief){say(path);say(": ");}
 if(isdir(&pb))say("HFS directory\n");else if(is_link(&pb)){e=link_target(native,target);if(e)return 1;say("Clarice symbolic link to ");say(target);say("\n");}else{code4(pb.hFileInfo.ioFlFndrInfo.fdType,type);code4(pb.hFileInfo.ioFlFndrInfo.fdCreator,creator);snprintf(buf,sizeof buf,"HFS file, type '%s', creator '%s', data %lu bytes, resource %lu bytes\n",type,creator,(unsigned long)pb.hFileInfo.ioFlLgLen,(unsigned long)pb.hFileInfo.ioFlRLgLen);say(buf);}return output_error?1:0;
}
static void usage_text(int fd,const char *s){cli_write(fd,s,strlen(s));}
static void usage(const char *name,int fd){if(!strcmp(name,"find"))usage_text(fd,"find [path ...] [-name pattern] [-path pattern] [-type f|d|l] [-maxdepth\nN] [-print]\nWalk HFS directories without following symbolic links. Default path: .\n-name matches encoded basenames; -path matches the printed pathname.\nPatterns support * and ?; quote them. All predicates must match.\nPredicates combine with AND. [], -o, -exec and other expressions are\nunsupported.\nTraversal grows with available memory; -maxdepth explicitly limits it.\n");else if(!strcmp(name,"du"))usage_text(fd,"du [-ashk] [--] [path ...]\nAllocated data+resource fork space; default units 512 bytes, -k 1024, -h\nreadable.\n-a includes files, -s gives argument totals. Links are not followed.\nHFS catalog/directory overhead is not assigned to individual folders.\nTraversal grows with available memory and native path capacity.\n");else usage_text(fd,"file [-b] [--] file ...\nDescribe HFS kind, symbolic target, type/creator and both fork sizes.\n-b omits the name. This is native metadata inspection, not libmagic\ncontent detection.\n");usage_text(fd,"Status: 0 success, 1 file error, 2 usage, 130 interrupted.\n");}
int clarice_main(int argc,char **av){const char *name=strrchr(av[0],'/');name=name?name+1:av[0];if(argc==2&&!strcmp(av[1],"--help")){usage(name,1);return 0;}int i=1,status=0;
 if(!strcmp(name,"find")){
  int first=1;while(i<argc&&av[i][0]!='-')i++;
  int roots=i-first;predicates=av+i;predicate_count=argc-i;
  /* Validate the complete expression before producing output. */
  for(;i<argc;i++){
   if(!strcmp(av[i],"-print"))continue;
   if(i+1>=argc){usage(name,2);return 2;}
   if(!strcmp(av[i],"-name")||!strcmp(av[i],"-path")){
    const char *value=av[++i];if(strchr(value,'[')||strchr(value,']')){usage(name,2);return 2;}
   }else if(!strcmp(av[i],"-type")){
    const char *value=av[++i];if(strlen(value)!=1||!strchr("fdl",*value)){usage(name,2);return 2;}
   }else if(!strcmp(av[i],"-maxdepth")){
    char *end;errno=0;long n=strtol(av[++i],&end,10);
    if(errno||*end||!*av[i]||n<0||n>2147483647L){usage(name,2);return 2;}maxdepth=n;
   }else{usage(name,2);return 2;}
  }
  for(i=0;i<(roots?roots:1);i++){
   unsigned long long n;int rc=walk(roots?av[first+i]:".",0,0,&n);
   if(rc==130)return rc;if(rc)status=rc;if(output_error)break;
  }
  return status;
 }

 int du=!strcmp(name,"du");for(;i<argc&&av[i][0]=='-'&&av[i][1];i++){if(!strcmp(av[i],"--")){i++;break;}for(int j=1;av[i][j];j++){char c=av[i][j];if(du&&c=='a')all=1;else if(du&&c=='s')summarize=1;else if(du&&c=='h')human=1;else if(du&&c=='k')unit=1024;else if(!du&&c=='b')brief=1;else{usage(name,2);return 2;}}}
 if(du&&all&&summarize){usage(name,2);return 2;}if(i==argc){if(du){unsigned long long n;return walk(".",0,1,&n);}usage(name,2);return 2;}
 for(;i<argc;i++){unsigned long long n;int rc=du?walk(av[i],0,1,&n):describe(av[i]);if(rc==130)return rc;if(rc)status=rc;if(app_yield())return 130;}return status;
}
