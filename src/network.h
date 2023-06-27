#ifndef _NETWORK_H_
#define _NETWORK_H_

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#include "global.h"

typedef SOCKET Connection_t;

Connection_t    network_listen  (uint16_t port);
void            network_close   (Connection_t);
void            network_send    (Connection_t, uint8_t*, int);
uint8_t*        network_receive (Connection_t, int*, BOOL*);

#endif