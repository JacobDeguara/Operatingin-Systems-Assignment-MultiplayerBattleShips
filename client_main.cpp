#ifndef __MAIN_CLIENT_H__
#define __MAIN_CLIENT_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include "client.cpp"

int main(int argc, char *argv[])
{
    client net = client(argc, argv);

    net.tcp_connect_server(argc, argv);
}
#endif // __MAIN_CLIENT_H__