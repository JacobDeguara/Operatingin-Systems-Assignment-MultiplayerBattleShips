#include <vector>
#include <string>
#include <tuple>
#include <iostream>
#include <cstdlib>
#include <random>

#include "Battleship_Stucts.hpp"

using namespace std;

class Battleships
{
private:
    vector<player_id> players;
    vector<player_id_minimal> players_copy;
    vector<ship_counter_multiple> ship_checker;
    vector<player_id> spectators;

    int current_player; // -1 -> 0 first player
    bool game_started = false;

public:
    Battleships();

    // print each of the boards
    void print_boards();

    // Adds players to the game up to 4
    player_id add_player(string name);
    // removes players from the game
    void remove_player(string name);

    void remove_spectator(string name);

    // finds the player with that name and provides an id (since startGame sets up ids there are no ids after game start)
    int find_player(string name);
    // Adds ships to the player
    void add_ships(vector<shipp> ship_list, string name);

    // starts the game and disables some of the functionallity (add_players, remove_players)
    vector<player_id> startGame();

    // returns the next players turn
    int next_turn();

    // sets player with -id- as dead
    void set_player_dead(int id);

    // sets player with -id- as Ai
    void set_player_KURT(int id);

    // bombs a space and changes the map
    void bomb_space(coordinates cords);

    // returns the players_copy vector
    vector<player_id_minimal> get_players_boards();

    // gets current player
    int get_current_player();

    // sets current player to cp_new
    void set_current_player(int cp_new);

    // runs K.U.R.T
    void run_KURT();

    // check if only one player is left standing (if true returns true and the players id)
    std::tuple<bool, int> check_end_game();

    vector<player_id> get_players_info();
    vector<player_id> get_spectators();

    ~Battleships();
};

void print_line()
{
    printf("===============================================================================================\n");
}

std::tuple<bool, int> check_end_game()
{

    return {false, -1};
}

int Battleships::find_player(string name)
{
    for (size_t i = 0; i < players.size(); i++)
    {
        if (players.at(i).name.compare(name) == 0)
        {
            return i;
        }
    }
    return -1;
}

Battleships::Battleships()
{
    print_line();
    printf("Created Game Element\n");
    print_line();
}

vector<player_id> Battleships::startGame()
{
    current_player = -1;

    for (size_t i = 0; i < players.size(); i++)
    {
        ship_counter_multiple ship_list_checker;
        players.at(i).id = i;
        ship_list_checker.add_ships(players.at(i).list);
        ship_checker.push_back(ship_list_checker);
    }

    game_started = true;

    player_id_minimal pim;

    for (size_t i = 0; i < players.size(); i++)
    {
        pim.copy_player_id(players.at(i));
        players_copy.push_back(pim);
    }

    printf("Game Has Started -inistilised variables- & disabled add_player(), add_ships(), remove_player()\n");
    print_line();
    print_boards();
    return players;
}

player_id Battleships::add_player(string name)
{
    player_id id;
    id.name.append(name);
    // does name exists
    for (size_t i = 0; i < players.size(); i++)
    {
        if (players.at(i).name.compare(name) == 0)
        {
            id.status_type = Not_Allowed;
            id.confirmation = false;
            return id;
        }
    }
    // is there already enough players
    if (players.size() >= 4 || game_started)
    {
        id.status_type = Not_Allowed;
        id.confirmation = true;
        spectators.push_back(id);
        return id;
    }

    // create player
    id.status_type = Alive;
    id.confirmation = true;
    id.clear_board();
    players.push_back(id);

    printf("Added Player: %s \n", id.name.c_str());
    print_line();
    print_boards();
    return id;
}

void Battleships::remove_player(string name)
{
    if (!game_started)
    {
        for (size_t i = 0; i < players.size(); i++)
        {
            if (players.at(i).name.compare(name) == 0)
            {
                players.erase(players.begin() + i);
                printf("Player %s was removed", name.c_str());
                print_line();
                print_boards();
                return;
            }
        }
    }
}

void Battleships::remove_spectator(string name)
{
    for (size_t i = 0; i < spectators.size(); i++)
    {
        if (spectators.at(i).name.compare(name) == 0)
        {
            spectators.erase(spectators.begin() + i);
            printf("Spectator %s was removed", name.c_str());
            print_line();
            print_boards();
            return;
        }
    }
}

