/* No allocation or networking while disabled. Loader owns the enabled image. */
static ClariceService service;
static Ptr service_image;
static int service_loading,service_registered,service_polling;
int cli_service_register(const ClariceService *s){
 if(!service_loading||service_image||service_registered||!s||!s->poll||!s->stop||shell_session_active())return -1;
 service=*s;service_registered=1;return 0;
}
void shell_service_poll(int execute){
 if(!service_image||service_polling)return;
 service_polling=1;service.poll(execute);service_polling=0;
}
int shell_service_stop(void){
 if(!service_image)return 0;
 if(service_polling||shell_session_active())return -1;
 if(service.stop())return -1;
 DisposePtr(service_image);service_image=0;memset(&service,0,sizeof service);return 0;
}

int cli_service_stop(void){return shell_service_stop();}
int cli_service_enabled(void){return service_image!=0;}
