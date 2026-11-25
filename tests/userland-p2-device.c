extern int cli_isatty(int);
int clarice_main(int argc,char **argv){int expected=argc>1&&argv[1][0]=='1';for(int fd=0;fd<3;fd++)if(!!cli_isatty(fd)!=expected)return 20+fd;return 0;}
