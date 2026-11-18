/* The updater is part of this application, loaded only for an update command.
 * Reuse the CLX validator/relocator directly on a locked native resource: no
 * installed updater dependency, extraction file, second image or idle cache.
 * Keep the resource clean; ReleaseResource discards the relocated in-memory copy. */
static int self_update_command(int argc,char **argv){
 if(session_io&&!(argc==2&&!strcmp(argv[1],"--help"))){
  errsay("clarice update currently requires local input ownership\n");return 2;
 }
 short prior=CurResFile();UseResFile(self_update_resources);
 Handle payload=Get1Resource('CLXU',128);UseResFile(prior);
 if(!payload){errsay("clarice: built-in updater resource is missing\n");return 126;}
 HLock(payload);long size=GetHandleSize(payload);int result=126;
 if(*payload)result=module_load_source("built-in clarice",argc,argv,1,(unsigned char*)*payload,size);
 else errsay("clarice: insufficient memory for built-in updater\n");
 HUnlock(payload);ReleaseResource(payload);return result;
}
