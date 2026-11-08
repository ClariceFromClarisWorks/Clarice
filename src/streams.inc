/* Three standard descriptors. -1/-3 are terminal output/error; -2 is explicitly closed.
   Duplications share a Mac file reference and therefore its current offset. */
static long stream_write(short ref,const void *data,unsigned long count){
 if(ref==-2){output_error=1;return -1;}
 if(ref<0){
#ifdef SH_HOST_TEST
 fwrite(data,1,count,ref==-3?stderr:stdout);
#else
 term_write(data,count);
#endif
 return count;}
 unsigned long done=0;while(done<count){long n=count-done;int e=FSWrite(ref,&n,(const char*)data+done);done+=n;if(e||!n){output_error=1;return -1;}}return done;
}
static long cli_write(int fd,const void *data,unsigned long count){if(fd!=1&&fd!=2)return -1;return stream_write(fd==1?output_ref:error_ref,data,count);}
static void emit(const char *s,long n){if(n>0)cli_write(1,s,n);}
static void errsay(const char *s){cli_write(2,s,strlen(s));}
static void error(const char *s,int e){char b[128];snprintf(b,sizeof b,"%s (Mac OS error %d)\n",s,e);errsay(b);}

static long cli_read(int fd,void *data,unsigned long count){
 if(fd!=0||input_ref==-2)return -1;if(!count)return 0;
 if(input_ref<0){
#ifdef SH_HOST_TEST
 return fread(data,1,count,stdin);
#else
 return app_terminal_read(data,count);
#endif
 }
 long n=count;int e=FSRead(input_ref,&n,data);return e&&e!=eofErr?-1:n;
}
