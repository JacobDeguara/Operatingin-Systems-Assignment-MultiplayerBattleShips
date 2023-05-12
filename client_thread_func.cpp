#ifndef __CLIENT_THREAD_FUNC_H__
#define __CLIENT_THREAD_FUNC_H__

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>

#include "server_structs.hpp"
#include "client_display.cpp"

void *update_thread(void *arg);
void *collapse_thread(void *arg);
void *main_thread(void *arg);

void *update_thread(void *arg)
{
    /* --- set cancelable state --- */
    int ert_j;
    ert_j = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_setcancelstate");

    /* --- Initilize variables --- */
    tcp_client_bulk_data *data = (tcp_client_bulk_data *)arg;
    settings_request_playerlist srp;
    int efd;
    while (1)
    {
        // check if client has left
        if (recv(*data->ufd, NULL, 1, MSG_PEEK | MSG_DONTWAIT) == 0)
        {
            data->disp->linger_message("SERVER DOWN: write 'exit' to exit program");
            break;
        }

        efd = (read(*data->ufd, &srp.size, sizeof(int)));
        if (efd < 0)
            handle_error("read");

        efd = (read(*data->ufd, &srp.player_list, srp.size * sizeof(player_data)));
        if (efd < 0)
            handle_error("read");

        data->disp->display_players(srp);
    }

    data->disp->display_players(srp);
    *data->ended = true;
    return NULL;
}

void *collapse_thread(void *arg)
{
    tcp_client_bulk_data *data = (tcp_client_bulk_data *)arg;
    int ert_j; // er|rror t|hread_j|oin => ert_j

    void *ret;
    ert_j = pthread_join(data->threadc->update_thread, &ret);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_join");

    ert_j = pthread_cancel(data->threadc->main_thread);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_cancel");

    ert_j = pthread_join(data->threadc->main_thread, &ret);
    if (ert_j != 0)
        handle_error_ernum(ert_j, "pthread_join");

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
    tcp_client_bulk_data *data = (tcp_client_bulk_data *)arg;
    int efd;

    fd_set rfds;
    int retval;
    FD_ZERO(&rfds);
    FD_SET(*data->cfd, &rfds);
    int meta_data_game_started = -1;

    retval = select(1024, &rfds, NULL, NULL, NULL);

    if (retval == -1)
        perror("select()");
    else if (retval)
    {
        efd = (read(*data->cfd, &meta_data_game_started, sizeof(int)));
        if (efd < 0)
            handle_error("read");

        data->disp->important_message("GAME STARTED waiting for ships");
    }

    retval = select(1024, &rfds, NULL, NULL, NULL);
    if (retval == -1)
        perror("select()");
    else if (retval)
    {
        efd = (read(*data->cfd, &meta_data_game_started, sizeof(int)));
        if (efd < 0)
            handle_error("read");

        data->disp->important_message("GAME STARTED");
    }

    do
    {
        retval = select(1024, &rfds, NULL, NULL, NULL);

        /* --- read given package --- */
        int temp = -1;
        efd = read(*data->cfd, &temp, sizeof(int));
        if (efd < 0)
            handle_error("read");

        /* ---  update boards --- */
        settings_request_boardlist srb;

        efd = (read(*data->cfd, &srb.size, sizeof(int)));
        if (efd < 0)
            handle_error("read");

        efd = (read(*data->cfd, &srb.board_list, srb.size * sizeof(bb)));
        if (efd < 0)
            handle_error("read");

        data->disp->linger_message("Board Updated");

        data->disp->add_board(srb);

        if (temp == data->player_id->cli_id)
        {
            data->disp->important_message(" your turn ");
            data->disp->linger_message(std::to_string(temp));
        }
        else
        {
            data->disp->important_message(" Other player turn");
            data->disp->linger_message(std::to_string(temp));
        }
    } while (1);

    while (1)
    {
        sleep(1);
    }
    return NULL;
}
#endif // __CLIENT_THREAD_FUNC_H__