/* Thin invocation-owned adapter; upstream source remains unmodified. */
#ifndef CLARICE_TELNET_ENGINE_H
#define CLARICE_TELNET_ENGINE_H
#include "../vendor/libtelnet/libtelnet.h"
int clarice_telnet_pending(telnet_t *t);
int clarice_telnet_enabled(telnet_t *t, unsigned char option, int local);
#endif
