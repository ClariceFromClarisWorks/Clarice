/* System-file 'vers' numeric version, not an application's resource chain. */
#ifndef CLARICE_SYSTEM_VERSION_H
#define CLARICE_SYSTEM_VERSION_H
static unsigned clarice_system_version(const unsigned char *bytes,long length){
 if(!bytes||length<6)return 0;
 for(int i=0;i<2;i++)if((bytes[i]>>4)>9||(bytes[i]&15)>9)return 0;
 return ((unsigned)bytes[0]<<8)|bytes[1];
}
#endif
