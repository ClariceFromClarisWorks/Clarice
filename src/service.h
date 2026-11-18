#ifndef CLARICE_SERVICE_H
#define CLARICE_SERVICE_H
/* One optional CLX service; callbacks remain valid until stop succeeds. */
typedef struct { void (*poll)(int); int (*stop)(void); } ClariceService;
int cli_service_register_slot(int,const ClariceService *);
int cli_service_stop_slot(int);
int cli_service_enabled_slot(int);
int cli_service_register(const ClariceService *);
void shell_service_poll(int);
int shell_service_stop(void);
#endif
