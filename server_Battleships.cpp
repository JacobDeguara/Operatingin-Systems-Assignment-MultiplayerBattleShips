#ifndef __SERVER_BATTLESHIPS_H__
#define __SERVER_BATTLESHIPS_H__

#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <random>

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

std::string get_player_state_as_string_battleships(status s);

class Battleships
{
private:
    std::vector<player_data> players;
    std::vector<player_data> spectators;
    std::vector<multiple_ship_checker> ship_checker;

    std::vector<bb> board_list;
    std::vector<bb> board_list_show;

    game_states state = start;
    struct next_player_node *head = NULL;

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
    void player_surrenders(int cli_id);
    bool bomb_space(cord_board bomb, int cli_id);
    bool set_player_AI(int cli_id);
    bool run_KURT();
    bool run_KURT_ships();

    /* --- Game_ended --- state: END --- */

    bool game_end();

    /* --- Default Values --- */

    int *print_num;
    Battleships(int *print_num);
    ~Battleships();

    /* --- next_player builder --- */

    void build_next_player_nodes();           // builds the nodes dictaticing the order
    bool remove_next_player_node(int cli_id); // removes a node from the rotation
    void move_back_player_node();             // moves back one step (this is to give the same player a second shot)
    int get_next_player_node();               // moves forward and returns the next players turn
    void free_player_nodes(next_player_node *head, next_player_node *temp);
    // recursivly removes each node from the player node

    /* --- Utilities --- */

    player_data find_player(int cli_id);        // returns the players player data based on their client_id
    bool player_exists(char name[15]);          // returns true only if the player with @name exists
    std::vector<player_data> get_players_all(); // returns the vector of player & spectators (players_vec + spectators_vec)
    int get_game_state();                       // returns 0-START, 1-SHIP, 2-GAME, 3-END
    void clear_board(bb *board, int cli_id);    // clears the board passed with '-' and adds @cli_id added
    std::vector<bb> get_show_boards();
    void sink_all_ships(int cli_id);
    bool are_all_players_AI();
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
                        print_line();
                        printf("%3d - Player %15s , %d Removed\n", (*print_num)++, id.name, id.cli_id);
                        print_line();
                        return id;
                    }
                }
            }
            else // ship state only
            {
                /* --- if player leaves he is changed to AI --- */
                set_player_AI(cli_id);
                return id;
            }
        }
    }

    if (id.state == Alive || id.state == AI)
    {
        /* --- if player leaves he is changed to AI --- */
        set_player_AI(cli_id);
        return id;
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
                    print_line();
                    printf("%3d - Player %15s , %d Removed\n", (*print_num)++, id.name, id.cli_id);
                    print_line();
                    return id;
                }
            }
        }
    }

    // this shouldn't happen
    print_line();
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
        printf("> %15s , %d %s\n", players.at(i).name, players.at(i).cli_id, get_player_state_as_string_battleships(players.at(i).state).c_str());
    }

    printf("Spectators: \n");
    for (size_t i = 0; i < spectators.size(); i++)
    {
        printf("- %15s , %d\n", spectators.at(i).name, spectators.at(i).cli_id);
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

                printf("%3d - Host Started the Game -> state changed to SHIP\n", (*print_num)++);
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
    /* --- check if ship format is correct (no overlaps, no out of bounce)--- */
    int res = ship_format_confirmer(ship_list);
    if (res == 1)
    {
        print_line();
        printf("%3d - ship_list of player %d is over the limit\n", (*print_num)++, cli_id);
        print_line();
        return false;
    }
    else if (res == 2)
    {
        print_line();
        printf("%3d - ship_list of player %d has overlapping ships\n", (*print_num)++, cli_id);
        print_line();
        return false;
    }

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
                break;
            }
        }

        multiple_ship_checker msc;
        msc.add_ships(ship_list, cli_id);
        ship_checker.push_back(msc);
        printf("%3d - Added new ships to ship_list for player %d\n", (*print_num)++, cli_id);
        print_line();
        return true;
    }
    return false;
}

