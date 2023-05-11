#ifndef __SERVER_BATTLESHIPS_H__
#define __SERVER_BATTLESHIPS_H__

#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

#include "server_Battleships_struct.hpp"

#define MAXPLAYERS 4
#define MAXSPECTATORS 26

enum game_states
{
    start,
    ship,
    game,
    end
};

class Battleships
{
private:
    std::vector<player_data> players;
    std::vector<player_data> spectators;
    std::vector<multiple_ship_checker> ship_checker;

    std::vector<bb> board_list;
    std::vector<bb> board_list_show;

    game_states state = start;

public:
    /* --- setting up --- state: START --- */

    player_data add_player(char name[15], int cli_id); // add a player to Battleships
    player_data switch_player(int cli_id);             // switch player -  player <=> spectator
    player_data remove_player(int cli_id);             // remove player
    void display_players();                            // prints each player & spectator to the terminal

    /* --- Game_start --- state: SHIP --- */

    bool game_start_ship(int cli_id);                                  // changes game state to SHIP, only host (first player) can.
    bool add_ships(std::vector<ship_placement> ship_list, int cli_id); // adds ships to ship counter, returns false => 1. state != SHIP or 2. ship not correct format (error should be displayed properly by server/client)

    /* --- Game_loop --- state: GAME --- */

    bool game_start();     // changes game state to GAME, only if each player has their ships placed.
    void display_boards(); // print each of the boards to the terminal
    void bomb_space();
    void set_player_AI();
    void run_KURT();

    /* --- Game_ended --- state: END --- */

    /* --- Default Values --- */

    int *print_num;
    Battleships(int *print_num);
    ~Battleships();

    /* --- Utilities --- */

    player_data find_player(int cli_id);        // returns the players player data based on their client_id
    bool player_exists(char name[15]);          // returns true only if the player with @name exists
    std::vector<player_data> get_players_all(); // returns the vector of player & spectators (players_vec + spectators_vec)
    int get_game_state();                       // returns 0-START, 1-SHIP, 2-GAME, 3-END
    void clear_board(bb *board, int cli_id);    // clears the board passed with '-' and adds @cli_id added
    std::vector<bb> get_show_boards();
};

Battleships::Battleships(int *print_num)
{
    print_line();
    this->print_num = print_num;
    printf("%3d - Game Created\n", (*print_num)++); // this wierd assortments of symbles increments the value and returns the value
    print_line();
}

player_data Battleships::add_player(char name[15], int cli_id) // should always give a unique name & client_id since it done on server
{
    print_line();
    player_data temp;
    temp.cli_id = cli_id;
    strcpy(temp.name, name);

    /* --- add players during state start only---*/
    if (state == start)
    {
        if (players.size() < MAXPLAYERS)
        {
            temp.state = Player;
            players.push_back(temp);

            printf("%3d - Added as Player %15s , %d \n", (*print_num)++, name, cli_id);
            print_line();

            return temp;
        }
    }
    /* --- else spectator unless, MAXSPECTATORS ACHIVED---*/
    if (spectators.size() < MAXSPECTATORS)
    {
        temp.state = Spectator;
        spectators.push_back(temp);

        printf("%3d - Added as Spectator %15s , %d \n", (*print_num)++, name, cli_id);
        print_line();

        return temp;
    }

    temp.state = Not_Allowed;

    printf("%3d - Player %15s , %d Wasn't Allowed (Server => FULL)\n", (*print_num)++, name, cli_id);
    print_line();

    return temp;
}

player_data Battleships::switch_player(int cli_id)
{
    print_line();
    auto id = find_player(cli_id);

    /* --- players can only switch during state start only --- */
    if (state == start)
    {
        // check what state, then check if possible then switch
        if (id.state == Player) // id -> player
        {
            if (spectators.size() < MAXSPECTATORS) // less then MAXSPECS
            {
                for (size_t i = 0; i < players.size(); i++)
                {
                    if (players.at(i).cli_id == id.cli_id)
                    {
                        players.erase(players.begin() + i); // remove player from vector
                        break;
                    }
                }

                id.state = Spectator;
                spectators.push_back(id); // add player to spectators

                printf("%3d - Player %15s , %d is now a Spectator\n", (*print_num)++, id.name, id.cli_id);
                print_line();
                return id;
            }
        }
        else if (id.state == Spectator) // spectator (NOT_ALLOWED is to never gain control)
        {
            if (players.size() < MAXPLAYERS)
            {
                for (size_t i = 0; i < spectators.size(); i++)
                {
                    if (spectators.at(i).cli_id == id.cli_id)
                    {
                        spectators.erase(spectators.begin() + i); // remove spectator from vector
                        break;
                    }
                }

                id.state = Player;
                players.push_back(id);

                printf("%3d - Player %15s , %d is now a Player\n", (*print_num)++, id.name, id.cli_id);
                print_line();
                return id;
            }
        }
    }
    printf("%3d - Player %15s , %d state wasnt changed\n", (*print_num)++, id.name, id.cli_id);
    print_line();
    return id;
}

