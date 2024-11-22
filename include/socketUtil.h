#ifndef SOCKETUTIL_H
#define SOCKETUTIL_H

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct sockaddr_in* createIPv4SockAddr(const char* ip, int port);
int createTCPIPv4Socket();

#endif // !SOCKETUTIL_H
