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

#include "Battleship_Stucts.hpp"
#include "Battleships.cpp"

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

struct client_data
{
    int cfd, vec_id, cli_id;     // Client File Descripter
    struct sockaddr_in cli_addr; // Client Address
    socklen_t cli_addr_size;     // Client Size
    pthread_t thread;
    Battleships *game;
};

class server
{
private:
    int sfd, cfd, efd;
    struct sockaddr_in serv_addr;
    std::vector<client_data> client_list;
    Battleships game;

public:
    void server_start_tcp();
    server(int portnum);
    ~server();
};

server::server(int portnum)
{
    printf("Getting Creating Server");

    printf(".");

    this->sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sfd == -1)
        handle_error("socket");

    printf(".");

    memset(&this->serv_addr, 0, sizeof(this->serv_addr));
    this->serv_addr.sin_family = AF_INET;
    this->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any address
    this->serv_addr.sin_port = htons(portnum);           // representing the USS California (BB-44) (88-44)

    printf(".\n");

    if (bind(this->sfd, (struct sockaddr *)&this->serv_addr,
             sizeof(this->serv_addr)) == -1)
        handle_error("bind");
}

void *host_player_check(void *arg)
{
    client_data *data = (client_data *)arg;
    fd_set rfds;
    struct timeval tv;
    int retval, efd;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    tv.tv_sec = 11000;
    tv.tv_usec = 0;

    /* --- wait for important read ---*/
    retval = select(1, &rfds, NULL, NULL, &tv);

    if (retval == -1)
    {
        handle_error("select");
    }
    else if (retval)
    {
        /* --- read player data --- */
        bool start_game = false;

        efd = read(data->cfd, &start_game, sizeof(bool));
        if (efd < 0)
            handle_error("read");

        if (start_game)
            pthread_exit((void *)1);
        printf("Host desided not to start game");
        pthread_exit((void *)0);
    }
    else
    {
        printf("Host hasnt started the game in 5min\n");
        pthread_exit((void *)0);
    }

    return NULL;
}

void *player_repeated_update(void *arg)
{
    /* --- set cancelable state --- */
    int s;
    s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0)
        handle_error_ernum(s, "pthread_setcancelstate");

    /* --- initialise data of player --- */
    client_data *data = (client_data *)arg;
    int efd;

    while (1)
    {
        /* --- setup other players info --- */
        players_info players_info_list;
        players_info_list.flag = true;
        auto players = data->game->get_players_info();
        auto spectators = data->game->get_spectators();
        players_info_list.amount_of_players = players.size();
        players_info_list.amount_of_spectators = spectators.size();

        for (size_t i = 0; i < players.size(); i++)
        {
            strcpy(players_info_list.players[i].name, players.at(i).name.c_str());
            players_info_list.players[i].id = players.at(i).id;
            players_info_list.players[i].status_type = players.at(i).status_type;
            printf(" %s ", players.at(i).name.c_str());
        }

        for (size_t i = 0; (i < spectators.size()) && (i < 11); i++)
        {
            strcpy(players_info_list.spectators[i].name, spectators.at(i).name.c_str());
            printf(" %s ", spectators.at(i).name.c_str());
        }

        /* --- send player other player info --- */
        efd = write(data->cfd, &players_info_list, sizeof(players_info));
        if (efd < 0)
        {
            printf("connection ended\n");
            pthread_exit(0);
        }
        sleep(9); // give some space inbetween
    }
    return NULL;
}

// gives additonal control : start game()
void *host_player(void *arg)
{
    /* --- set cancelable state --- */
    int s;
    s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0)
        handle_error_ernum(s, "pthread_setcancelstate");

    /* --- initialise data of player --- */
    client_data *data = (client_data *)arg;
    int efd;
    char buffer[BUFFERSIZE];
    char name[15];

    /* --- read player data --- */
    efd = read(data->cfd, &name, sizeof(name)); // should only have player name
    if (efd < 0)
        handle_error("read");

    /* --- add player to game --- */
    auto new_id = data->game->add_player(name);
    printf("Player created : (%s, %d)\n", new_id.name.c_str(), new_id.id);
    /* --- copy info to proper format --- */
    player_info new_info;
    new_info.id = new_id.id;
    new_info.status_type = new_id.status_type;
    new_info.conf = new_id.confirmation;
    new_info.host = true; // set host to true
    strcpy(new_info.name, new_id.name.c_str());

    /* --- send player state confirmation --- */
    efd = write(data->cfd, (char *)&new_info, sizeof(player_info));
    if (efd < 0)
        handle_error("write"); // this can be seens as player leaveing so if player is playing he will get replaced by KURT

    /* --- Repeated player update --- */
    pthread_t thread_players_info;
    if (pthread_create(&thread_players_info, NULL, player_repeated_update, data) != 0)
        handle_error("thread");

    pthread_t thread_game_start;
    if (pthread_create(&thread_game_start, NULL, host_player_check, data) != 0)
        handle_error("thread");

    while (1)
    {
    }

    close(data->cfd);
    return NULL;
}

