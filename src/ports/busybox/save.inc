/* Clarice safe HFS transaction; host owns the recovery token. */
int bb_save_file(const char *path,const char *data,size_t n){char stage[512];save_token=cli_replace_begin(path,stage,sizeof stage);if(!save_token)return -1;save_fd=port_open(stage,O_WRONLY|O_CREAT|O_TRUNC);if(save_fd<0)goto fail;ssize_t got=full_write(save_fd,data,n);int rc=port_close(save_fd);save_fd=-1;if(got!=(ssize_t)n||rc)goto fail;void *t=save_token;save_token=NULL;if(cli_replace_commit(t))return -1;return n;
 fail:cli_replace_abort(save_token);save_token=NULL;errno=EIO;return -1;}
