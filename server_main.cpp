#include "server.cpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <pthread.h>
#include <vector>

#include "rw_mutex.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("give me port number!\n");
        exit(EXIT_FAILURE);
    }
    server hosted = server(atoi(argv[1]));

    hosted.tcp_start_run();
}