void Battleships::print_boards()
{
    for (int i = 0; i < players.size(); i++)
    {
        if (players[i].status_type != Not_Allowed)
        {
            printf("   A B C D E F G H I J  ");
        }
    }
    printf("\n");

    for (int i = 0; i < 10; i++)
    {
        for (int pos = 0; pos < players.size(); pos++)
        {
            if (players.at(pos).status_type != Not_Allowed)
            {
                printf("%2d ", (i + 1));

                for (int j = 0; j < 10; j++)
                {
                    printf("%c ", players.at(pos).board[i][j]);
                }
                printf(" ");
            }
        }
        printf("\n");
    }
    printf("\n");
    for (int i = 0; i < players.size(); i++)
    {
        if (players[i].status_type != Not_Allowed)
        {
            printf(" Player: %-15s", players[i].name.c_str());
        }
    }

    if (game_started)
    {
        printf("\n");
        printf("\n");
        printf("What Players See: \n");
        for (int i = 0; i < players_copy.size(); i++)
        {
            if (players_copy[i].status_type != Not_Allowed)
            {
                printf("   A B C D E F G H I J  ");
            }
        }
        printf("\n");

        for (int i = 0; i < 10; i++)
        {
            for (int pos = 0; pos < players_copy.size(); pos++)
            {
                if (players_copy[pos].status_type != Not_Allowed)
                {
                    printf("%2d ", (i + 1));

                    for (int j = 0; j < 10; j++)
                    {
                        printf("%c ", players_copy.at(pos).board[i][j]);
                    }
                    printf(" ");
                }
            }
            printf("\n");
        }
        printf("\n");
        for (int i = 0; i < players_copy.size(); i++)
        {
            if (players_copy[i].status_type != Not_Allowed)
            {
                printf(" Player: %-15s", players_copy.at(i).name);
            }
        }
    }

    printf("\n");
    print_line();
}

void Battleships::add_ships(vector<shipp> ship_list, string name)
{
    if (game_started)
    {
        return;
    }

    if (ship_list.size() != 7)
    {
        return;
    }

    int pos = find_player(name);

    for (size_t i = 0; i < ship_list.size(); i++)
    {
        for (size_t j = 0; j < size_of_ship(ship_list.at(i).ship); j++)
        {
            int xplus = 0;
            int yplus = 0;

            if (ship_list.at(i).hor)
                yplus = j;
            else
                xplus = j;

            players.at(pos).board[ship_list.at(i).x + xplus][ship_list.at(i).y + yplus] = ship_list.at(i).ship;
        }
    }

    players.at(pos).list = ship_list;
    printf("Player: %s has added ships to his board\n", name.c_str());
    print_line();
    print_boards();
}

void Battleships::bomb_space(coordinates cords)
{
    int x = cords.x;
    int y = cords.y;
    int board = cords.board;

    printf("Hit postion %d,%d on board %d \n", x, y, board);

    if (players.at(board).status_type == Not_Allowed || players.at(board).status_type == Dead)
    {
        printf("Player is already dead-\n");
        print_line();
        return;
    }

    if (board == current_player)
    {
        printf("Why are you hitting yourself ?-\n");
        set_current_player(current_player - 1);
        print_line();
        return;
    }

    int c = players_copy.at(board).board[x][y]; // - | O | X | F

    printf("Hit position is %c\n", players_copy.at(board).board[x][y]);

    if (c == 'O' || c == 'X' || c == 'F')
    {
        printf("Hit Was Already Hit? -THIS SHOULDNT HAPPEN !!HACKER!!-\n");
        set_current_player(current_player - 1);
        print_line();
        return;
    }
    else if (c == '-')
    {
        c = players.at(board).board[x][y]; // - | A - S (ships)

        if (c == '-')
        {
            players_copy.at(board).board[x][y] = 'O';
            printf("Hit was a miss.\n");
        }
        else
        {
            if (ship_checker.at(board).bomb_ship_hit(x, y, c))
            {
                printf("Hit was a hit.\n");
                players_copy.at(board).board[x][y] = 'X';

                if (ship_checker.at(board).ship_sunk(x, y, c))
                {
                    printf("Hit has sunk a ship.\n");
                    auto xs = ship_checker.at(board).get_x_list(x, y, c);
                    auto ys = ship_checker.at(board).get_y_list(x, y, c);

                    for (size_t i = 0; i < xs.size(); i++)
                    {
                        players_copy.at(board).board[xs.at(i)][ys.at(i)] = 'F';
                    }
                }

                // we have to give the player another shot now!
                set_current_player(current_player - 1);
                // 0 -> -1 , 3 -> 2 | meaning when we do | -1++ -> 0 , 2++ -> 3 | which is their turn again
            }
        }

        if (ship_checker.at(board).all_ships_sunk())
        {
            printf("All ships were sunk.\n");
            set_player_dead(board);
            return;
        }
    }

    print_line();
    print_boards();
}

