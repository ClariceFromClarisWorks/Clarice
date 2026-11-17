#ifndef CLARICE_REMOTE_SETTINGS_H
#define CLARICE_REMOTE_SETTINGS_H
/* Native dialog boundary: transient password never enters shell argv/history. */
typedef struct {char peer[16];unsigned short port;short enabled;char password[128];} ClariceRemoteSettings;
int app_remote_settings(ClariceRemoteSettings *);
int cli_service_stop(void);
int cli_service_enabled(void);
#endif
