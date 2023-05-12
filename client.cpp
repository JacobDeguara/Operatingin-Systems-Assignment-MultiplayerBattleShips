#ifndef __CLIENT_NETWORKING_H__
#define __CLIENT_NETWORKING_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <string_view>

#include "server_structs.hpp"
#include "client_display.cpp"
#include "client_thread_func.cpp"

thread_containter threads_global;

bool end_everything = false;
void ctrlhandler(int dummy)
{
    printf("press enter to exit:");
    end_everything = true;
}

class client
{
private:
    client_display *disp = new client_display();
    settings_request_playerlist srp;
    int general_settings(std::string str, int sfd, player_data *pd);
    void send_position(std::string pos_str, int cfd, bool *yourflag);
    bool does_player_exists(int cli_id);

    /* --- adding ships --- */
    void add_ship_general_settings(std::string ship_str, int sfd);
    char ship_str[7] = {'A', 'B', 'C', 'D', 'D', 'S', 'S'};
    int ship_count = 0;

    bool ended = true;
    bool your_turn_flag = false;
    bool ended2 = false; // these dont make any sense in boolean aspects but they work :^)
    std::vector<ship_placement> ship_list;

public:
    client(int argc, char *argv[]);
    void tcp_connect_server(int argc, char *argv[]);
    ~client();
};

client::client(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Missing ip address/portnum...\n");
        exit(0);
    }
}

