#include <Multiverse.h>
#include <string.h>
#include "hardware.h"
#include "system-version.h"
extern unsigned long sys6_memtop(void);
int cli_hardware(ClariceHardware *out,unsigned long size){
 if(!out||size<sizeof *out)return -50;
 memset(out,0,sizeof *out);out->size=sizeof *out;out->version=1;out->fpu=-1;
 SysEnvRec env;memset(&env,0,sizeof env);
 if(!SysEnvirons(1,&env)){
  out->system=env.systemVersion;out->cpu=env.processor;
  out->model=env.machineType;out->model_namespace=1;
  out->fpu=env.hasFPU?-2:0;
 }
 out->ram=sys6_memtop();
 /* Gestalt is absent on early System 6. Test the native OS trap before use. */
 if(NGetTrapAddress(0xA1AD,0 /* OS trap */)!=NGetTrapAddress(0xA89F,1 /* Toolbox trap */)){
  int32_t value;
  if(!Gestalt('proc',&value)&&value>0&&value<=65535)out->cpu=value;
  if(!Gestalt('fpu ',&value)&&value>=0&&value<=32767)out->fpu=value;
  if(!Gestalt('ram ',&value)&&value>0)out->ram=value;
  if(!Gestalt('sysv',&value)&&value>0&&value<=65535)out->system=value;
  if(!Gestalt('mach',&value)){out->model=value;out->model_namespace=2;
   if(value>0&&value<=32767){unsigned char name[256];GetIndString(name,-16395,value);
    unsigned n=name[0];if(n>=sizeof out->model_name)n=sizeof out->model_name-1;
    memcpy(out->model_name,name+1,n);out->model_name[n]=0;}
  }
 }
 /* System 6.0.8 can retain the 6.0.7 SysEnvirons/Gestalt value.
    Resource Manager refNum 0 explicitly selects the System file. */
 short previous=CurResFile();UseResFile(0);
 if(!ResError()){
  Handle version=Get1Resource('vers',1);
  if(version){
   signed char state=HGetState(version);HLock(version);
   unsigned number=clarice_system_version((const unsigned char *)*version,GetHandleSize(version));
   HSetState(version,state);ReleaseResource(version);
   if(number)out->system=number;
  }
 }
 UseResFile(previous);
 if(!out->model_name[0]){const char *name=0;
  switch(env.machineType){case 2:name="Macintosh Plus";break;case 3:name="Macintosh SE";break;
   case 4:name="Macintosh II";break;case 5:name="Macintosh IIx";break;
   case 6:name="Macintosh IIcx";break;case 7:name="Macintosh SE/30";break;
   case 8:name="Macintosh Portable";break;}
  if(name)strcpy(out->model_name,name);
 }
 Rect bounds=qd.screenBits.bounds;out->depth=1;
 if(env.hasColorQD){GDHandle device=GetMainDevice();if(device&&(*device)->gdPMap){
  bounds=(*device)->gdRect;out->depth=(*(*device)->gdPMap)->pixelSize;}}
 out->width=bounds.right-bounds.left;out->height=bounds.bottom-bounds.top;
 return 0;
}
