#ifndef __SERVER_STRUCTS_H__
#define __SERVER_STRUCTS_H__

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

#include "server_Battleships.cpp"
#include "server_Battleships_struct.hpp"
#include "server_util_func.hpp"

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

// information the client needs to communicate
struct client_data
{
    int cfd, ufd, sfd;           // Client File Descripters (client, update, settings)
    int cli_id;                  // Client Identfieiers
    struct sockaddr_in cli_addr; // Client Address
    socklen_t cli_addr_size;     // Client Size

    pthread_t thread_main, thread_update, thread_settings, thread_collapse;
    /*  thread_main    : main game oriended thread
     *  thread_update  : update related thread (message out only)
     *  thread_settings: setting/update thread (message in , message out only)
     *  thread_collapse: for ending the connection (wait for settings thread)
     */
    bool removed = false;
};

struct settings_request_playerlist
{
    int size;
    player_data player_list[30];
};

struct settings_request_boardlist
{
    int size;
    bb board_list[4];
};

struct client_game_package
{
    int next_cli_id;
    int size;
    bb board_list[4];
};

struct megamain_shared_data
{
    Battleships *game; // game iteself
    int *print_num;    // print num for server info

    /* --- these will ne shared with main_thread --- */
    bool flag1, flag2, flag3; // flags dectating when things should happen
    int next_cli;             // next player that pos should recive from
    int pos_player_info;      // 0 => player new pos_hit, 1 => AI
    cord_board pos_hit;       // position hit
};

struct tcp_bulk_data
{
    Battleships *game;
    int *print_num;

    int sfd;
    struct megamain_shared_data *mmsd;
    std::vector<client_data *> *client_list;
};

struct bulk_data
{
    struct client_data *client; // private- client
    // std::vector<player_data *> *player_list; // public - list of players
    // player_data *player_info;                // private - player listed
    Battleships *game;
    int *print_num;

    // this is for main_thread only & possibly collapse thread
    struct megamain_shared_data *mmsd;
};

struct thread_containter
{
    pthread_t main_thread, collapse_thread, update_thread;
};

#include "client_display.cpp"

struct tcp_client_bulk_data
{
    client_display *disp;
    int *cfd, *ufd, *sfd, portno;
    char *ip;
    player_data *player_id;
    thread_containter *threadc;
    bool *ended, *your_turn_flag;
    settings_request_playerlist *srp;
};

#endif // __SERVER_STRUCTS_H__