bool Battleships::game_start()
{
    /* --- can only start if ship state AND ship_list.size() == player_list.size() (each player has put their ships) --- */
    if (state != ship)
        return false;

    if (!run_KURT_ships())
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
        /* --- players.state -> Players => Alive --- */
        if (players.at(i).state == Player)
        {
            players.at(i).state = Alive;
        }

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
    display_players();
    display_boards();

    /* --- sets up next player data struct ---*/
    build_next_player_nodes();
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
}

bool Battleships::set_player_AI(int cli_id)
{
    print_line();
    for (size_t i = 0; i < players.size(); i++)
    {
        if (players.at(i).cli_id == cli_id)
        {
            if (players.at(i).state == Alive || players.at(i).state == Player)
            {
                players.at(i).state = AI;
                printf("%3d - Player %d has been changed to KURT\n", (*print_num)++, cli_id);
                print_line();
                return true;
            }
            else if (players.at(i).state == Dead)
            {
                printf("%3d - Player %d is dead\n", (*print_num)++, cli_id);
                print_line();
                return false;
            }
        }
    }
    printf("%3d - Player %d hasn't been changed to KURT\n", (*print_num)++, cli_id);
    print_line();
    return false;
}

void Battleships::player_surrenders(int cli_id)
{
    print_line();
    for (size_t i = 0; i < players.size(); i++)
    {
        if (players.at(i).cli_id == cli_id)
        {
            players.at(i).state = Dead;
            remove_next_player_node(cli_id);
            printf("%3d - Player %d has surrendered and is now dead\n", (*print_num)++, cli_id);
            if (state != ship)
                sink_all_ships(cli_id);

            print_line();
            display_boards();
            return;
        }
    }
    printf("%3d - Player %d wasnt found\n", (*print_num)++, cli_id);
    print_line();
}

bool Battleships::bomb_space(cord_board bomb, int cli_id) // true -> re-do else false -> miss
{
    print_line();
    /* --- did correct player play bomb_space ---*/
    if (head->cli_id != cli_id)
    {
        printf("%3d - Player %d shouldn't be using bomb_space since its Player %d turn\n", (*print_num)++, cli_id, head->cli_id);
        print_line();
        return false;
    }

    /* --- did player play bomb_space on himself --- */
    if (bomb.cli_id == cli_id)
    {
        printf("%3d - Player %d bombed himself, action stoped...\n", (*print_num)++, cli_id);
        print_line();
        return true;
    }

    /* --- loop through each player to see which player was hit --- */
    for (size_t i = 0; i < players.size(); i++)
    {
        if (players.at(i).cli_id == bomb.cli_id)
        {
            /* --- is player dead ---*/
            if (players.at(i).state == Dead)
            {
                printf("%3d - Player %d is dead, you cant shot him that would break the geneva convention!!\n", (*print_num)++, bomb.cli_id);
                print_line();
                return true;
            }

            char char_board = board_list.at(i).board[bomb.x][bomb.y]; // '-' | 'O'

            if (char_board == '-')
            {
                char char_board_show = board_list_show.at(i).board[bomb.x][bomb.y]; // '-' | 'A' - 'S' ships
                if (char_board_show == 'O')
                {
                    /*--- you hit a poition thats already been hit --- */
                    printf("%3d - Player %d has hit a spot that has already been hit\n", (*print_num)++, cli_id);
                    print_line();
                    return true;
                }

                /*--- set pos on board_show to O - miss --- */
                board_list_show.at(i).board[bomb.x][bomb.y] = 'O';
                printf("%3d - Player %d hit was a miss\n", (*print_num)++, cli_id);
                print_line();
                return false;
            }
            else
            {
                char char_board_show = board_list_show.at(i).board[bomb.x][bomb.y]; // '-' | 'X' | 'F'

                if (char_board_show == 'X' || char_board_show == 'F')
                {
                    /*--- you hit a poition thats already been hit --- */
                    printf("%3d - Player %d hit a spot that has already been hit\n", (*print_num)++, cli_id);
                    print_line();
                    return true;
                }

                /*--- set pos on board_show to X - hit --- */
                board_list_show.at(i).board[bomb.x][bomb.y] = 'X';
                printf("%3d - Player %d has hit a ship -%c-\n", (*print_num)++, cli_id, char_board);

                /* --- use ship checker to check if ship has sunk--- */
                cord c;
                c.x = bomb.x;
                c.y = bomb.y;

                if (ship_checker.at(i).hit_ship(c, char_board))
                {
                    if (ship_checker.at(i).ship_sunk(c, char_board))
                    {
                        auto ship_list = ship_checker.at(i).get_cord_list(c, char_board);

                        for (size_t j = 0; j < ship_list.size(); j++)
                        {
                            board_list_show.at(i).board[ship_list.at(j).x][ship_list.at(j).y] = 'F';
                        }

                        printf("%3d - Player %d has sunk ship hit\n", (*print_num)++, cli_id);
                    }
                }

                /* --- if player is has all ships down remove them --- */
                if (ship_checker.at(i).all_ship_sunk())
                {
                    printf("%3d - Player %d has lost all his ship and is now raising the white flag of surrender\n", (*print_num)++, bomb.cli_id);
                    print_line();
                    player_surrenders(bomb.cli_id);
                    return true;
                }
                print_line();
                return true;
            }
        }
    }
    return true;
}

