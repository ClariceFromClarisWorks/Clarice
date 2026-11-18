/* HFS permits seeking only through EOF. Extend gaps only in owned temp files;
 * the existing runtime resize zero-fills extension and preserves cancellation. */
int vi_temp_seek(int fd,off_t offset,int writing){
 if(writing){struct stat st;if(port_fstat(fd,&st)<0)return -1;if(st.st_size<offset&&port_ftruncate(fd,offset)<0)return -1;}
 if(port_lseek(fd,offset,SEEK_SET)!=offset)return -1;
 return 0;
}
