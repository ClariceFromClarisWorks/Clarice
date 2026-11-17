/* Advisory verification receipts, not authentication. A missing, torn, stale or
   unreadable receipt always falls back to full validation. No resident cache. */
#define CLX_RECORD_BYTES 352
static int clx_record_path(const char *native,char *out){
 size_t n=strlen(root_path),total=strlen(native);
 if(n+4>=256||total>=256||total<n+4||strncmp(native,root_path,n)||strncmp(native+n,"bin:",4))return 0;
 const char *leaf=native+n+4;size_t len=strlen(leaf);if(!len||len>20)return 0;
 for(size_t i=0;i<len;i++)if(!isalnum((unsigned char)leaf[i])&&leaf[i]!='_'&&leaf[i]!='-')return 0;
 int count=snprintf(out,PATH_CAP,"%setc:clarice:clx-%s.ok",root_path,leaf);return count>0&&count<256&&count<PATH_CAP;
}
#ifndef CLX_HOST_TEST
static int clx_identity(short ref,const char *native,unsigned char out[32]){
 FCBPBRec f;CInfoPBRec c;HParamBlockRec v;memset(&f,0,sizeof f);f.ioRefNum=ref;
 if(PBGetFCBInfoSync(&f)||info(native,&c))return 0;
 short volume;if(cli_native_volume(native,&volume)||volume!=f.ioFCBVRefNum||c.hFileInfo.ioDirID!=f.ioFCBFlNm||c.hFileInfo.ioFlParID!=f.ioFCBParID||c.hFileInfo.ioFlLgLen!=f.ioFCBEOF)return 0;
 memset(&v,0,sizeof v);v.volumeParam.ioVRefNum=volume;if(PBHGetVInfoSync(&v))return 0;
 clx_put32(out,v.volumeParam.ioVCrDate);clx_put32(out+4,f.ioFCBFlNm);clx_put32(out+8,f.ioFCBParID);
 clx_put32(out+12,c.hFileInfo.ioFlCrDat);clx_put32(out+16,c.hFileInfo.ioFlMdDat);
 clx_put32(out+20,c.hFileInfo.ioFlLgLen);clx_put32(out+24,c.hFileInfo.ioFlRLgLen);clx_put32(out+28,c.hFileInfo.ioFlAttrib&1);return 1;
}
#endif
static int clx_receipt_read(const char *path,unsigned char b[CLX_RECORD_BYTES]){
 unsigned char p[256];short ref;pstr(p,path);if(HOpen(0,0,p,fsRdPerm,&ref))return 0;
 long len=0,n=CLX_RECORD_BYTES;int e=GetEOF(ref,&len);if(!e&&len==n)e=FSRead(ref,&n,b);else e=ioErr;
 int close=FSClose(ref);return !e&&!close&&n==CLX_RECORD_BYTES&&!memcmp(b,"CLXV0001",8)&&clx_u32(b+348)==clx_hash(2166136261UL,b,348);
}
static int clx_receipt_forget(const char *native){
 char path[PATH_CAP];unsigned char p[256],b[CLX_RECORD_BYTES];short ref;
 if(!clx_record_path(native,path))return 0;pstr(p,path);
 int e=HOpen(0,0,p,fsRdPerm,&ref);if(e)return e==fnfErr?0:e;
 long len=0,n=sizeof b;e=GetEOF(ref,&len);if(!e&&len==(long)sizeof b)e=FSRead(ref,&n,b);
 int closed=FSClose(ref);if(e||closed)return e?e:closed;
 /* Never delete an unrelated file occupying our reserved record name. */
 if(len!=(long)sizeof b||n!=(long)sizeof b||memcmp(b,"CLXV0001",8)||!memchr(b+88,0,256)||strcmp((char*)b+88,native))return 0;
 return HDelete(0,0,p);
}
static void clx_receipt_write(const char *path,const unsigned char b[CLX_RECORD_BYTES]){
 unsigned char pn[256];short ref=-1;CInfoPBRec pb;pstr(pn,path);int e=info(path,&pb);
 if(e==fnfErr)e=HCreate(0,0,pn,'CLRC','DATA');
 else if(!e){unsigned char old[CLX_RECORD_BYTES];if(!clx_receipt_read(path,old))return;}
 if(!e)e=HOpen(0,0,pn,fsWrPerm,&ref);
 if(!e){e=SetEOF(ref,0);long n=CLX_RECORD_BYTES;if(!e)e=FSWrite(ref,&n,b);if(!e&&n!=CLX_RECORD_BYTES)e=ioErr;int closed=FSClose(ref);if(!e)e=closed;}
 if(!e){short v;if(!cli_native_volume(path,&v))FlushVol(0,v);}
 /* Receipt failure cannot invalidate a fully checked command. Next launch will
    perform full checking again. Partial records fail their size/checksum. */
}
