extern long cli_write(int,const void*,unsigned long);
static int count;
int clarice_main(int argc,char **argv){(void)argv;count++;cli_write(1,"module probe\n",13);return count==1?argc-1:99;}
