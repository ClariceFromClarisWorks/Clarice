/* Best-effort emergency data-fork preservation, not a normal file save.
 * Invocation cleanup has disabled polling: no allocation or recursive exit here.
 * A .part file is never advertised as a complete recovery. */
extern int cli_file_open(const char *,const char *),cli_file_close(int);
extern long cli_file_write(int,const void *,unsigned long);
extern int cli_native_rename(const char *,const char *);
void bb_preserve(const char *original,const char *data,size_t n){
 char path[96],complete[96],native[512],target[512];
 unsigned long first=cli_ticks(),id=first;int ref;
 do {snprintf(path,sizeof path,"/etc/clarice/vi-%08lx.part",id++);
  ref=cli_file_open(path,"x");
  if(ref>=0||errno!=EEXIST)break;
 }while(id!=first);
 if(ref<0){fprintf(stderr,"vi: cannot preserve unsaved text from %s\n",original?original:"unnamed buffer");return;}
 size_t done=0;int failed=0;
 while(done<n){unsigned long k=n-done;if(k>4096)k=4096;long got=cli_file_write(ref,data+done,k);
  if(got<=0||got>(long)k){failed=1;break;}done+=got;}
 if(cli_file_close(ref))failed=1;
 strcpy(complete,path);strcpy(strrchr(complete,'.'),".recover");
 if(!failed&&(resolve_ex(path,native,0,1)||resolve_ex(complete,target,0,1)||cli_native_rename(native,target)))failed=1;
 fprintf(stderr,failed?"vi: incomplete recovery retained at %s\n":"vi: unsaved text preserved at %s\n",failed?path:complete);
}
