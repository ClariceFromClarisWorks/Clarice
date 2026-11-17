#ifndef CLARICE_NETWORK_COMMAND_H
#define CLARICE_NETWORK_COMMAND_H
#include <Multiverse.h>
#include <stdio.h>
#include <string.h>
#include "native.h"
extern void say(const char *),errsay(const char *),error(const char *,int);
extern int app_yield(void),output_error;
#endif
