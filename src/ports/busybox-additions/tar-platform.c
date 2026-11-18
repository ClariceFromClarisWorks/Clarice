#include "libbb.h"
#include "bb_archive.h"
int bb_got_signal;
int xopen(const char *p,int flags){int fd=open(p,flags);if(fd<0)bb_simple_perror_msg_and_die(p);return fd;}
void xread(int fd,void *b,size_t n){if(full_read(fd,b,n)!=(ssize_t)n)bb_simple_error_msg_and_die("short read");}
void seek_by_jump(int fd,off_t n){seek_by_read(fd,n);}
const llist_t *find_list_entry(const llist_t *l,const char *s){for(;l;l=l->link)if(!strcmp(l->data,s))return l;return NULL;}
char filter_accept_reject_list(archive_handle_t *h){return (!h->accept||find_list_entry(h->accept,h->file_header->name))&&!find_list_entry(h->reject,h->file_header->name)?EXIT_SUCCESS:EXIT_FAILURE;}