bool Battleships::run_KURT() // true -> re-do else false -> it ran possibly not but not because it broke
{
    printf("%3d --- K.U.R.T Running ---\n", (*print_num)++);
    /* --- is current player an Ai ---*/
    if (find_player(head->cli_id).state != AI)
    {
        printf("%3d - Player %d isnt an AI\n", (*print_num)++, head->cli_id);
        print_line();
        return false;
    }

    std::random_device dev;
    std::mt19937 rng('K' + 'U' + 'R' + 'T' + dev());
    std::uniform_int_distribution<std::mt19937::result_type> rand_board_gen(0, players.size() - 1);
    std::uniform_int_distribution<std::mt19937::result_type> rand_pos_gen(0, 9);

    // random number between 0-3 such that it isnt current_player and that player isnt dead
    int rand_board = rand_board_gen(rng);

    /* --- random board number cant be of dead player and cant be itself --- */
    while (players.at(rand_board).state == Dead || players.at(rand_board).cli_id == head->cli_id)
    {
        rand_board = rand_board_gen(rng);
    }

    // random number between 0 - 9 such that it hasnt been hit yet
    int rand_x = rand_pos_gen(rng);
    int rand_y = rand_pos_gen(rng);
    int count = 0;

    /* --- player at rand_board must be a position that isnt hit yet ---*/
    while (board_list_show.at(rand_board).board[rand_x][rand_y] != '-')
    {
        int rand_x = rand_pos_gen(rng);
        int rand_y = rand_pos_gen(rng);
        count++;
        if (count == 15)
        {
            printf("%3d - KURT went over 15 random limit, resetting KURT\n", (*print_num)++);
            std::random_device dev;
            std::mt19937 rng('K' + 'U' + 'R' + 'T' + dev());
            // instead of returning true we can just run kurt until it hits (but i dont trust it, it gave me unexpected results)
            count = 0;
            break;
        }
    }

    // add it to cords struct
    cord_board cord_rand;
    cord_rand.x = rand_x;
    cord_rand.y = rand_y;
    cord_rand.cli_id = players.at(rand_board).cli_id;

    printf("%3d - K.U.R.T hitting player %d \n", (*print_num)++, players.at(rand_board).cli_id);

    // BLOW IT UP KURT!!
    return bomb_space(cord_rand, head->cli_id); // since true means shot again then return true which => shot again then shot again
}