int Battleships::next_turn()
{
    int cp = current_player;

    for (size_t i = 0; i < players.size(); i++)
    {
        cp++;

        if (cp >= players.size())
        {
            cp = 0;
        }

        if (players.at(cp).status_type != Dead && players.at(cp).status_type != Not_Allowed) // false => when player is dead or Not Allowed/ true => when player isnt dead or
        {
            break;
        }
    }

    printf("Player turn changed to player %s %d.\n", players.at(cp).name.c_str(), cp);
    set_current_player(cp);
    print_line();

    return current_player;
}

void Battleships::set_player_dead(int id)
{
    if (id > players.size())
    {
        return;
    }

    players.at(id).status_type = Dead;
    players_copy.at(id).status_type = Dead;

    printf("Player %s %d is now dead.\n", players.at(id).name.c_str(), id);

    for (size_t i = 0; i < ship_checker.at(id).ship_lists.size(); i++)
    {
        if (ship_checker.at(id).ship_lists.at(i).count != 0)
        {
            auto ys = ship_checker.at(id).ship_lists.at(i).y;
            auto xs = ship_checker.at(id).ship_lists.at(i).x;

            for (size_t i = 0; i < xs.size(); i++)
            {
                players_copy.at(id).board[xs.at(i)][ys.at(i)] = 'F';
            }
        }
    }

    print_line();
    print_boards();
}

void Battleships::set_player_KURT(int id)
{
    if (id > players.size())
    {
        return;
    }

    if (players.at(id).status_type == Dead || players.at(id).status_type == Not_Allowed)
    {
        printf("Players isnt going to change to AI since player is already dead\n");
        print_line();
        return;
    }

    printf("Player %s %d is an Ai.\n", players.at(id).name.c_str(), id);
    print_line();

    players.at(id).status_type = AI;
    players.at(id).name = "K.U.R.T";

    players_copy.at(id).status_type = AI;
    strcpy(players_copy.at(id).name, "K.U.R.T");
}

vector<player_id_minimal> Battleships::get_players_boards()
{
    return players_copy;
}

int Battleships::get_current_player()
{
    return current_player;
}

vector<player_id> Battleships::get_players_info()
{
    return this->players;
}

vector<player_id> Battleships::get_spectators()
{
    return this->spectators;
}

void Battleships::set_current_player(int cp_new)
{
    this->current_player = cp_new;
}

void Battleships::run_KURT()
{
    printf("Running K.U.R.T...\n");
    if (players.at(current_player).status_type != AI)
    {
        return;
    }

    random_device dev;
    mt19937 rng('K' + 'U' + 'R' + 'T' + dev());
    uniform_int_distribution<mt19937::result_type> rand_board_gen(0, players.size() - 1);
    uniform_int_distribution<mt19937::result_type> rand_pos_gen(0, 9);

    printf("K.U.R.T getting enemy board...\n");

    // random number between 0-3 such that it isnt current_player and that player isnt dead
    int rand_board = rand_board_gen(rng);

    while (rand_board == current_player || players.at(rand_board).status_type == Dead || players.at(rand_board).status_type == Not_Allowed)
    {
        rand_board = rand_board_gen(rng);
    }

    printf("K.U.R.T getting enemy position to hit...\n");

    // random number between 0 - 9 such that it hasnt been hit yet
    int rand_x = rand_pos_gen(rng);
    int rand_y = rand_pos_gen(rng);
    int count = 0;

    while (players_copy.at(rand_board).board[rand_x][rand_y] != '-')
    {
        int rand_x = rand_pos_gen(rng);
        int rand_y = rand_pos_gen(rng);
        count++;
        if (count == 1000)
        {
            printf("RANDOM BROKE??? -- idk why ---\n");
            break;
        }
    }

    printf("K.U.R.T filling Artilliary... \n");
    // add it to cords struct
    coordinates cord_rand;
    cord_rand.x = rand_x;
    cord_rand.y = rand_y;
    cord_rand.board = rand_board;

    // BLOW IT UP KURT!!
    bomb_space(cord_rand);
}

Battleships::~Battleships()
{
    printf("Game Ended\n");
    print_line();
}