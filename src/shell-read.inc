/* Grow record storage only when needed; keep escape provenance for IFS.
 * All destination values are prepared before changing any variable. */
static int sh_read(int n,char **v){
 char *record=0,*line=0;unsigned char *protected=0;size_t used=0,cap=0,start=0;
 int raw=n>1&&!strcmp(v[1],"-r"),var=raw?2:1,escaped=0,rc=0;unsigned polled=0;long count=0;char c;
 struct sh_env *pending=0,**tail=&pending;
 if(var<n&&!strcmp(v[var],"--"))var++;
 if(var>=n)return sh_fail("read [-r] requires variable names");
 for(int i=var;i<n;i++)if(!env_name(v[i],strlen(v[i])))return sh_fail("invalid read variable name");
 while((count=cli_read(0,&c,1))>0){
  if(!(++polled&127)&&app_yield()){rc=130;goto done;}
  if(!c){rc=sh_fail("read record contains NUL");goto done;}
  int quote=escaped;escaped=0;
  if(!raw&&!quote&&c=='\\'){escaped=1;continue;}
  if(c=='\n'||c=='\r'){if(quote)continue;break;}
  if(used+1>=cap){size_t next=cap?cap+cap/2:128;char *p;
   if(next<=cap||next>(size_t)-1/2){rc=sh_fail("read record too large");goto done;}
   p=malloc(next*2);if(!p){rc=sh_fail("out of memory reading record");goto done;}
   if(used){memcpy(p,line,used);memcpy(p+next,protected,used);}free(record);record=line=p;protected=(unsigned char*)p+next;cap=next;
  }
  line[used]=c;protected[used++]=quote;
 }
 if(count<0){rc=app_yield()?130:sh_fail("read failed");goto done;}
 if(line)line[used]=0;
 const char *ifs=sh_isset("IFS")?env_get("IFS"):" \t\n";
 #define READ_SEP(k) (!protected[k]&&strchr(ifs,line[k]))
 #define READ_WHITE(k) (READ_SEP(k)&&strchr(" \t\n",line[k]))
 while(start<used&&READ_WHITE(start))start++;
 while(used>start&&READ_WHITE(used-1))used--;
 for(int i=var;i<n;i++){
  size_t end=start,next;
  while(end<used&&!READ_SEP(end))end++;
  next=end;
  if(next<used){if(READ_WHITE(next)){while(next<used&&READ_WHITE(next))next++;if(next<used&&READ_SEP(next))next++;}else next++;while(next<used&&READ_WHITE(next))next++;}
  if(i==n-1&&next<used)end=used;
  char save=0;if(line){save=line[end];line[end]=0;}
  *tail=env_new(v[i],strlen(v[i]),line?line+start:"");
  if(line)line[end]=save;
  if(!*tail){rc=sh_fail("out of memory assigning read variables");goto done;}tail=&(*tail)->next;start=next;
 }
 while(pending){struct sh_env *e=pending;pending=e->next;env_put(&sh_env,e);}
 rc=count>0?0:1;
 done:env_free(pending);free(record);return rc;
 #undef READ_SEP
 #undef READ_WHITE
}