void client::tcp_connect_server(int argc, char *argv[])
{

    /* --- Initilize variables --- */
    player_data pd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int cfd, ufd, sfd, portno, efd;

    /* --- initializing shared memory --- */
    tcp_client_bulk_data tcbd;
    tcbd.cfd = &(cfd);
    tcbd.sfd = &(sfd);
    tcbd.ufd = &(ufd);
    tcbd.disp = this->disp;
    tcbd.threadc = &threads_global;
    tcbd.ended = &(this->ended);
    bool your_flag = false;
    tcbd.your_turn_flag = &(your_flag);

    portno = atoi(argv[2]);

    /* --- create mutiple sockets ---*/
    printf("Createing socket... \n");
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("ERROR opening socket\n");
        return;
    }

    if ((cfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("ERROR opening socket\n");
        close(sfd);
        return;
    }

    if ((ufd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("ERROR opening socket\n");
        close(cfd);
        close(sfd);
        return;
    }

    /* --- search for server ---*/
    printf("Getting server name... \n");
    if ((server = gethostbyname(argv[1])) == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        close(ufd);
        close(cfd);
        close(sfd);
        exit(0);
    }

    // Populate serv_addr structure
    printf("Creating memory buffer... \n");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // Set to AF_INET
    memcpy(&serv_addr.sin_addr.s_addr,
           server->h_addr_list[0], // Set server address
           server->h_length);
    serv_addr.sin_port = htons(portno); // Set port (convert to network byte ordering)

    /* --- connect to server ---*/
    printf("Waiting for connection... \n");
    if (connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting \n");
        close(ufd);
        close(cfd);
        close(sfd);
        return;
    }

    /* --- connect each file descripter to server ---*/
    if (connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting \n");
        close(ufd);
        close(cfd);
        close(sfd);
        return;
    }

    if (connect(ufd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting \n");
        close(ufd);
        close(cfd);
        close(sfd);
        return;
    }

    /* --- get player name ---*/
    char name[15] = "no-name";
    printf("Write a username: ");
    scanf("%s", name); // if name goes greater then 15 it will break the server

    /* --- test correct fd connections ---*/
    if ((efd = write(cfd, &name, sizeof(name))) < 0)
    {
        perror("ERROR writing to socket \n");
        close(ufd);
        close(cfd);
        close(sfd);
        return;
    }

    if ((efd = write(sfd, &name, sizeof(name))) < 0)
    {
        perror("ERROR writing to socket \n");
        close(ufd);
        close(cfd);
        close(sfd);
        return;
    }

    if ((efd = write(ufd, &name, sizeof(name))) < 0)
    {
        perror("ERROR writing to socket \n");
        close(ufd);
        close(cfd);
        close(sfd);
        return;
    }

    /* --- get player data --- */
    efd = read(cfd, &pd, sizeof(player_data)); // should only have player name
    if (efd < 0)
        handle_error("read");

    disp->setup_us(pd);
    /* --- display player data ---*/
    if (pd.state == Spectator)
        printf("You are a Spectator, Player %s\n", pd.name);
    else if (pd.state == Not_Allowed)
    {
        printf("You are not allowed to connect, Player %s\n", pd.name);
        close(cfd);
        close(sfd);
        close(ufd);
        return;
    }
    else
        printf("You are a Player, Player %s\n", pd.name);

    this->ended = false;
    this->ended2 = true;

    tcbd.player_id = &pd;
    tcbd.srp = &(this->srp);

    if (pthread_create(&threads_global.update_thread, NULL, update_thread, &tcbd) != 0)
        handle_error("thread");

    if (pthread_create(&threads_global.main_thread, NULL, main_thread, &tcbd) != 0)
        handle_error("thread");

    if (pthread_create(&threads_global.collapse_thread, NULL, collapse_thread, &tcbd) != 0)
        handle_error("thread");

    signal(SIGINT, ctrlhandler); // this should never happen but just incase its here
    /* --- command prompt style input --- */
    while (1)
    {
        /* --- re-print input token --- */
        printf("\n>>> ");

        /* --- ctrl+c exit --- */
        if (end_everything)
        {
            break;
        }
        /* --- get input --- */
        std::string str = disp->getin();

        /* --- echo input --- */
        printf("\n-%s-\n", str.c_str());

        /* --- handle general settings --- */
        efd = general_settings(str, sfd, &pd);
        if (efd == -1)
        {
            break;
        }
        else if (efd == 1)
        {
            continue;
        }

        if (strncmp(str.c_str(), "hit", 3) == 0)
        {
            str.erase(0, 4);
            printf("-%s-\n", str.c_str());
            send_position(str, cfd, &your_flag);
        }

        // ship func to add ships
        if (strncmp(str.c_str(), "ship", 4) == 0)
        {
            str.erase(0, 5);
            printf("-%s-\n", str.c_str());
            add_ship_general_settings(str, sfd);
        }
    }
    if (ended == false)
    {
        efd = pthread_cancel(threads_global.update_thread);
        if (efd != 0)
            handle_error_ernum(efd, "pthread_cancel");
    }
    close(cfd);
    close(sfd);
    close(ufd);
}

int client::general_settings(std::string str, int sfd, player_data *pd)
{
    int req, efd;

    if (str.compare("exit") == 0)
    {
        req = -1;
        efd = write(sfd, &req, sizeof(req));
        return -1;
    }
    else if (str.compare("switch") == 0)
    {
        req = 1;
        if ((efd = write(sfd, &req, sizeof(req))) < 0)
            return -1;

        if ((efd = read(sfd, pd, sizeof(player_data))) < 0)
            return -1;

        if (pd->state == Spectator)
        {
            std::string msg = "You are a Spectator, Player ";
            msg.append(pd->name);
            disp->linger_message(msg);
        }
        else
        {
            std::string msg = "You are a Player, Player ";
            msg.append(pd->name);
            disp->linger_message(msg);
        }

        return 1;
    }
    else if (str.compare("update") == 0)
    {
        req = 0;
        if ((efd = write(sfd, &req, sizeof(req))) < 0)
            return -1;

        efd = (read(sfd, &srp.size, sizeof(int)));
        if (efd < 0)
            return -1;

        efd = (read(sfd, &srp.player_list, srp.size * sizeof(player_data)));
        if (efd < 0)
            return -1;

        disp->display_players(srp);

        return 1;
    }
    else if (str.compare("start") == 0)
    {
        req = 2;
        if ((efd = write(sfd, &req, sizeof(req))) < 0)
            return -1;

        int res;
        if (efd = read(sfd, &res, sizeof(int)) < 0)
            return -1;
        if (res == 0)
            disp->linger_message("Game wasn't started");
        else if (res == 1)
            disp->linger_message("Game started");

        return 1;
    }
    else if (str.compare("updateb") == 0)
    {
        req = 4;
        if ((efd = write(sfd, &req, sizeof(req))) < 0)
            return -1;

        settings_request_boardlist srb;

        efd = (read(sfd, &srb.size, sizeof(int)));
        if (efd < 0)
            return -1;

        efd = (read(sfd, &srb.board_list, srb.size * sizeof(bb)));
        if (efd < 0)
            return -1;

        disp->linger_message("Board Updated");

        disp->add_board(srb);
        return 1;
    }
    return 0;
}

void client::add_ship_general_settings(std::string ship_str, int sfd)
{
    if (ship_str.compare("clear") == 0)
    {
        ship_list.clear();
        disp->linger_message("cleared ship list");
        disp->copy_ship_list(ship_list);
        this->ship_count = 0;
        return;
    }

    if (ship_str.compare("default") == 0)
    {
        ship_placement sp;
        {
            sp.hor = true;
            sp.ship = 'A';
            sp.x = 0;
            sp.y = 0;
            ship_list.push_back(sp);

            sp.hor = true;
            sp.ship = 'B';
            sp.x = 0;
            sp.y = 1;
            ship_list.push_back(sp);

            sp.hor = true;
            sp.ship = 'C';
            sp.x = 0;
            sp.y = 2;
            ship_list.push_back(sp);

            sp.hor = true;
            sp.ship = 'D';
            sp.x = 0;
            sp.y = 3;
            ship_list.push_back(sp);

            sp.hor = true;
            sp.ship = 'S';
            sp.x = 0;
            sp.y = 4;
            ship_list.push_back(sp);

            sp.hor = true;
            sp.ship = 'D';
            sp.x = 0;
            sp.y = 5;
            ship_list.push_back(sp);

            sp.hor = true;
            sp.ship = 'S';
            sp.x = 0;
            sp.y = 6;
            ship_list.push_back(sp);
        }
        disp->copy_ship_list(ship_list);
        this->ship_count = 7;
        return;
    }

    if (ship_str.compare("check") == 0)
    {
        int res = ship_format_confirmer(ship_list);

        if (res == 0)
            disp->linger_message("ships are in correct format");
        else if (res == 1)
            disp->linger_message("ships not correct format: Out of bounce error");
        else if (res == 2)
            disp->linger_message("ships not correct format: Overlapping error");
    }

    if (ship_str.compare("push") == 0)
    {
        if (ship_list.size() != 7)
        {
            disp->linger_message("cant push since ship list has to have 7 ships");
            return;
        }

        int req = 3, efd;
        if ((efd = write(sfd, &req, sizeof(req))) < 0)
            return;

        ship_placement ship_list_arr[7];
        std::copy(ship_list.begin(), ship_list.end(), ship_list_arr);

        if ((efd = write(sfd, (char *)&ship_list_arr, sizeof(ship_placement) * 7)) < 0)
            return;

        int res;
        if (efd = read(sfd, &res, sizeof(int)))
            return;

        if (res == 0)
            disp->linger_message("ships added to game");
        else if (res == 1)
            disp->linger_message("ships not added: not correct format (Out of bounce error)");
        else if (res == 2)
            disp->linger_message("ships not added: not correct format (Overlapping error)");
        else if (res == -1)
            disp->linger_message("ships not added: game not waiting for ships to be added");
        else
            disp->linger_message("unknown error");
    }

    if (check_format(ship_str) == 0)
    {
        // here you can check the ship again before it has to be push and then cleared again and stuff
        // using ship_format_confirmer()
        if (ship_list.size() < 7)
        {
            ship_placement sp;
            sp.ship = this->ship_str[ship_count];
            sp.x = char_pos(ship_str.at(1));
            sp.y = atoi(&ship_str.at(0));
            if (ship_str.at(2) == 'h')
                sp.hor = true;
            else
                sp.hor = false;

            ship_list.push_back(sp);
            this->ship_count++;
            disp->linger_message("Added ship to ship list");
            disp->copy_ship_list(ship_list);
        }
    }
    else if (check_format(ship_str) == 1)
    {
        disp->linger_message("ship func wrong format: wrong size");
    }
    else if (check_format(ship_str) == 2)
    {
        disp->linger_message("ship func wrong format: first letter [0-9]'");
    }
    else if (check_format(ship_str) == 3)
    {
        disp->linger_message("ship func wrong format: second letter [A-J]'");
    }
    else if (check_format(ship_str) == 4)
    {
        disp->linger_message("ship func wrong format: third letter [h|v]'");
    }
}

void client::send_position(std::string pos_str, int cfd, bool *yourflag)
{
    // [0-9] [A-J] [board num]

    if (*yourflag == false)
    {
        disp->linger_message("You cannot send a shot since its not your turn");
        return;
    }

    int check = check_format_hit(pos_str);

    if (check == 1)
    {
        disp->linger_message("Wrong format: must be greater then 2 characters");
        return;
    }
    else if (check == 2)
    {
        disp->linger_message("Wrong format: must start with [0-9]");
        return;
    }
    else if (check == 3)
    {
        disp->linger_message("Wrong format: second character must be [A-J]");
        return;
    }
    else if (check == 4)
    {
        disp->linger_message("Wrong format: thrid charcater onwards (cli_id) must be numbers");
        return;
    }

    cord_board cb;
    cb.x = char_pos(pos_str.at(1));
    cb.y = atoi(&pos_str.at(0));

    int cli_id;
    std::string cli_id_sting;
    cli_id_sting = pos_str;
    cli_id_sting.erase(0, 2);
    cli_id = std::stoi(cli_id_sting);
    cb.cli_id = cli_id;

    if (!does_player_exists(cli_id))
    {
        disp->linger_message("player with cli_id " + std::to_string(cb.cli_id) + " doesnt exists or is dead or is spectator idk...");
        return;
    }

    std::string str = "Hitting position x:" + std::to_string(cb.x) + " y:" + std::to_string(cb.y) + " board:" + std::to_string(cb.cli_id);
    disp->linger_message(str);

    /* --- send pos to game --- */
    int efd;
    efd = write(cfd, (char *)&cb, sizeof(cord_board));
    if (efd > 0)
        return;

    *yourflag = false;
}

client::~client()
{
    /* --- wait until collapse thread closes --- */
    int s;
    void *ret;
    if (ended2 == true)
    {
        s = pthread_join(threads_global.collapse_thread, &ret); // while true loop
        if (s != 0)
            handle_error_ernum(s, "pthread_join- update");
    }

    printf("Program Closed...\n");

    free(disp);
}

bool client::does_player_exists(int cli_id)
{
    for (size_t i = 0; i < srp.size; i++)
    {
        if (srp.player_list[i].state == Player || srp.player_list[i].state == AI || srp.player_list[i].state == Alive)
        {
            if (srp.player_list[i].cli_id == cli_id)
            {
                return true;
            }
        }
    }
    return false;
}

#endif // __CLIENT_NETWORKING_H__