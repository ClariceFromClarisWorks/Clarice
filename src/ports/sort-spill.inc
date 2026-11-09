/* Clarice bounded external merge driver; upstream sbase owns comparison/options.
 * Binary merge levels prevent quadratic repeated merging of the entire input.
 * All names are runtime-owned and removed on normal return, error or Ctrl+C. */
#define SORT_LINES 128
#define SORT_BYTES 16384
#define SORT_LEVELS 16
static struct line pending[SORT_LINES];
static size_t pending_count,pending_bytes;
static const char *runs[SORT_LEVELS];
static void sort_write(FILE *fp,struct line *line){if(fwrite(line->data,1,line->len,fp)!=line->len)enprintf(2,"sort write:");}
static FILE *sort_open(const char *name,const char *mode){FILE *f=fopen(name,mode);if(!f)enprintf(2,"sort fopen %s:",name);return f;}
static void sort_close(FILE *fp){if(fclose(fp))enprintf(2,"sort fclose:");}
static ssize_t sort_read(FILE *fp,struct line *line,size_t *cap){ssize_t n=getline(&line->data,cap,fp);if(n<0&&ferror(fp))enprintf(2,"sort read:");line->len=n>0?n:0;return n;}
static const char *sort_merge(const char *a,const char *b){
 const char *name=port_temp_new();if(!name)enprintf(2,"sort temporary file:");
 FILE *fa=sort_open(a,"r"),*fb=sort_open(b,"r"),*out=sort_open(name,"w");
 struct line la={0},lb={0};size_t ca=0,cb=0;ssize_t na=sort_read(fa,&la,&ca),nb=sort_read(fb,&lb,&cb);
 while(na>0||nb>0){port_poll();if(nb<=0||(na>0&&slinecmp(&la,&lb)<=0)){sort_write(out,&la);na=sort_read(fa,&la,&ca);}else{sort_write(out,&lb);nb=sort_read(fb,&lb,&cb);}}
 free(la.data);free(lb.data);sort_close(fa);sort_close(fb);sort_close(out);port_temp_remove(a);port_temp_remove(b);return name;
}
static void sort_flush(void){
 if(!pending_count)return;
 qsort(pending,pending_count,sizeof *pending,(int (*)(const void *,const void *))slinecmp);
 const char *name=port_temp_new();if(!name)enprintf(2,"sort temporary file:");FILE *fp=sort_open(name,"w");
 for(size_t i=0;i<pending_count;i++){sort_write(fp,&pending[i]);free(pending[i].data);}sort_close(fp);pending_count=pending_bytes=0;
 for(int level=0;level<SORT_LEVELS;level++){if(!runs[level]){runs[level]=name;return;}name=sort_merge(runs[level],name);runs[level]=NULL;}
 enprintf(2,"sort: merge-level limit reached\n");
}
static void spill_add(FILE *fp){
 char *line=NULL;size_t cap=0;ssize_t n;
 while((n=getline(&line,&cap,fp))>0){
  size_t len=n;int newline=line[len-1]!='\n';if(len+newline>16384)enprintf(2,"sort: line exceeds 16384-byte limit\n");
  if(pending_count&&(pending_count==SORT_LINES||pending_bytes+len+newline>SORT_BYTES))sort_flush();
  struct line *entry=&pending[pending_count++];entry->data=emalloc(len+newline+1);memcpy(entry->data,line,len);if(newline)entry->data[len++]='\n';entry->data[len]=0;entry->len=len;pending_bytes+=len;
 }
 free(line);if(ferror(fp))enprintf(2,"sort read:");
}
static void spill_finish(FILE *out){
 int spilled=0;for(int i=0;i<SORT_LEVELS;i++)if(runs[i])spilled=1;
 if(!spilled){qsort(pending,pending_count,sizeof *pending,(int (*)(const void *,const void *))slinecmp);for(size_t i=0;i<pending_count;i++)if(!uflag||!i||slinecmp(&pending[i-1],&pending[i]))sort_write(out,&pending[i]);for(size_t i=0;i<pending_count;i++)free(pending[i].data);return;}
 sort_flush();const char *name=NULL;for(int i=0;i<SORT_LEVELS;i++)if(runs[i])name=name?sort_merge(name,runs[i]):runs[i];
 FILE *fp=sort_open(name,"r");struct line cur={0},prev={0};size_t cap=0,prev_cap=0;
 while(sort_read(fp,&cur,&cap)>0){if(!uflag||!prev.data||slinecmp(&prev,&cur)){sort_write(out,&cur);if(uflag){struct line tmp=prev;prev=cur;cur=tmp;size_t size=prev_cap;prev_cap=cap;cap=size;}}}
 free(cur.data);free(prev.data);sort_close(fp);port_temp_remove(name);
}
