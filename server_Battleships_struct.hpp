#ifndef __SERVER_BATTLESHIPS_STRUCT_H__
#define __SERVER_BATTLESHIPS_STRUCT_H__

#include <vector>

#include "server_util_func.hpp"

enum status
{
    Alive,
    Dead,
    AI,
    Player,
    Spectator,
    Not_Allowed,
};

struct next_player_node
{
    int cli_id;
    struct next_player_node *next;
    struct next_player_node *back;
};

struct player_data
{
    char name[15]; // name of player
    status state;  // state confermation
    int cli_id;    // client id (for the server but gives a unqiue identifier)
};

struct bb // i need a unique name here but idk what you put ... bb it is
{
    int board[10][10];
    int cli_id;
};

struct ship_placement
{
    char ship;
    int x, y;
    bool hor;
};

struct cord_board
{
    int x, y, cli_id;
};

struct cord
{
    int x, y;
};

struct single_ship_checker
{
    std::vector<cord> cords;
    int count = 0; // count == 0 => ship sunk
    char ship;

    // sets up cords for ship checker using ship_placement
    void setup_ship_checker(ship_placement ship)
    {
        this->ship = ship.ship;                // char: A,B,C,D,S
        this->count = size_of_ship(ship.ship); // size: 5,4,3,2,1

        for (size_t i = 0; i < count; i++)
        {
            cord pos;
            int xplus = 0;
            int yplus = 0;

            if (ship.hor)
                xplus = i;
            else
                yplus = i;

            pos.x = ship.x + xplus;
            pos.y = ship.y + yplus;

            cords.push_back(pos);
        }
    }

    // true if hit & reduces count
    bool hit(cord pos)
    {
        for (size_t i = 0; i < cords.size(); i++)
        {
            if ((pos.x == cords.at(i).x) && (pos.y == cords.at(i).y))
            {
                this->count--;
                return true;
            }
        }
        return false;
    }

    // true if it exists in this package
    bool cord_exists(cord pos)
    {
        for (size_t i = 0; i < cords.size(); i++)
        {
            if ((pos.x == cords.at(i).x) && (pos.y == cords.at(i).y))
            {
                return true;
            }
        }
        return false;
    }
};

struct multiple_ship_checker
{
    std::vector<single_ship_checker> ship_checker_list;
    int cli_id;

    // adds ships using list of ship_placments
    void add_ships(std::vector<ship_placement> ship_list, int cli_id)
    {
        this->cli_id = cli_id;
        for (size_t i = 0; i < ship_list.size(); i++)
        {
            single_ship_checker ssc;
            ssc.setup_ship_checker(ship_list.at(i));
            ship_checker_list.push_back(ssc);
        }
    }

    // returns the cords or ship at pos(x,y)
    std::vector<cord> get_cord_list(cord pos, char ship)
    {
        for (size_t i = 0; i < ship_checker_list.size(); i++)
        {
            if (ship_checker_list.at(i).ship == ship)
            {
                if (ship_checker_list.at(i).cord_exists(pos))
                {
                    return ship_checker_list.at(i).cords;
                }
            }
        }
        std::vector<cord> random;
        return random;
    }

    std::vector<cord> get_cord_list_at(int i)
    {
        return ship_checker_list.at(i).cords;
    }

    // true if ship at pos(x,y) exists
    bool ship_exists(cord pos, char ship)
    {
        for (size_t i = 0; i < ship_checker_list.size(); i++)
        {
            if (ship_checker_list.at(i).ship == ship)
            {
                if (ship_checker_list.at(i).cord_exists(pos))
                {
                    return true;
                }
            }
        }
        return false;
    }

    // true if ship at pos(x,y) has sunk
    bool ship_sunk(cord pos, char ship)
    {
        for (size_t i = 0; i < ship_checker_list.size(); i++)
        {
            if (ship_checker_list.at(i).ship == ship)
            {
                if (ship_checker_list.at(i).cord_exists(pos))
                {
                    if (ship_checker_list.at(i).count <= 0)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    // true if all ships are count:0 else false
    bool all_ship_sunk()
    {
        for (size_t i = 0; i < ship_checker_list.size(); i++)
        {
            if (ship_checker_list.at(i).count > 0)
            {
                return false;
            }
        }
        return true;
    }

    // returns true if hit ship & reduces ship by 1
    bool hit_ship(cord pos, char ship)
    {
        for (size_t i = 0; i < ship_checker_list.size(); i++)
        {
            if (ship_checker_list.at(i).ship == ship)
            {
                if (ship_checker_list.at(i).hit(pos))
                {
                    return true;
                }
            }
        }
        return false;
    }
};

/* ship format [kɔ̃fiʀme]

    return:
    0 => success
    1 => fail - out of scope (x >= 10 || y >= 10)
    2 => fail - matching pair
*/
int ship_format_confirmer(std::vector<ship_placement> ship_list)
{
    std::vector<cord> cord_list;

    /* --- populate cord_list with all the cordinates of the ships --- */
    for (size_t i = 0; i < ship_list.size(); i++)
    {
        int count = size_of_ship(ship_list.at(i).ship); // size: 5,4,3,2,1

        for (size_t j = 0; j < count; j++)
        {
            cord pos;
            int xplus = 0;
            int yplus = 0;

            if (ship_list.at(i).hor)
                xplus = j;
            else
                yplus = j;

            pos.x = ship_list.at(i).x + xplus;
            pos.y = ship_list.at(i).y + yplus;

            cord_list.push_back(pos);
        }
    }

    /* --- check for over the limit numbers --- */
    for (size_t i = 0; i < cord_list.size(); i++)
    {
        if (cord_list.at(i).x >= 10 || cord_list.at(i).y >= 10)
        {
            return 1;
        }
    }

    /* --- check for matching pairs --- */ // (there is very possible faster way of doing this but i dont feel like doing that)
    for (size_t i = 0; i < cord_list.size(); i++)
    {
        for (size_t j = i + 1; j < cord_list.size(); j++)
        {
            if ((cord_list.at(i).x == cord_list.at(j).x) && (cord_list.at(i).y == cord_list.at(j).y))
            {
                return 2;
            }
        }
    }

    return 0;
}
#endif // __SERVER_BATTLESHIPS_STRUCT_H__