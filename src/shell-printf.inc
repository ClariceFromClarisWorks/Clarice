/* Portable integer/string printf, streamed through the existing shell output.
 * No allocation proportional to a user-supplied field width or precision. */
static int sh_print_poll(void){if(output_error)return 1;if(app_yield()){sh_flow=4;sh_flow_status=130;return 130;}return 0;}
static int sh_padding(size_t n,char c){char b[64];memset(b,c,sizeof b);while(n){size_t k=n<sizeof b?n:sizeof b;int rc=sh_print_poll();if(rc)return rc;emit(b,k);n-=k;}return output_error?1:0;}
/* Decode one escape; unknown escapes preserve the backslash. */
static unsigned char sh_escape(const char **next,int binary,int *stop){const char *p=*next;unsigned char c=*p;if(!c)return '\\';
 if(binary&&c=='c'){*stop=1;*next=p+1;return 0;}
 if(c>='0'&&c<='7'&&(!binary||c=='0')){unsigned value=0;int count=0;if(binary)p++;while(count<3&&*p>='0'&&*p<='7'){value=value*8+*p++-'0';count++;}*next=p;return (unsigned char)value;}
 const char *codes="abfnrtv\\",*q=strchr(codes,c);if(q){static const char bytes[]="\a\b\f\n\r\t\v\\";*next=p+1;return bytes[q-codes];}return '\\';}
static size_t sh_escape_length(const char *s,int *stop){size_t n=0;while(*s){char c=*s++;if(c=='\\')sh_escape(&s,1,stop);if(*stop)break;n++;}return n;}
static int sh_print_string(const char *s,size_t len,int binary){char b[64];size_t used=0;int stop=0;while(len--){unsigned char c=*s++;if(binary&&c=='\\')c=sh_escape(&s,1,&stop);if(stop)break;b[used++]=c;if(used==sizeof b){int rc=sh_print_poll();if(rc)return rc;emit(b,used);used=0;}}if(used)emit(b,used);return output_error?1:0;}
static int sh_format_count(const char **p,int *n){*n=0;while(isdigit((unsigned char)**p)){int digit=*(*p)++-'0';if(*n>(INT_MAX-digit)/10)return sh_fail("printf: field width or precision overflow");*n=*n*10+digit;}return 0;}
static int sh_printf(int n,char **v){int arg=1,rc=0,bad=0;if(arg<n&&!strcmp(v[arg],"--"))arg++;if(arg==n)return sh_fail("printf: format required");const char *format=v[arg++];
 do{int before=arg;const char *p=format;while(*p){if((rc=sh_print_poll()))return rc;unsigned char c=*p++;
  if(c=='\\'){int stop=0;c=sh_escape(&p,0,&stop);emit((char*)&c,1);continue;}if(c!='%'){emit((char*)&c,1);continue;}if(*p=='%'){say("%");p++;continue;}
  int left=0,plus=0,space=0,zero=0,alt=0,width=0,precision=-1;
  while(*p&&strchr("-+ 0#",*p)){switch(*p++){case '-':left=1;break;case '+':plus=1;break;case ' ':space=1;break;case '0':zero=1;break;case '#':alt=1;break;}}
  if((rc=sh_format_count(&p,&width)))return rc;
  if(*p=='.'){p++;if((rc=sh_format_count(&p,&precision)))return rc;}
  c=*p;if(!c||!strchr("sbcd iuoxX",c)||c==' ')return sh_fail("printf: unsupported conversion");p++;
  const char *s=arg<n?v[arg++]:"";char digits[3*sizeof(unsigned long)+1],prefix[3];size_t len=0,head=0,zpad=0;int stop=0;
  if(c=='s'||c=='b'||c=='c'){len=c=='c'?1:c=='b'?sh_escape_length(s,&stop):strlen(s);if(c!='c'&&precision>=0&&len>(size_t)precision)len=precision;
   size_t pad=(size_t)width>len?(size_t)width-len:0;if(!left&&(rc=sh_padding(pad,' ')))return rc;
   if(c=='c')emit(s,1);else if((rc=sh_print_string(s,len,c=='b')))return rc;
   if(stop)return output_error?1:0;if(left&&(rc=sh_padding(pad,' ')))return rc;continue;}
  unsigned long value;char *end;errno=0;int signed_value=c=='d'||c=='i';long signed_number=0;
  if(*s=='\''||*s=='"'){value=(unsigned char)s[1];signed_number=(long)value;}
  else if(!*s){value=0;}
  else {if(signed_value){signed_number=strtol(s,&end,0);value=(unsigned long)signed_number;}else value=strtoul(s,&end,0);if(end==s||*end||errno==ERANGE){errsay("printf: invalid or out-of-range number\n");bad=1;}}
  if(signed_value){if(signed_number<0){prefix[head++]='-';value=0UL-value;}else if(plus)prefix[head++]='+';else if(space)prefix[head++]=' ';}
  unsigned base=c=='o'?8:(c=='x'||c=='X')?16:10;const char *alphabet=c=='X'?"0123456789ABCDEF":"0123456789abcdef";unsigned long remaining=value;
  do{digits[len++]=alphabet[remaining%base];remaining/=base;}while(remaining);
  for(size_t i=0;i<len/2;i++){char t=digits[i];digits[i]=digits[len-i-1];digits[len-i-1]=t;}
  if(!value&&precision==0)len=0;
  if(precision>0&&(size_t)precision>len)zpad=(size_t)precision-len;
  if(alt&&base==8&&(!len||digits[0]!='0')&&!zpad)zpad=1;
  if(alt&&base==16&&value){prefix[head++]='0';prefix[head++]=c;}
  size_t total=head+zpad+len,pad=(size_t)width>total?(size_t)width-total:0;
  if(zero&&!left&&precision<0){zpad+=pad;pad=0;}
  if(!left&&(rc=sh_padding(pad,' ')))return rc;if(head)emit(prefix,head);if((rc=sh_padding(zpad,'0')))return rc;if(len)emit(digits,len);if(left&&(rc=sh_padding(pad,' ')))return rc;
 }if(arg==before)break;}while(arg<n);return output_error||bad?1:0;}
