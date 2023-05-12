#ifndef __SERVER_THREAD_FUNC_H__
#define __SERVER_THREAD_FUNC_H__

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

#include "server_structs.hpp"
#include "server.cpp"
#include "rw_mutex.h"

void *tcp_thread(void *arg);
void *megamain_thread(void *arg);
void *settings_thread(void *arg);
void *collapse_thread(void *arg);
void *main_thread(void *arg);
void *update_thread(void *arg);

void *tcp_thread(void *arg)
{
    /* --- set cancelable state --- */
    int ert_j;
    ert_j = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_setcancelstate");

    /* --- Initilize variables --- */
    tcp_bulk_data *data = (tcp_bulk_data *)arg;
    int efd;
    int client_count = 0;

    while (1)
    {
        data->game->display_players();

        /* --- Wait for client connection --- */
        if (listen(data->sfd, LISTEN_BACKLOG) == -1)
            handle_error("listen");

        /* --- Connect client to server --- */
        struct client_data *client = (struct client_data *)malloc(sizeof(client_data)); // will be created
        client->cli_addr_size = sizeof(client->cli_addr);
        client->cli_id = ++(client_count);

        /* --- Connect /client/ file descriptor --- */
        client->cfd = accept(data->sfd, (struct sockaddr *)&client->cli_addr,
                             &client->cli_addr_size);
        if (client->cfd == -1)
            handle_error("accept");

        /* --- Connect /settings/ file descriptor --- */
        client->sfd = accept(data->sfd, (struct sockaddr *)&client->cli_addr,
                             &client->cli_addr_size);
        if (client->sfd == -1)
            handle_error("accept");

        /* --- Connect /update/ file descriptor --- */
        client->ufd = accept(data->sfd, (struct sockaddr *)&client->cli_addr,
                             &client->cli_addr_size);
        if (client->ufd == -1)
            handle_error("accept");

        printf("%3d - Player test in progress...\n", (*data->print_num)++);
        /* --- read player name test --- */
        char name[15], name_tmp[15];
        bool player_test = false;

        memset(name, 0, sizeof(name));
        efd = read(client->cfd, &name, sizeof(name)); // should only have player name
        if (efd < 0)
            handle_error("read");
        strcpy(name_tmp, name);

        memset(name, 0, sizeof(name));
        efd = read(client->sfd, &name, sizeof(name)); // should only have player name
        if (efd < 0)
            handle_error("read");

        // check same name
        if (strcmp(name, name_tmp) != 0)
            player_test = true;

        memset(name, 0, sizeof(name));
        efd = read(client->ufd, &name, sizeof(name));
        if (efd < 0)
            handle_error("read");

        // check same name
        if (strcmp(name, name_tmp) != 0)
            player_test = true;

        /* --- check that player name is unique --- */
        if (data->game->player_exists(name))
        {
            player_test = true;
        }

        if (player_test)
        {
            printf("%3d - Player test !FAIL! (didn't add %15s)\n", (*data->print_num)++, name);
            /* --- tests fail : return with Not allowed --- */
            player_data pd;
            strncpy(pd.name, name, sizeof(name));
            pd.state = Not_Allowed;
            pd.cli_id = client->cli_id;

            if ((efd = write(client->cfd, &pd, sizeof(player_data))) < 0)
                handle_error("write");
        }
        else
        {
            printf("%3d - Player test !SUCCESS! (adding %s to the system)\n", (*data->print_num)++, name);
            /* --- test success : return new player data , and add them to server & game, & start threads  --- */

            /* --- new player data initialize --- */
            player_data pd = data->game->add_player(name, client->cli_id); // add player to battle ships

            if ((efd = write(client->cfd, &pd, sizeof(player_data))) < 0)
                handle_error("write");

            /* --- setup connections and data sharing --- */
            data->client_list->push_back(client); // add client to client list

            player_data *pd_ptr = (player_data *)malloc(sizeof(player_data));
            memcpy(pd_ptr, &pd, sizeof(player_data));

            /* --- shared data bulk creation --- */
            bulk_data *shared_data_bulk = (bulk_data *)malloc(sizeof(bulk_data));
            shared_data_bulk->client = client;
            shared_data_bulk->game = data->game;
            shared_data_bulk->print_num = data->print_num;
            shared_data_bulk->mmsd = data->mmsd;

            printf("%3d - Starting Thread for player %s...\n", (*data->print_num)++, name);
            /* --- start thread cycle --- */
            /*  Thread idea:

                main thread - will go on constantly and there will also be a host thread
                (wheather the host thread is going to be seprate or not is still in question)
                | host - cancelable - false/true (not decided) |
                | players - cancelable - true |

                update thread - will go on constantly providing the current state of players and will be avaiable for all players
                | cancelable - true |

                settings thread - will go constantly and will wait until a request will happen.
                Additonally it will check for still alive parameters.
                If player decided to end the game will be removed by exiting into the collapse thread
                | mutex locking required (only one setting can run at a time to prevent issues)|
                | cancelable - true(unless in a sensative state) |

                collapse thread - will always wait for settings thread to end.
                (once it ends) it will close each thread and set the thread_cancilation in that client to on.
                will remove the player from the global variables exit()

            */

            /* --- settings thread --- */
            if (pthread_create(&shared_data_bulk->client->thread_settings, NULL, settings_thread, shared_data_bulk) != 0)
                handle_error("thread");

            if (pthread_create(&shared_data_bulk->client->thread_update, NULL, update_thread, shared_data_bulk) != 0)
                handle_error("thread");

            if (pthread_create(&shared_data_bulk->client->thread_main, NULL, main_thread, shared_data_bulk) != 0)
                handle_error("thread");

            if (pthread_create(&shared_data_bulk->client->thread_collapse, NULL, collapse_thread, shared_data_bulk) != 0)
                handle_error("thread");

            printf("%3d - Threading established, Waiting for next player...\n", (*data->print_num)++);
        }
    }
}

