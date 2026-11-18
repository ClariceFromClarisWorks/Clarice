#include "libbb.h"
#include "bb_archive.h"
int addition_input_fd;
static int test_only;
int addition_pack(char **v,int (*fn)(transformer_state_t *)){
 if(*v && !(option_mask32&1) && !test_only)bb_error_msg_and_die("use -c FILE to write stdout; in-place mode unavailable");
 int status=0;
 do {transformer_state_t state;memset(&state,0,sizeof(state));
  addition_input_fd=*v?open_or_warn_stdin(*v):0;
  if(addition_input_fd<0)status=1;
  else {state.src_fd=addition_input_fd;state.dst_fd=1;if(fn(&state)<0)status=1;if(addition_input_fd && close(addition_input_fd))status=1;}
 }while(*v && *++v);
 return status;
}
ssize_t transformer_write(transformer_state_t *s,const void *b,size_t n){return test_only?(ssize_t)n:full_write(s->dst_fd,b,n);}
int check_signature16(transformer_state_t *s,unsigned magic){unsigned char b[2];if(s->signature_skipped)return 0;if(full_read(s->src_fd,b,2)!=2 || ((unsigned)b[0]<<8|b[1])!=magic){bb_simple_error_msg("invalid gzip signature");return -1;}return 0;}
int gunzip_main(int argc,char **argv){(void)argc;unsigned opt=getopt32(argv,"ct");test_only=!!(opt&2);return addition_pack(argv+optind,unpack_gz_stream);}
