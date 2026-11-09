/* Bounded, non-executable preferences format; shared with host validation. */
#ifndef CLARICE_PREFERENCES_FORMAT_H
#define CLARICE_PREFERENCES_FORMAT_H
#define PREFS_TEXT_MAX 2048
struct ClaricePrefs { char font[256]; int size,inverse,paging,retained,renderer,width,height,x,y,fill; };
static int pref_uint(const char *s){int n=0;if(!*s)return -1;while(*s){if(*s<'0'||*s>'9'||n>10000)return -1;n=n*10+*s++-'0';}return n;}
static char *pref_trim(char *s){char *end;while(*s==' '||*s=='\t')s++;end=s+strlen(s);while(end>s&&(end[-1]==' '||end[-1]=='\t'))*--end=0;return s;}
/* Invalid values/unknown keys leave defaults unchanged. CR, LF and CRLF accepted.
   Reject a whole oversized or binary file before changing any settings. */
static int prefs_parse(struct ClaricePrefs *p,char *data,size_t len){
 if(len>PREFS_TEXT_MAX||memchr(data,0,len))return 0;data[len]=0;
 char *line=data;while(*line){char *next=line;while(*next&&*next!='\n'&&*next!='\r')next++;if(*next)*next++=0;
 char *key=pref_trim(line),*eq=strchr(key,'=');if(*key!='#'&&eq){*eq++=0;key=pref_trim(key);char *v=pref_trim(eq);int n=pref_uint(v);
 if(!strcmp(key,"font")){size_t k=strlen(v);if(k&&k<sizeof p->font)memcpy(p->font,v,k+1);}
 else if(!strcmp(key,"size")&&n>=6&&n<=24)p->size=n;
 else if(!strcmp(key,"colours")){if(!strcmp(v,"dark"))p->inverse=1;else if(!strcmp(v,"light"))p->inverse=0;}
 else if(!strcmp(key,"paging")){if(!strcmp(v,"off"))p->paging=0;else if(!strcmp(v,"screen"))p->paging=1;else if(!strcmp(v,"100"))p->paging=2;}
 else if(!strcmp(key,"scrollback")&&(n==32||n==64||n==128))p->retained=n;
 else if(!strcmp(key,"renderer")){if(!strcmp(v,"native"))p->renderer=0;else if(!strcmp(v,"c"))p->renderer=1;else if(!strcmp(v,"asm"))p->renderer=2;}
 else if(!strcmp(key,"width")&&n>=180&&n<=8192)p->width=n;
 else if(!strcmp(key,"height")&&n>=100&&n<=8192)p->height=n;
 else if(!strcmp(key,"x")&&n>=0&&n<=8192)p->x=n;
 else if(!strcmp(key,"y")&&n>=0&&n<=8192)p->y=n;
 else if(!strcmp(key,"fill")&&(n==0||n==1))p->fill=n;
 }line=next;}return 1;
}
static int prefs_format(char *out,size_t cap,const struct ClaricePrefs *p){return snprintf(out,cap,
 "# Clarice preferences; read at launch. Values are not shell commands.\n"
 "font=%s\nsize=%d\ncolours=%s\npaging=%s\nscrollback=%d\nrenderer=%s\n"
 "# Windowed content geometry; columns follow font and width.\n"
 "width=%d\nheight=%d\nx=%d\ny=%d\nfill=%d\n",p->font,p->size,p->inverse?"dark":"light",p->paging==0?"off":p->paging==1?"screen":"100",p->retained,p->renderer==0?"native":p->renderer==1?"c":"asm",p->width,p->height,p->x,p->y,p->fill);}
#endif