// controls the each player output
void *player(void *arg)
{
    /* --- set cancelable state --- */
    int s;
    s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0)
        handle_error_ernum(s, "pthread_setcancelstate");

    /* --- initialise data of player --- */
    client_data *data = (client_data *)arg;
    int efd;
    char buffer[BUFFERSIZE];
    char name[15];

    /* --- read player data --- */
    efd = read(data->cfd, &name, sizeof(name)); // should only have player name
    if (efd < 0)
        handle_error("read");

    /* --- add player to game --- */
    auto new_id = data->game->add_player(name);
    printf("Player created : (%s, %d)\n", new_id.name.c_str(), new_id.id);

    /* --- copy info to proper format --- */
    player_info new_info;
    new_info.id = new_id.id;
    new_info.status_type = new_id.status_type;
    new_info.conf = new_id.confirmation;
    new_info.host = false; // set host to false
    strcpy(new_info.name, new_id.name.c_str());

    /* --- send player state confirmation --- */
    efd = write(data->cfd, (char *)&new_info, sizeof(player_info));
    if (efd < 0)
        handle_error("write"); // this can be seens as player leaveing so if player is playing he will get replaced by KURT

    /* --- use state confirmation to determine weather player is spectator or player---*/
    if (new_info.conf == false) // bad user name
    {
        close(data->cfd);
        return NULL;
    }

    pthread_t thread_players_info;

    if (pthread_create(&thread_players_info, NULL, player_repeated_update, data) != 0)
        handle_error("thread");

    while (1)
    {
    }

    if (new_info.status_type != Not_Allowed) // player
    {
    }
    else // spectator
    {
    }

    close(data->cfd);
    return NULL;
}

void server::server_start_tcp()
{
    printf("Waiting for client...\n");

    int count = 0;
    game = Battleships();
    /* --- listen for multiple clients ---*/
    while (1)
    {
        if (listen(sfd, LISTEN_BACKLOG) == -1)
            handle_error("listen");

        struct client_data client;
        client.vec_id = client_list.size();
        client.cli_addr_size = sizeof(client.cli_addr);
        client.game = &game;

        client.cfd = accept(sfd, (struct sockaddr *)&client.cli_addr,
                            &client.cli_addr_size);
        if (client.cfd == -1)
            handle_error("accept");

        printf("Accepted Client, Creating Thread\n");
        if (count == 0)
        {
            if (pthread_create(&client.thread, NULL, host_player, &client) != 0)
                handle_error("thread");
            count++;
        }
        else
        {
            if (pthread_create(&client.thread, NULL, player, &client) != 0)
                handle_error("thread");
        }

        client_list.push_back(client);
    }
}

server::~server()
{
    printf("Closing client connections...");
    void *ret;
    for (size_t i = client_list.size() - 1; i >= 0; i--)
    {
        int s;
        /* cancel thread half way */
        s = pthread_cancel(client_list.at(i).thread);
        if (s != 0)
            handle_error_ernum(s, "pthread_cancel");

        /* join thead to get result */
        s = pthread_join(client_list.at(i).thread, &ret);
        if (s != 0)
            handle_error_ernum(s, "pthread_join");

        /* additonal check to see if pthread was cancelled */
        if (ret != PTHREAD_CANCELED)
            handle_error("pthread_cancel");
    }

    printf("Closing server...");
    for (size_t i = 0; i < client_list.size(); i++)
    {
        close(client_list.at(i).cfd);
    }
    close(sfd);
    exit(0);
}

#endif // __SERVER_H__