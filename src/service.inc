/* Two optional transports, one shared remote shell. No disabled allocations. */
static ClariceService service,serial_service;
static Ptr service_image,serial_service_image;
static int service_loading,service_registered,service_polling;
int cli_service_register_slot(int slot,const ClariceService *s){
 if(slot<0||slot>1||!service_loading||(slot?serial_service_image:service_image)||service_registered||!s||!s->poll||!s->stop||shell_session_active())return -1;
 if(slot)serial_service=*s;else service=*s;service_registered=slot+1;return 0;
}
int cli_service_register(const ClariceService *s){return cli_service_register_slot(0,s);}
void shell_service_poll(int execute){
 if(service_polling)return;
 service_polling=1;if(service_image)service.poll(execute);if(serial_service_image)serial_service.poll(execute);service_polling=0;
}
int cli_service_stop_slot(int slot){
 if(slot<0||slot>1)return -1;
 Ptr *image=slot?&serial_service_image:&service_image;ClariceService *s=slot?&serial_service:&service;
 if(!*image)return 0;if(service_polling||shell_session_active())return -1;
 if(s->stop())return -1;DisposePtr(*image);*image=0;memset(s,0,sizeof *s);return 0;
}
int shell_service_stop(void){int a=cli_service_stop_slot(0),b=cli_service_stop_slot(1);return a||b?-1:0;}
int cli_service_stop(void){return cli_service_stop_slot(0);}
int cli_service_enabled_slot(int slot){return slot==0?service_image!=0:slot==1?serial_service_image!=0:0;}
int cli_service_enabled(void){return cli_service_enabled_slot(0);}
