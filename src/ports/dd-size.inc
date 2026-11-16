/* Checked spelling of sbase's decimal/b/k/x grammar. Reject overflow before
   allocating or opening/truncating an output file. */
static size_t
parsesize(char *expr)
{
 const char *p=expr;size_t result=1;
 for(;;){char *end;unsigned long value,factor=1;
  if(*p<'0'||*p>'9')eprintf("invalid block size '%s'\n",expr);
  errno=0;value=strtoul(p,&end,10);
  if(errno||end==p||!value||value>2147483647UL)eprintf("invalid block size '%s'\n",expr);
  if(*end=='k'){factor=1024;end++;}else if(*end=='b'){factor=512;end++;}
  if(value>2147483647UL/factor)eprintf("block size overflow\n");value*=factor;
  if(result>2147483647UL/value)eprintf("block size overflow\n");result*=value;
  if(!*end)return result;if(*end!='x'||!end[1])eprintf("invalid block size '%s'\n",expr);p=end+1;
 }
}
