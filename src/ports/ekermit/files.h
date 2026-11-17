/* One explicitly named file; incoming names never select a destination. */
static char native_path[512],temporary[512];static int owned,completed,opened;
#ifdef KERMIT_HOST
static int file_fd=-1;
static int file_prepare(const char *path,int sending){struct stat s;
 if(strlen(path)>400)return -1;strcpy(native_path,path);
 if(sending)return stat(path,&s)||!S_ISREG(s.st_mode)?-1:0;
 if(!lstat(path,&s)||errno!=ENOENT)return -1;
 snprintf(temporary,sizeof temporary,"%s.clarice-XXXXXX",path);file_fd=mkstemp(temporary);if(file_fd<0)return -1;owned=1;return 0;
}
static int file_open(int sending){if(sending)file_fd=open(native_path,O_RDONLY);return file_fd<0?-1:0;}
static int file_read(void *p,int n){return read(file_fd,p,n);}
static int file_write(const void *p,int n){return write(file_fd,p,n)==n?0:-1;}
static int file_close(void){int e=0;if(file_fd>=0){e=close(file_fd);file_fd=-1;}return e;}
static int file_finish(int ok){int e=file_close();if(ok&&!e){/* link is atomic and refuses an existing destination. */if(link(temporary,native_path))e=-1;}if(owned){if(unlink(temporary))e=-1;owned=0;}return e;}
#else
static short file_fd=-1;
static int file_prepare(const char *path,int sending){CInfoPBRec pb;char canonical[PATH_CAP];int e;
 e=resolve_ex(path,native_path,canonical,sending);if(e)return e;
 if(!sending&&protected_path(canonical))return paramErr;
 e=info(native_path,&pb);if(sending)return e?e:isdir(&pb)?paramErr:0;
 if(e!=fnfErr)return e?e:dupFNErr;
 strcpy(temporary,native_path);char *leaf=strrchr(temporary,':');if(!leaf)return bdNamErr;
 for(int i=0;i<32;i++){snprintf(leaf+1,sizeof temporary-(leaf+1-temporary),"Clv kermit %08lx %02x",ticks(),i);e=cli_native_create(temporary,0,0);if(!e){owned=1;return 0;}if(e!=dupFNErr)return e;}return dupFNErr;
}
static int file_open(int sending){return cli_native_open(sending?native_path:temporary,0,!sending,&file_fd);}
static int file_read(void *p,int n){long got=n;int e=cli_native_read(file_fd,p,&got);return e&&e!=eofErr?-1:(int)got;}
static int file_write(const void *p,int n){long put=n;int e=cli_native_write(file_fd,p,&put);return e||put!=n?-1:0;}
static int file_close(void){int e=0;if(file_fd>=0){e=cli_native_close(file_fd);file_fd=-1;}return e;}
static int file_finish(int ok){int e=file_close();if(ok&&!e)e=cli_native_rename(temporary,native_path);if(owned){if(!ok||e){int c=cli_native_delete(temporary);if(!e)e=c;}owned=0;}return e;}
#endif
