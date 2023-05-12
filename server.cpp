#ifndef __SERVER_H__
#define __SERVER_H__

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

bool end_everything = true; // false = end tcp

void ctrlhandler(int dummy)
{
    end_everything = false; // this should close the entire prosses of threads
}

#include "copied_code.cpp"
#include "server_structs.hpp"
#include "server_Battleships.cpp"
#include "server_thread_func.cpp"

struct shared_data
{
    Battleships *game;
    int *print_num;
}; // to add later

class server
{
private:
    // Server global data
    int sfd;                                // Server file descripter
    struct sockaddr_in serv_addr;           // Server address information
    std::vector<client_data *> client_list; // Server client data

    int print_num = 0;                               // for line number
    Battleships *game = new Battleships(&print_num); // the battleships game

public:
    server(int portnum);
    void tcp_start_run();
    ~server();
};

server::server(int portnum)
{
    /* --- Initilise variables--- */
    int efd;

    /* --- Creating server--- */
    printf("%3d - Getting Creating Server", print_num++);
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

    int opts = 1;
    efd = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts));
    if (efd == -1)
        handle_error("setsockopt");

    /* --- Bind server to terminal --- */
    if (bind(sfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
        handle_error("bind");

    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;

    /* --- print server ips for connecting --- */
    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    checkHostName(hostname);
    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);
    checkHostEntry(host_entry);
    // To convert an Internet network
    // address into ASCII string
    IPbuffer = inet_ntoa(*((struct in_addr *)
                               host_entry->h_addr_list[0]));

    printf("info- Hostname   : %s\n", hostbuffer);
    printf("info- Ip Address : %s\n", IPbuffer);
    printf("info- Port Num   : %d\n", portnum);
}

void server::tcp_start_run()
{
    printf("%3d - Waiting for client...\n", print_num++);

    /* --- initialization --- */
    pthread_t tcp_start_run_thread, thread_megamain;
    tcp_bulk_data bulk_data_tcp;
    bulk_data_tcp.game = this->game;
    bulk_data_tcp.print_num = &(this->print_num);
    bulk_data_tcp.sfd = this->sfd;
    bulk_data_tcp.client_list = &(this->client_list);
    struct megamain_shared_data mmsd;
    mmsd.game = this->game;
    mmsd.print_num = &(this->print_num);
    bulk_data_tcp.mmsd = &mmsd;

    signal(SIGINT, ctrlhandler);

    if (pthread_create(&tcp_start_run_thread, NULL, tcp_thread, &bulk_data_tcp) != 0)
        handle_error("thread");

    if (pthread_create(&thread_megamain, NULL, megamain_thread, &mmsd) != 0)
        handle_error("thread");

    while (end_everything) // while true loop
    {
        // do nothing tcp_thread is doing everythingshi
    }
    // once end_everything = false => we end everything

    printf("losing server... \n"); // ^C then print losing server => Closing Server
    void *ret;
    int s;

    /* --- closing tcp thread --- */
    s = pthread_cancel(tcp_start_run_thread);
    if (s != 0)
        handle_error_ernum(s, "pthread_cancel");

    s = pthread_join(tcp_start_run_thread, &ret);
    if (s != 0)
        handle_error_ernum(s, "pthread_join");

    s = pthread_cancel(thread_megamain);
    if (s != 0)
        handle_error_ernum(s, "pthread_cancel");

    s = pthread_join(thread_megamain, &ret);
    if (s != 0)
        handle_error_ernum(s, "pthread_join");
};

server::~server()
{
    printf("Closing client connections...\n");
    void *ret;
    int s;

    for (size_t i = 0; i < client_list.size(); i++)
    {
        /* --- removed indicates that the  --- */
        if (!client_list.at(i)->removed)
        {
            s = pthread_cancel(client_list.at(i)->thread_settings);
            if (s != 0)
                handle_error_ernum(s, "pthread_cancel");
            /* --- once settings ends, collapse captures settings and closes everything --- */
        }
        /* --- we catch collapse once finished --- */
        s = pthread_join(client_list.at(i)->thread_collapse, &ret);
        if (s != 0)
            handle_error_ernum(s, "pthread_join");

        /* --- close each file descripter --- */
        close(client_list.at(i)->cfd);
        close(client_list.at(i)->sfd);
        close(client_list.at(i)->ufd);

        /* --- free the client memory we malloced --- */
        free(client_list.at(i));
    }
    free(game);

    printf("Closing server...\n");
    close(sfd);
    exit(0);
}

#endif // __SERVER_H__