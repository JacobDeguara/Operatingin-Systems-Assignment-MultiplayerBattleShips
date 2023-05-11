#ifndef __SERVER_TEST_H__
#define __SERVER_TEST_H__

#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/un.h>
#include <string.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#define BUFFERSIZE 256
#define LISTEN_BACKLOG 5120 // up to (1024*4) messages hopefully thats enough?

#define handle_error_ernum(en, msg) \
    do                              \
    {                               \
        errno = en;                 \
        perror(msg);                \
        exit(EXIT_FAILURE);         \
    } while (0)

#define handle_error(msg)   \
    do                      \
    {                       \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

struct shared_data
{
};

struct client_data
{
    char name[15];
    int cfd, ufd, sfd;            // Client File Descripters (client, update, settings)
    int vec_id, cli_id;           // Client Identfieiers
    struct sockaddr_in cli_addr;  // Client Address
    socklen_t cli_addr_size;      // Client Size
    pthread_t thread;             // threads running
    struct shared_data *data_ptr; // Shared data
};

struct bulk_data
{
    std::vector<client_data> client_list;
    struct client_data client;
    struct shared_data *data_ptr; // Shared data
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("give me port number!\n");
        exit(EXIT_FAILURE);
    }
    printf("PortNum %s\n", argv[1]);

    /* --- Initilise variables--- */
    int portnum = atoi(argv[1]);
    int sfd, efd;
    struct sockaddr_in serv_addr;
    struct shared_data data;

    /* --- Creating server--- */
    printf("Getting Creating Server");
    printf(".");

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
        handle_error("socket");

    printf(".");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any address
    serv_addr.sin_port = htons(portnum);           // representing the USS California (BB-44) (88-44)

    printf(".\n");

    /* --- Bind server to terminal --- */
    if (bind(sfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) == -1)
        handle_error("bind");

    /* --- Wait for client connection --- */
    if (listen(sfd, LISTEN_BACKLOG) == -1)
        handle_error("listen");

    /* --- Connect client to server --- */
    struct client_data client;
    client.cli_addr_size = sizeof(client.cli_addr);

    /* --- Connect /client/ file descriptor --- */
    client.cfd = accept(sfd, (struct sockaddr *)&client.cli_addr,
                        &client.cli_addr_size);
    if (client.cfd == -1)
        handle_error("accept");

    fcntl(cfd, F_SETFD, 1);
    /* --- Connect /settings/ file descriptor --- */
    client.sfd = accept(sfd, (struct sockaddr *)&client.cli_addr,
                        &client.cli_addr_size);
    if (client.cfd == -1)
        handle_error("accept");

    /* --- Connect /update/ file descriptor --- */
    client.ufd = accept(sfd, (struct sockaddr *)&client.cli_addr,
                        &client.cli_addr_size);
    if (client.cfd == -1)
        handle_error("accept");

    /* --- select test Initilise --- */
    fd_set rfds;
    struct timeval tv;
    int retval;
    FD_ZERO(&rfds);
    FD_SET(client.sfd, &rfds);
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    /* --- select test wait for secon input --- */
    retval = select(FD_SETSIZE, &rfds, NULL, NULL, NULL);
    if (retval == -1)
        perror("select()");
    else if (retval)
        printf("Data is available now.\n");
    else
        printf("This shouldn't happen\n");

    /* --- read player data --- */
    char name[15];
    memset(name, 0, sizeof(name));
    efd = read(client.cfd, &name, sizeof(name)); // should only have player name
    if (efd < 0)
        handle_error("read");

    printf(" Player messaged through cfd : %s\n", name);

    memset(name, 0, sizeof(name));
    efd = read(client.sfd, &name, sizeof(name)); // should only have player name
    if (efd < 0)
        handle_error("read");

    printf(" Player messaged through sfd : %s\n", name);

    /* --- test select timer fail ---*/
    FD_ZERO(&rfds);
    FD_SET(client.ufd, &rfds);
    retval = select(1, &rfds, NULL, NULL, &tv);

    if (retval == -1)
        perror("select()");
    else if (retval)
        printf("Data is available now.\n");
    else
        printf("No data within three seconds.\n"); // should always happen since sleep(5) from client_test

    memset(name, 0, sizeof(name));
    efd = read(client.ufd, &name, sizeof(name));
    if (efd < 0)
        handle_error("read");

    printf(" Player messaged through ufd : %s\n", name);
}

#endif // __SERVER_TEST_H__