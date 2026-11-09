/* Native HFS primitives shared by independently loaded utilities. */
#include "../sys6.h"
#include <Files.h>
#include <Errors.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
extern int resolve_ex(const char *,char *,char *,int),resolve(const char *,char *);
extern int info(const char *,CInfoPBRec *),is_link(const CInfoPBRec *);
extern int link_target(const char *,char *),protected_path(const char *);
extern int encode_name(const char *,char *,size_t),same_name(const char *,const char *);
extern void say(const char *),errsay(const char *),error(const char *,int),emit(const char *,long);
extern int output_error;
extern unsigned long cli_ticks(void),cli_mac_time(void);
extern int cli_native_create(const char *,unsigned long,unsigned long);
extern int cli_native_delete(const char *),cli_native_mkdir(const char *);
extern int cli_native_open(const char *,int,int,short *);
extern int cli_native_close(short),cli_native_read(short,void *,long *),cli_native_write(short,const void *,long *);
extern int cli_native_set(const char *,CInfoPBRec *),cli_native_lock(const char *,int);
extern int cli_native_nth(const char *,int,char *,CInfoPBRec *);
extern int cli_native_rename(const char *,const char *),cli_native_move(const char *,const char *);
static int isdir(const CInfoPBRec *pb){return !!(pb->hFileInfo.ioFlAttrib&16);}
static int join_path(const char *parent,const char *leaf,char *out){size_t n=strlen(parent),m=strlen(leaf);if(n+m+2>=PATH_CAP)return bdNamErr;strcpy(out,parent);if(n&&out[n-1]!='/')strcat(out,"/");strcat(out,leaf);return 0;}
static int checked(const char *path,char *native,char *canonical,int follow){int e=resolve_ex(path,native,canonical,follow);if(!e&&protected_path(canonical))e=paramErr;return e;}
