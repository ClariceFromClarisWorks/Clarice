/* Substitution output has one writer and starts empty. Keep its length locally,
 * rather than rescanning the growing destination and the unused source suffix.
 * No persistent cache or change to upstream String layout. */
static void substitution_append(String *dst,size_t *used,const char *src,size_t n){
 size_t take=0;while(take<n&&src[take])take++;
 if(take>SIZE_MAX-*used-1)enprintf(2,"sed: substitution size overflow\n");
 size_t need=*used+take+1;
 if(dst->cap<need){size_t cap=dst->cap<=SIZE_MAX/2?dst->cap*2:need;if(cap<need)cap=need;resize((void **)&dst->str,&dst->cap,1,cap,NULL);}
 memcpy(dst->str+*used,src,take);*used+=take;dst->str[*used]=0;
}
