#ifndef SOCKETUTIL_H
#define SOCKETUTIL_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __APPLE__
    #include <stdlib.h>
#elif defined(__linux__)
    #include <malloc.h>
#endif

struct sockaddr_in* createIPv4SockAddr(const char* ip, int port);
int createTCPIPv4Socket();

#endif // !SOCKETUTIL_H