player_data Battleships::remove_player(int cli_id)
{
    print_line();
    auto id = find_player(cli_id);

    if (id.state == Player) // id -> player
    {
        if (players.size() < MAXPLAYERS)
        {
            if (state == start)
            {
                for (size_t i = 0; i < players.size(); i++)
                {
                    if (players.at(i).cli_id == id.cli_id)
                    {
                        players.erase(players.begin() + i); // remove player from vector

                        printf("%3d - Player %15s , %d Removed\n", (*print_num)++, id.name, id.cli_id);
                        print_line();
                        return id;
                    }
                }
            }
            else
            {
                /* --- set player to dead or KURT idk whatever seems easier --- */
            }
        }
    }

    if (id.state == Spectator)
    {
        if (spectators.size() < MAXSPECTATORS) // less then MAXSPECS
        {
            for (size_t i = 0; i < spectators.size(); i++)
            {
                if (spectators.at(i).cli_id == id.cli_id)
                {
                    spectators.erase(spectators.begin() + i); // remove spectator from vector

                    printf("%3d - Player %15s , %d Removed\n", (*print_num)++, id.name, id.cli_id);
                    print_line();
                    return id;
                }
            }
        }
    }

    // this shouldn't happen
    id.state = Not_Allowed;
    printf("%3d - Player %15s , %d wasn't Removed\n", (*print_num)++, id.name, id.cli_id);
    print_line();
    return id;
}

void Battleships::display_players()
{
    print_line();

    printf("Players: \n");
    for (size_t i = 0; i < players.size(); i++)
    {
        printf("> %15s , %d \n", players.at(i).name, players.at(i).cli_id);
    }

    printf("Spectators: \n");
    for (size_t i = 0; i < spectators.size(); i++)
    {
        printf("- %15s , %d \n", spectators.at(i).name, spectators.at(i).cli_id);
    }

    print_line();
}

bool Battleships::game_start_ship(int cli_id)
{
    if (state == start)
    {
        print_line();
        if (players.size() > 1)
        {
            if (players.at(0).cli_id == cli_id)
            {
                this->state = ship;
                print_line();
                printf("Host Started the Game -> state changed to SHIP\n");
                print_line();
                return true;
            }
        }
        printf("%3d - Other player tried to started the Game -> state still at START\n", (*print_num)++);
        print_line();
    }
    return false;
}

bool Battleships::add_ships(std::vector<ship_placement> ship_list, int cli_id)
{
    /* --- this will only happen if we are in ship state --- */
    if (state == ship)
    {
        print_line();
        for (size_t i = 0; i < ship_checker.size(); i++)
        {
            if (ship_checker.at(i).cli_id == cli_id) // if a player re-submitted a ship they can replace it
            {
                ship_checker.erase(ship_checker.begin() + i);
                printf("%3d - Removed ship_list of player %d\n", (*print_num)++, cli_id);
            }
        }

        multiple_ship_checker msc;
        msc.add_ships(ship_list, cli_id);
        ship_checker.push_back(msc);
        return true;
        printf("%3d - Added new ships to ship_list for player %d\n", (*print_num)++, cli_id);
        print_line();
    }
    return false;
}

