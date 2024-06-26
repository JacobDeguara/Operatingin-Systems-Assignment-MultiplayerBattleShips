#ifndef __CLIENT_DISPLAY_H__
#define __CLIENT_DISPLAY_H__

#include <stdio.h>
#include <termios.h> //termios, TCSANOW, ECHO, ICANON
#include <unistd.h>  //STDIN_FILENO

#include "server_structs.hpp"

#define MAXLINGER 10

std::string get_player_state_as_string(status s);

class client_display
{
private:
    std::string command;

    std::string imp_msg;
    std::string msg;
    int count = 0;

    std::vector<bb> boards;
    bool board_flag = false;

    player_data player;
    std::vector<ship_placement> ship_list;

public:
    client_display(/* args */);
    ~client_display();

    void linger_message(std::string msg);
    void important_message(std::string msg);
    void display_players(settings_request_playerlist srp);
    void setup_us(player_data pd);
    void add_board(settings_request_boardlist srb);
    std::string getin();
    void copy_ship_list(std::vector<ship_placement> copy);
};

client_display::client_display()
{
}

void client_display::setup_us(player_data pd)
{
    this->player = pd;
}

void client_display::display_players(settings_request_playerlist srp)
{
    system("clear");
    printf("\n");
    printf("--- %s ---\n", imp_msg.c_str());
    printf("-You are a %s, %s ,%d\n", get_player_state_as_string(player.state).c_str(), player.name, player.cli_id);
    print_line();
    printf("Currently connected players:\n");

    size_t i = 0;
    printf("Players:\n");
    for (; i < srp.size;)
    {
        if (srp.player_list[i].state != Spectator)
        {
            printf("> %-15s %d %s\n", srp.player_list[i].name, srp.player_list[i].cli_id, get_player_state_as_string(srp.player_list[i].state).c_str());
        }
        else
        {
            break;
        }
        i++;
    }
    printf("Spectators:\n");
    for (; i < srp.size; i++)
    {
        printf("> %-15s %d %s\n", srp.player_list[i].name, srp.player_list[i].cli_id, get_player_state_as_string(srp.player_list[i].state).c_str());
    }
    print_line();
    if (ship_list.size() != 0)
    {
        for (size_t i = 0; i < ship_list.size(); i++)
        {
            printf("Ship %c: x:%d y:%d ,hor: %d\n", ship_list.at(i).ship, ship_list.at(i).x, ship_list.at(i).y, ship_list.at(i).hor);
        }
        print_line();
    }

    if (board_flag)
    {
        print_line();
        for (size_t i = 0; i < boards.size(); i++)
        {
            printf("   A B C D E F G H I J  ");
        }
        printf("\n");

        for (int y = 0; y < 10; y++)
        {
            for (int pos = 0; pos < boards.size(); pos++)
            {
                printf("%2d ", (y));

                for (int x = 0; x < 10; x++)
                {
                    printf("%c ", boards.at(pos).board[x][y]);
                }
                printf(" ");
            }
            printf("\n");
        }
        printf("\n");

        for (int i = 0; i < boards.size(); i++)
        {
            printf(" Player: %-15s %d", srp.player_list[i].name, srp.player_list[i].cli_id);
        }
        printf("\n");
    }

    if (count > 0)
    {
        printf("- %s -\n\n", msg.c_str());
        count--;
    }
    printf("\n>>> %s", command.c_str());
    fflush(stdout);
}

void client_display::add_board(settings_request_boardlist srb)
{
    boards.clear();

    for (size_t i = 0; i < srb.size; i++)
    {
        boards.push_back(srb.board_list[i]);
    }

    board_flag = true;
}

void client_display::linger_message(std::string msg)
{
    this->msg = msg;
    count = MAXLINGER;
}

void client_display::important_message(std::string msg)
{
    this->imp_msg = msg;
}

std::string client_display::getin()
{
    char c;
    command.clear();

    static struct termios oldt, newt;

    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr(STDIN_FILENO, &oldt);
    /*now the settings will be copied*/
    newt = oldt;

    /*ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON | ECHO);

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while ((c = getchar()) != '\n')
    {
        if (c == '-')
        {
            if (command.size() > 0)
            {
                command.pop_back();
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        }
        else
        {
            command.append(&c);
            putchar(c);
        }
    }

    /*restore the old settings*/
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return command;
}

void client_display::copy_ship_list(std::vector<ship_placement> copy)
{
    this->ship_list = copy;
}

std::string get_player_state_as_string(status s)
{
    switch (s)
    {
    case Player:
        return "Player";
    case Alive:
        return "Alive";
    case Dead:
        return "Dead";
    case AI:
        return "AI";
    case Spectator:
        return "Spectator";
    default:
        return " ";
    }
}

#endif // __CLIENT_DISPLAY_H__