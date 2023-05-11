#ifndef __BATTLESHIP_STUCTS_H__
#define __BATTLESHIP_STUCTS_H__

#include <string>
#include <vector>
#include <cstring>

enum status
{
    Alive,
    Dead,
    AI,
    Spectator,
    Not_Allowed,
};

typedef struct ship_placement
{
    char ship;
    int x, y;
    bool hor;
} shipp;

struct Board
{
    int b[10][10];
};

struct player_id
{
    std::string name;
    int id;
    bool confirmation;
    status status_type;
    int board[10][10];
    std::vector<shipp> list;

    void clear_board()
    {
        for (size_t i = 0; i < 10; i++)
        {
            for (size_t j = 0; j < 10; j++)
            {
                board[i][j] = '-';
            }
        }
    };
};

struct player_id_minimal
{
    char name[15];
    int id;
    status status_type;
    int board[10][10];

    void copy_player_id(player_id player)
    {
        strcpy(name, player.name.c_str());
        id = player.id;
        status_type = player.status_type;

        for (size_t i = 0; i < 10; i++)
        {
            for (size_t j = 0; j < 10; j++)
            {
                board[i][j] = '-';
            }
        }
    }
};

struct players_info
{
    int amount_of_players, amount_of_spectators;
    player_id_minimal players[4];
    player_id_minimal spectators[11];
    bool flag;
};

struct player_info
{
    char name[15];
    int id;
    status status_type;
    bool conf;
    bool host;
};

struct ship_counter
{
    std::vector<int> x;
    std::vector<int> y;
    int count = 0;
    char ship;

    void add_pos(int x, int y)
    {
        this->x.push_back(x);
        this->y.push_back(y);
    };

    bool exists_pos(int x, int y)
    {
        for (size_t i = 0; i < this->x.size(); i++)
        {
            if (this->x.at(i) == x && this->y.at(i) == y)
            {
                return true;
            }
        }
        return false;
    };

    void add_count(int amount)
    {
        count = amount;
    };

    bool sunk_position()
    {
        count--;
        return true;
    };

    void clear()
    {
        x.clear();
        y.clear();
    };
};

int size_of_ship(char c)
{
    if (c == 'A')
        return 5;
    else if (c == 'B')
        return 4;
    else if (c == 'C')
        return 3;
    else if (c == 'D')
        return 2;
    else if (c == 'S')
        return 1;
    return 0;
}

typedef struct ship_counter_multiple
{
    std::vector<ship_counter> ship_lists;

    std::vector<int> get_x_list(int x, int y, char ship)
    {
        for (size_t i = 0; i < ship_lists.size(); i++)
        {
            if (ship_lists.at(i).ship == ship)
            {
                if (ship_lists.at(i).exists_pos(x, y))
                {
                    return ship_lists.at(i).x;
                }
            }
        }
        std::vector<int> random;
        return random;
    };

    std::vector<int> get_y_list(int x, int y, char ship)
    {
        for (size_t i = 0; i < ship_lists.size(); i++)
        {
            if (ship_lists.at(i).ship == ship)
            {
                if (ship_lists.at(i).exists_pos(x, y))
                {
                    return ship_lists.at(i).y;
                }
            }
        }
        std::vector<int> random;
        return random;
    };

    void add_ships(std::vector<shipp> ship_list)
    {
        for (size_t i = 0; i < ship_list.size(); i++)
        {
            ship_counter sc;
            sc.add_count(size_of_ship(ship_list.at(i).ship));
            sc.ship = ship_list.at(i).ship;

            for (size_t j = 0; j < sc.count; j++)
            {
                int xplus = 0;
                int yplus = 0;

                if (ship_list.at(i).hor)
                    yplus = j;
                else
                    xplus = j;

                sc.add_pos(xplus + ship_list.at(i).x, yplus + ship_list.at(i).y);
            }
            ship_lists.push_back(sc);
            sc.clear();
        }
    };

    bool ship_exists(char ship)
    {
        for (size_t i = 0; i < ship_lists.size(); i++)
        {
            if (ship_lists.at(i).ship == ship)
            {
                if (ship_lists.at(i).count == 0)
                {
                    return true;
                }
            }
        }
        return false;
    };

    bool ship_sunk(int x, int y, char ship)
    {
        for (size_t i = 0; i < ship_lists.size(); i++)
        {
            if (ship_lists.at(i).ship == ship)
            {
                if (ship_lists.at(i).exists_pos(x, y))
                {
                    if (ship_lists.at(i).count == 0)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    };

    bool all_ships_sunk()
    {
        for (size_t i = 0; i < ship_lists.size(); i++)
        {
            if (ship_lists.at(i).count != 0)
            {
                return false;
            }
        }
        return true;
    };

    bool bomb_ship_hit(int x, int y, char ship)
    {
        for (size_t i = 0; i < ship_lists.size(); i++)
        {
            if (ship_lists.at(i).ship == ship)
            {
                if (ship_lists.at(i).exists_pos(x, y))
                {
                    return ship_lists.at(i).sunk_position();
                }
            }
        }
        return false; //? idk how you got here ?
    };

} ship_counter_m;

typedef struct cord
{
    int x, y;
    int board;
} coordinates;

#endif // __BATTLESHIP_STUCTS_H__