bool Battleships::game_start()
{
    /* --- can only start if ship state AND ship_list.size() == player_list.size() (each player has put their ships) --- */
    if (state != ship)
        return false;

    if (ship_checker.size() != players.size())
        return false;

    print_line();
    /* --- sort the ship_checker and players_list so at i then can match the cli_id --- */
    std::sort(ship_checker.begin(), ship_checker.end(),
              [](const multiple_ship_checker &left, const multiple_ship_checker &right)
              { return (left.cli_id < right.cli_id); });

    std::sort(players.begin(), players.end(),
              [](const player_data &left, const player_data &right)
              { return (left.cli_id < right.cli_id); });

    /* --- create the boards & fill them up --- */
    for (size_t i = 0; i < players.size(); i++)
    {
        bb board;
        clear_board(&board, players.at(i).cli_id);
        board_list.push_back(board);
        board_list_show.push_back(board);
    }

    /* --- add ships to board_list --- */
    for (size_t i = 0; i < ship_checker.size(); i++) // each ship_list per player
    {
        for (size_t j = 0; j < ship_checker.at(i).ship_checker_list.size(); j++) // each ship per player
        {
            for (size_t k = 0; k < ship_checker.at(i).ship_checker_list.at(j).cords.size(); k++) // each pos per ship
            {
                int x = ship_checker.at(i).ship_checker_list.at(j).cords.at(k).x;
                int y = ship_checker.at(i).ship_checker_list.at(j).cords.at(k).y;
                board_list.at(i).board[x][y] = ship_checker.at(i).ship_checker_list.at(j).ship;
            }
        }
    }
    this->state = game;
    printf("%3d - Game state changed to GAME game has started\n", (*print_num)++);
    print_line();
    display_boards();
    return true;
}

void Battleships::display_boards()
{
    if (state != game)
        return;

    print_line();
    for (size_t i = 0; i < players.size(); i++)
    {
        printf("   A B C D E F G H I J  ");
    }
    printf("\n");

    for (int y = 0; y < 10; y++)
    {
        for (int pos = 0; pos < players.size(); pos++)
        {
            printf("%2d ", (y + 1));

            for (int x = 0; x < 10; x++)
            {
                printf("%c ", board_list.at(pos).board[x][y]);
            }
            printf(" ");
        }
        printf("\n");
    }
    printf("\n");

    for (int i = 0; i < players.size(); i++)
    {
        printf(" Player: %-15s", players.at(i).name);
    }

    printf("\n");
    printf("\n");
    printf("What Players See: \n");

    for (size_t i = 0; i < players.size(); i++)
    {
        printf("   A B C D E F G H I J  ");
    }
    printf("\n");

    for (int y = 0; y < 10; y++)
    {
        for (int pos = 0; pos < players.size(); pos++)
        {
            printf("%2d ", (y + 1));

            for (int x = 0; x < 10; x++)
            {
                printf("%c ", board_list_show.at(pos).board[x][y]);
            }
            printf(" ");
        }
        printf("\n");
    }
    printf("\n");

    for (int i = 0; i < players.size(); i++)
    {
        printf(" Player: %-15s", players.at(i).name);
    }
    printf("\n");
    print_line();

    printf("\n");
    for (size_t x = 0; x < 10; x++)
    {
        for (size_t y = 0; y < 10; y++)
        {
            printf("%c ", board_list.at(0).board[x][y]);
        }
        printf("\n");
    }
    printf("\n");
}

Battleships::~Battleships()
{
    print_line();
    printf("%3d - Game Ended\n", (*print_num)++);
    print_line();
}

player_data Battleships::find_player(int cli_id)
{
    for (size_t i = 0; i < players.size(); i++)
    {
        if (players.at(i).cli_id == cli_id)
        {
            return players.at(i);
        }
    }

    for (size_t i = 0; i < spectators.size(); i++)
    {
        if (spectators.at(i).cli_id == cli_id)
        {
            return spectators.at(i);
        }
    }

    /* ERROR situation */
    player_data temp;
    temp.cli_id = cli_id;
    temp.state = Not_Allowed;
    strcpy(temp.name, "UNKOWN");
    return temp;
}

bool Battleships::player_exists(char name[15])
{
    for (size_t i = 0; i < players.size(); i++)
    {
        if (strcmp(players.at(i).name, name) == 0)
        {
            return true;
        }
    }

    for (size_t i = 0; i < spectators.size(); i++)
    {
        if (strcmp(spectators.at(i).name, name) == 0)
        {
            return true;
        }
    }
    return false;
}

std::vector<player_data> Battleships::get_players_all()
{
    std::vector<player_data> temp_list = players;

    temp_list.insert(temp_list.begin(), spectators.begin(), spectators.end());

    return temp_list;
}

int Battleships::get_game_state()
{
    switch (this->state)
    {
    case start:
        return 0;
    case ship:
        return 1;
    case game:
        return 2;
    case end:
        return 3;
    default:
        return -1;
        break;
    }
};

void Battleships::clear_board(bb *board, int cli_id)
{
    for (size_t i = 0; i < 10; i++)
    {
        for (size_t j = 0; j < 10; j++)
        {
            board->board[i][j] = '-';
        }
    }
    board->cli_id = cli_id;
}

std::vector<bb> Battleships::get_show_boards()
{
    return board_list_show;
}

#endif // __SERVER_BATTLESHIPS_H__