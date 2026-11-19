/* Clarice xvi safe-save boundary; licenses/xvi/COPYING. */
static void *replacement;
static FILE *save_stream;
FILE *xvi_save_open(const char *path,const char *mode){
 if(*mode=='a'){errno=ENOSYS;return NULL;} /* append requires separate semantics */
 char stage[512];replacement=cli_replace_begin(path,stage,sizeof stage);if(!replacement)return NULL;
 save_stream=port_fopen(stage,"w");if(!save_stream){cli_replace_abort(replacement);replacement=NULL;}return save_stream;
}
int xvi_save_close(FILE *f){int bad=ferror(f),e=errno,tracked=f==save_stream;int rc=port_fclose(f);if(rc){bad=1;e=errno;}
 if(!tracked)return rc;void *token=replacement;save_stream=NULL;replacement=NULL;
 if(bad){cli_replace_abort(token);errno=e?e:EIO;return EOF;}return cli_replace_commit(token);
}
