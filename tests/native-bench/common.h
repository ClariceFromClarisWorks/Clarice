/* Independent bounded launch probe, not a production execution protocol. */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#define NB_MAGIC 0x4e423031UL
typedef struct {uint32_t magic,ticket,started,entered,finished,heap;int16_t status;char argument[32],out[96],err[32];} NBJob;
static void nb_body(NBJob *j){snprintf(j->out,sizeof j->out,"probe argument=%s\n",j->argument);strcpy(j->err,"probe stderr\n");j->status=7;}