bool Battleships::run_KURT_ships() // true if it succeeds , else false if it failed
{
    /* --- get amount of players that arent AI's --- */
    int amount_players = players.size();
    for (size_t i = 0; i < players.size(); i++)
    {
        if (players.at(i).state == AI)
        {
            amount_players--;
        }
    }

    /* --- if player amount (exculuding AI's) <= the amount of ships put => continue... --- */
    if (amount_players > ship_checker.size())
    {
        return false;
    }

    /* --- setup AI's with ships cause they might not have any ---*/
    for (size_t i = 0; i < players.size(); i++)
    {
        if (players.at(i).state == AI)
        {
            bool has_no_ship_flag = true;

            /* --- turn on flag if AI has ship ---*/
            for (size_t j = 0; j < ship_checker.size(); j++)
            {
                if (ship_checker.at(j).cli_id == players.at(i).cli_id)
                {
                    has_no_ship_flag = false;
                    break;
                }
            }

            /* --- if AI doesnt have ship --- */
            if (has_no_ship_flag)
            {
                std::vector<ship_placement> ship_list;

                ship_placement sp;
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

                add_ships(ship_list, players.at(i).cli_id);
            }
        }
    }
    return true;
}

bool Battleships::game_end()
{
    int count = players.size();
    for (size_t i = 0; i < players.size(); i++)
    {
        if (players.at(i).state == Dead)
        {
            count--;
        }
    }

    if (count == 1)
    {
        return true;
    }
    return are_all_players_AI();
}

Battleships::~Battleships()
{
    print_line();
    printf("%3d - Game Ended\n", (*print_num)++);
    print_line();

    if (head != NULL)
        free_player_nodes(this->head, this->head->next);
}

void Battleships::build_next_player_nodes()
{
    struct next_player_node *temp_first = (next_player_node *)malloc(sizeof(next_player_node));
    temp_first->cli_id = players.at(0).cli_id;

    next_player_node *prevoius = temp_first;
    struct next_player_node *temp_next;

    for (size_t i = 1; i < players.size(); i++)
    {
        temp_next = (next_player_node *)malloc(sizeof(next_player_node));

        temp_next->cli_id = players.at(i).cli_id; // set cli_id
        temp_next->back = prevoius;               // previous to prevoius ptr
        prevoius->next = temp_next;               // set prevoius ptr next to
        prevoius = temp_next;                     // set temp_next to prevoius ptr
    }

    temp_next->next = temp_first;
    temp_first->back = temp_next;

    this->head = temp_next;
}

bool Battleships::remove_next_player_node(int cli_id)
{
    if (head == NULL)
        return false;

    auto temp = head;
    while (temp->cli_id != cli_id) // loop until you find the right node
    {
        temp = temp->next;
        if (temp->cli_id == head->cli_id) // if temp == head meaning we looped back
        {
            return false; // failed
        }
    }

    if (temp == head)
    {
        this->head = this->head->next;
    }

    auto temp_back = temp->back;
    auto temp_next = temp->next;

    temp_back->next = temp->next;
    temp_next->back = temp->back;

    free(temp);
    return true;
}

void Battleships::move_back_player_node()
{
    if (head == NULL)
        return;

    this->head = this->head->back;
}

int Battleships::get_next_player_node()
{
    if (head == NULL)
        return -1;

    this->head = this->head->next; // move head to next_head

    return head->cli_id; // return cli_id
}

void Battleships::free_player_nodes(next_player_node *head, next_player_node *temp) // head:.1->2->3->4->.1 , free(1) , free(4) , free(3), free(2)
{

    if (head == temp) // base case: reached the start again
    {
        free(head);
        return;
    }
    free_player_nodes(head, temp->next); // recurse

    free(temp);
    return;
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

void Battleships::sink_all_ships(int cli_id)
{
    for (size_t i = 0; i < players.size(); i++)
    {
        if (players.at(i).cli_id == cli_id) // find player
        {
            for (size_t j = 0; j < ship_checker.at(i).ship_checker_list.size(); j++) // loop through each ship
            {
                auto ship = ship_checker.at(i).get_cord_list_at(j);

                for (size_t k = 0; k < ship.size(); k++) // loop through pos of ship
                {
                    board_list_show.at(i).board[ship.at(k).x][ship.at(k).y] = 'F'; // make is sunk
                }
            }
            break;
        }
    }
}

bool Battleships::are_all_players_AI()
{
    bool flag = true;
    for (size_t i = 0; i < players.size(); i++)
    {
        if (players.at(i).state != AI)
        {
            flag = false;
            break;
        }
    }
    return flag;
}

std::string get_player_state_as_string_battleships(status s)
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

#endif // __SERVER_BATTLESHIPS_H__