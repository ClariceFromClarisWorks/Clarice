/* Policy and Macintosh I/O adapter around genuine picohttpparser.
   Parse only a complete header block, then release all header storage. */
#include "vendor/picohttpparser/picohttpparser.h"
static int field_equal(const char *s,size_t n,const char *expected){
 return strlen(expected)==n&&!strncasecmp(s,expected,n);
}
static int field_number(const char *s,size_t n,unsigned long *out){
 unsigned long value=0;if(!n)return 0;
 for(size_t i=0;i<n;i++){
  if(s[i]<'0'||s[i]>'9'||value>214748364UL||(value==214748364UL&&s[i]>'7'))return 0;
  value=value*10+s[i]-'0';
 }
 *out=value;return 1;
}
static int response_headers(unsigned long *size,int *chunked,char *status_text,
                            size_t status_cap,const char **why){
 char *text=NULL;struct phr_header *headers=NULL;
 size_t length=0,capacity=512,lines=0;int ok=0,previous=0;
 text=cli_alloc(capacity);if(!text){*why="out of memory reading HTTP headers";goto done;}
 for(;;){
  int ready=poll_input();if(ready<0)goto done;if(!ready)continue;
  unsigned char c=buffer[used++];
  /* Keep the previous strict CRLF policy even though upstream accepts LF. */
  if((c=='\n'&&previous!='\r')||(previous=='\r'&&c!='\n'))goto done;
  if(length==capacity){
   if(capacity==8192){*why="HTTP headers exceed 8192 bytes";goto done;}
   char *next=cli_alloc(capacity*2);if(!next){*why="out of memory reading HTTP headers";goto done;}
   memcpy(next,text,length);cli_free(text);text=next;capacity*=2;
  }
  text[length++]=c;previous=c;if(c=='\n')lines++;
  if(length>=4&&!memcmp(text+length-4,"\r\n\r\n",4))break;
 }
 /* Each actual line needs at most one header descriptor; no fixed field count. */
 headers=cli_alloc(lines*sizeof(*headers));
 if(!headers){*why="out of memory parsing HTTP headers";goto done;}
 size_t count=lines,msg_length;const char *msg;int minor,status;
 int consumed=phr_parse_response(text,length,&minor,&status,&msg,&msg_length,headers,&count,0);
 if(consumed!=(int)length||minor>1||status<100||status>599)goto done;
 snprintf(status_text,status_cap,"HTTP %d %.*s",status,(int)(msg_length>100?100:msg_length),msg);
 if(status!=200){*why=status_text;goto done;}
 int have_length=0,have_transfer=0,have_encoding=0;
 for(size_t i=0;i<count;i++){
  struct phr_header *h=headers+i;
  if(!h->name){*why="obsolete folded HTTP header unsupported";goto done;}
  if(field_equal(h->name,h->name_len,"Content-Length")){
   if(have_length||!field_number(h->value,h->value_len,size))goto done;have_length=1;
  }else if(field_equal(h->name,h->name_len,"Transfer-Encoding")){
   if(have_transfer||!field_equal(h->value,h->value_len,"chunked")){*why="unsupported or duplicate transfer encoding";goto done;}
   have_transfer=1;
  }else if(field_equal(h->name,h->name_len,"Content-Encoding")){
   if(have_encoding||!field_equal(h->value,h->value_len,"identity")){*why="unsupported or duplicate content encoding";goto done;}
   have_encoding=1;
  }
 }
 if(have_transfer&&have_length){*why="ambiguous HTTP length and transfer encoding";goto done;}
 if(!have_transfer&&!have_length){*why="Content-Length or chunked encoding required";goto done;}
 if(have_transfer&&minor!=1){*why="chunked encoding requires HTTP/1.1";goto done;}
 *chunked=have_transfer;ok=1;
done:
 if(headers)cli_free(headers);if(text)cli_free(text);return ok;
}
