/* Explicit document replacement, not altered O_TRUNC semantics. One active save
   per synchronous invocation. Transaction state is allocated only while saving. */
typedef struct SafeSave {char dest[PATH_CAP],stage[PATH_CAP],old[PATH_CAP];CInfoPBRec original;int exists,staged,moved,installed;} SafeSave;
static SafeSave *active_save;
static int save_flush(const char *path){short v;int e=cli_native_volume(path,&v);return e?e:FlushVol(0,v);}
static void save_report(const char *path){errsay("save recovery file: native:");errsay(path);errsay("\n");}
static int save_absent(const char *path){CInfoPBRec p;int e=info(path,&p);return e==fnfErr?0:e?e:dupFNErr;}
static void cli_replace_abort(void *token){
 SafeSave *s=token;if(!s||s!=active_save)return;int saved=errno;
 if(s->moved){save_report(s->old);if(s->staged)save_report(s->stage);}
 else if(s->staged&&cli_native_delete(s->stage))save_report(s->stage);
 active_save=0;cli_free(s);errno=saved;
}
static void *cli_replace_begin(const char *path,char *target,unsigned long capacity){
 int e=0;short in=-1,out=-1;CInfoPBRec meta;
 if(active_save){errno=EBUSY;return 0;}
 SafeSave *s=cli_alloc(sizeof *s);if(!s)return 0;memset(s,0,sizeof *s);active_save=s;
 e=resolve(path,s->dest);if(e)goto fail;
 if(strlen(s->dest)>255){e=bdNamErr;goto fail;}
 e=info(s->dest,&s->original);if(!e){s->exists=1;if(s->original.hFileInfo.ioFlAttrib&17){e=permErr;goto fail;}}
 else if(e==fnfErr)e=0;else goto fail;
 /* Deterministic siblings make an interrupted save visible on the next attempt.
    Hash collisions only block a save; never replace existing recovery files. */
 unsigned long hash=2166136261UL;const unsigned char *p=(const unsigned char*)s->dest;
 while(*p){unsigned char c=*p++;if(c>='A'&&c<='Z')c+=32;hash=((hash^c)*16777619UL)&0xffffffffUL;}
 strcpy(s->stage,s->dest);char *leaf=strrchr(s->stage,':');if(!leaf){e=bdNamErr;goto fail;}
 if((size_t)(leaf-s->stage)+28>255){e=bdNamErr;goto fail;}
 snprintf(leaf+1,28,".clarice-%08lx.new",hash);strcpy(s->old,s->stage);strcpy(s->old+strlen(s->old)-3,"old");
 if(strlen(s->stage)+8>capacity){e=bdNamErr;goto fail;}
 if((e=save_absent(s->stage))||(e=save_absent(s->old))){save_report(s->stage);save_report(s->old);goto fail;}
 /* Check write permission/locked volume without truncating the source. */
 if(s->exists){e=cli_native_open(s->dest,0,1,&in);if(e)goto fail;e=cli_native_close(in);in=-1;if(e)goto fail;}
 e=cli_native_create(s->stage,'TEXT','CLRC');if(e)goto fail;s->staged=1;
 if(s->exists){
  e=cli_native_open(s->dest,1,0,&in);if(e)goto fail;
  e=cli_native_open(s->stage,1,1,&out);if(e)goto fail;
  char buffer[4096];for(;;){long n=sizeof buffer;e=cli_native_read(in,buffer,&n);int end=e==eofErr;if(e&&!end)break;e=0;
   if(n){long wanted=n;e=cli_native_write(out,buffer,&n);if(!e&&n!=wanted)e=ioErr;}
   if(e||end||!n)break;if(app_yield()){e=userCanceledErr;break;}
  }
  {int c=cli_native_close(in);in=-1;if(!e)e=c;c=cli_native_close(out);out=-1;if(!e)e=c;}if(e)goto fail;
  e=info(s->stage,&meta);if(e)goto fail;
  meta.hFileInfo.ioFlFndrInfo=s->original.hFileInfo.ioFlFndrInfo;
  meta.hFileInfo.ioFlXFndrInfo=s->original.hFileInfo.ioFlXFndrInfo;
  meta.hFileInfo.ioFlCrDat=s->original.hFileInfo.ioFlCrDat;
  e=cli_native_set(s->stage,&meta);if(e)goto fail;
 }
 strcpy(target,"native:");strcat(target,s->stage);return s;
fail:
 if(in>=0)cli_native_close(in);if(out>=0)cli_native_close(out);cli_replace_abort(s);port_errno(e);return 0;
}
/* Caller must close its writer successfully before committing. No event yields
   between rename/rollback operations. Native HFS rename never replaces a target. */
static int cli_replace_commit(void *token){
 SafeSave *s=token;CInfoPBRec now;int e=0;
 if(!s||s!=active_save){errno=EINVAL;return -1;}
 if(app_yield()){e=userCanceledErr;goto done;}
 if(s->exists){e=info(s->dest,&now);if(e)goto done;
  if(now.hFileInfo.ioDirID!=s->original.hFileInfo.ioDirID||now.hFileInfo.ioFlMdDat!=s->original.hFileInfo.ioFlMdDat||now.hFileInfo.ioFlLgLen!=s->original.hFileInfo.ioFlLgLen||now.hFileInfo.ioFlRLgLen!=s->original.hFileInfo.ioFlRLgLen){e=permErr;goto done;}
 }else if((e=save_absent(s->dest)))goto done;
 if((e=save_flush(s->stage)))goto done;
 if(s->exists){e=cli_native_rename(s->dest,s->old);if(e)goto done;s->moved=1;
  e=save_flush(s->old);if(e)goto rollback;}
 e=cli_native_rename(s->stage,s->dest);if(e)goto rollback;s->staged=0;s->installed=1;
 e=save_flush(s->dest);if(e)goto done;
 if(s->moved){e=cli_native_delete(s->old);if(e)goto done;s->moved=0;if(save_flush(s->dest))errsay("save: replacement saved; recovery-file cleanup flush failed\n");}
 goto done;
rollback:
 if(s->moved){int r=cli_native_rename(s->old,s->dest);if(!r){s->moved=0;r=save_flush(s->dest);}if(r)save_report(s->moved?s->old:s->dest);}
done:
 if(e&&s->installed)save_report(s->dest);
 cli_replace_abort(s);return e?port_errno(e):0;
}