void *megamain_thread(void *arg)
{
    /* --- set cancelable state --- */
    int ert_j;
    ert_j = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_setcancelstate");

    /* --- Initilize variables --- */
    megamain_shared_data *data = (megamain_shared_data *)arg;
    int efd;

    /* --- Pre-Initilize shared data before player breaks --- */
    data->flag1 = true;
    data->flag2 = true;
    data->flag3 = true;
    data->next_cli = -1;
    data->pos_player_info = 0;
    data->pos_hit.x = 0;
    data->pos_hit.y = 0;

    /* --- if (game state != SHIP) => |loop| else proceed --- */
    while (data->game->get_game_state() != 1)
    {
        sleep(1);
    }

    printf("%3d - Game started main_Func noticed - sent waiting for ships message\n", (*data->print_num)++);
    data->flag1 = false;

    /* --- if( game_start() == false ) => |loop| else proceed --- */
    while (!data->game->game_start())
    {
        sleep(1);
    }

    printf("%3d - Game started main_Func noticed - starting game...\n", (*data->print_num)++);
    data->flag2 = false;

    while (1)
    {
        sleep(1);
    }

    return NULL;
}

void *settings_thread(void *arg)
{
    /* --- set cancelable state --- */
    int ert_j;
    ert_j = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_setcancelstate");

    /* --- Initilize variables --- */
    bulk_data *data = (bulk_data *)arg;
    int efd;

    /* --- select test Initilise --- */
    fd_set rfds;
    int retval;
    FD_ZERO(&rfds);
    FD_SET(data->client->sfd, &rfds);

    while (1)
    {
        /* --- reset timer and wait for request / periodic still alive check --- */
        retval = select(FD_SETSIZE, &rfds, NULL, NULL, NULL); // breaks here? second round

        if (retval == -1)
            handle_error("select");
        else if (retval)
        {
            // check if client has left
            if (recv(data->client->sfd, NULL, 1, MSG_PEEK | MSG_DONTWAIT) == 0)
            {
                printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
                return NULL;
            }

            /* --- Player sent request get player request --- */
            int req;
            printf("%3d - Player %d request information\n", (*data->print_num)++, data->client->cli_id);
            efd = read(data->client->sfd, &req, sizeof(int)); // should only have player name
            if (efd < 0)
            {
                printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
                return NULL;
            }

            printf("%3d - Player %d request information protocol %d \n", (*data->print_num)++, data->client->cli_id, req);

            /* request list of actions:
             *  (-1) - remove player
             *  ( 0) - update players (gives the list of players)
             *  ( 1) - lets you switch from player to spectator
             *  ( ect..)
             */
            std::vector<player_data> list;

            switch (req)
            {
            case -1: // remove player
                printf("%3d - Player %d decided to disconnect\n", (*data->print_num)++, data->client->cli_id);
                return NULL;
                break;
            case 0: // update players
                settings_request_playerlist srp;
                list = data->game->get_players_all();

                srp.size = list.size();
                std::copy(list.begin(), list.end(), srp.player_list);

                efd = write(data->client->sfd, (char *)&srp, srp.size * sizeof(player_data) + sizeof(int));
                if (efd < 0)
                {
                    printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
                    return NULL;
                }
                printf("%3d - Player %d recived Players list..\n", (*data->print_num)++, data->client->cli_id);

                break;
            case 1: // switch player state
                player_data new_id = data->game->switch_player(data->client->cli_id);

                efd = write(data->client->sfd, (char *)&new_id, sizeof(player_data));
                if (efd < 0)
                {
                    printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
                    return NULL;
                }
                printf("%3d - Player %d new information changed and sent...\n", (*data->print_num)++, data->client->cli_id);
                break;
            }
            // switch was causeing errors if case 2: ?????
            if (req == 2)
            {
                bool flag = data->game->game_start_ship(data->client->cli_id);
                if (flag) // true
                {
                    printf("%3d - Player %d started the game, now allowing ships to start being added\n", (*data->print_num)++, data->client->cli_id);
                    int res = 1; // game started
                    efd = write(data->client->sfd, (char *)&res, sizeof(int));
                    if (efd < 0)
                    {
                        printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
                        return NULL;
                    }
                }
                else
                {
                    printf(" %3d - Player %d tried starting the game, game state wasnt changed\n", (*data->print_num)++, data->client->cli_id);
                    int res = 0; // game hasnt started
                    efd = write(data->client->sfd, (char *)&res, sizeof(int));
                    if (efd < 0)
                    {
                        printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
                        return NULL;
                    }
                }
            }

            if (req == 3)
            {
                ship_placement ships_list[7];

                /* --- get ship placements --- */
                printf("%3d - Waiting for ship list...\n", (*data->print_num)++);
                efd = read(data->client->sfd, &ships_list, sizeof(ship_placement) * 7);
                if (efd < 0)
                {
                    printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
                    return NULL;
                }

                /* --- change form c standerd array to vector --- */
                std::vector<ship_placement> ship_list_vec;
                for (size_t i = 0; i < 7; i++)
                {
                    ship_list_vec.push_back(ships_list[i]);
                }

                /* --- add ships ---*/
                bool flag = data->game->add_ships(ship_list_vec, data->client->cli_id);

                if (flag) // true
                {
                    printf("%3d - Ships added to game...\n", (*data->print_num)++);
                    int res = 0; // ship added
                    efd = write(data->client->sfd, (char *)&res, sizeof(int));
                    if (efd < 0)
                    {
                        printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
                        return NULL;
                    }
                }
                else // false
                {
                    if (data->game->get_game_state() == 1) // game state == SHIP
                    {

                        printf("%3d - ships wrong format...\n", (*data->print_num)++);
                        int res = ship_format_confirmer(ship_list_vec); // ship not correct format
                        efd = write(data->client->sfd, (char *)&res, sizeof(int));
                        if (efd < 0)
                        {
                            printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
                            return NULL;
                        }
                    }
                    else
                    {

                        printf("%3d - ships cant be added currently...\n", (*data->print_num)++);
                        int res = -1; // game state not right time
                        efd = write(data->client->sfd, (char *)&res, sizeof(int));
                        if (efd < 0)
                        {
                            printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
                            return NULL;
                        }
                    }
                }
            }

            memset(&req, -2, sizeof(int));
        }
    }
    return NULL;
}

