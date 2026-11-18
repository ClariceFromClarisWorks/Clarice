/* One supplementary command context. Transport remains owned by its caller.
   Calls are synchronous; never reenter from an I/O callback or app_yield. */
#ifndef CLARICE_SESSION_H
#define CLARICE_SESSION_H
#include <stddef.h>
typedef struct {
 void *context;
 long (*read)(void *,void *,unsigned long);
 long (*write)(void *,int,const void *,unsigned long);
 int (*cancelled)(void *);
} ClariceSessionIO;
/* Binary protocol ownership: query=0, begin=1, read=2, write=3, end=4.
   Existing session ABI stays unchanged; optional callback is registered separately. */
typedef long (*ClariceSessionWire)(int,void *,unsigned long);
int cli_session_set_wire(void *,ClariceSessionWire);
long cli_remote_wire(int,void *,unsigned long);
void *cli_session_open(const ClariceSessionIO *);
/* Result: low byte command status; bit 8 requests logout. 125 rejects reentry. */
int cli_session_run(void *,const char *);
int cli_session_close(void *);
int shell_session_active(void);
int shell_session_ui_begin(void);
void shell_session_ui_end(int);
void cli_session_prompt(void *,char *,unsigned long);
int shell_session_cancelled(void);
int shell_session_output(const char *,long);
int shell_session_remote_io(void);
#endif
