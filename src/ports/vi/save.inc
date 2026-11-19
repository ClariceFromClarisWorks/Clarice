/* Descriptor-based safe HFS replacement for ex/vi. */
static void *save_token;
static int save_fd=-1,save_failed;
static void vi_save_written(int fd,long got,size_t wanted){if(fd==save_fd&&got!=(long)wanted)save_failed=1;}
int vi_save_create(const char *path){char stage[512];if(save_token){errno=EBUSY;return -1;}save_token=cli_replace_begin(path,stage,sizeof stage);if(!save_token)return -1;save_fd=port_open(stage,O_WRONLY|O_CREAT|O_TRUNC);save_failed=0;if(save_fd<0){cli_replace_abort(save_token);save_token=0;}return save_fd;}
int vi_save_close(int fd){int rc=port_close(fd);if(fd==save_fd){void *t=save_token;save_token=0;save_fd=-1;if(rc||save_failed){cli_replace_abort(t);errno=EIO;return -1;}return cli_replace_commit(t);}return rc;}
int vi_save_abort_close(int fd){int rc=port_close(fd);if(fd==save_fd){cli_replace_abort(save_token);save_token=0;save_fd=-1;}return rc;}