void *main_thread(void *arg)
{
    /* --- set cancelable state --- */
    int ert_j;
    ert_j = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_setcancelstate");

    /* --- Initilize variables --- */
    bulk_data *data = (bulk_data *)arg;
    int efd;

    /* --- waits for mega main to switch to game SHIP state --- */
    while (data->mmsd->flag1)
    {
        sleep(1);
    }

    int meta_data_game_started = 0;

    /* --- send player game started => SHIP ---*/
    efd = write(data->client->cfd, &meta_data_game_started, sizeof(int));
    if (efd < 0)
    {
        printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
        return NULL;
    }

    while (data->mmsd->flag2)
    {
        sleep(1);
    }

    meta_data_game_started = 1;

    /* --- send player game started => GAME---*/
    efd = write(data->client->cfd, &meta_data_game_started, sizeof(int));
    if (efd < 0)
    {
        printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
        return NULL;
    }

    while (1)
    {
        sleep(1);
    }

    return NULL;
}

void *update_thread(void *arg)
{
    /* --- set cancelable state --- */
    int ert_j;
    ert_j = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_setcancelstate");

    /* --- Initilize variables --- */
    bulk_data *data = (bulk_data *)arg;
    int efd;

    while (1)
    {
        // this can be changed later to be faster
        std::vector<player_data> list;
        settings_request_playerlist srp;
        list = data->game->get_players_all();

        srp.size = list.size();
        std::copy(list.begin(), list.end(), srp.player_list);

        efd = write(data->client->ufd, (char *)&srp, srp.size * sizeof(player_data) + sizeof(int));
        if (efd < 0)
        {
            printf("%3d - Player %d went away \n", (*data->print_num)++, data->client->cli_id);
            return NULL;
        }
        sleep(1);
    }

    return NULL;
}

void *collapse_thread(void *arg)
{
    /* --- Initilize variables --- */
    bulk_data *data = (bulk_data *)arg;
    int ert_j; // er|rror t|hread_j|oin => ert_j

    void *ret;
    ert_j = pthread_join(data->client->thread_settings, &ret);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_join");
    /* --- ^ this should happen once they player disconnects/exits ^ ---*/

    /* --- end threads to prevent issues --- */
    ert_j = pthread_cancel(data->client->thread_main);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_cancel");

    ert_j = pthread_cancel(data->client->thread_update);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_cancel");

    ert_j = pthread_join(data->client->thread_main, &ret);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_join");

    ert_j = pthread_join(data->client->thread_update, &ret);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_join");

    /* --- remove player from lists --- */
    printf("%3d - Removed player\n", (*data->print_num)++);

    data->game->remove_player(data->client->cli_id);

    data->client->removed = true;
    /* --- free data block since it wont be accesable --- */
    free(data);

    return NULL;
}

#endif // __SERVER_THREAD_FUNC_H__