#include "Battleship_Stucts.hpp"
#include "Battleships.cpp"

int main()
{
    Battleships game = Battleships();
    game.add_player("HeatBattery");
    game.add_player("RyzeRaven");
    game.add_player("OwenOwenO");
    vector<shipp> ship_list;
    shipp ship_item;

    ship_item.x = 0;
    ship_item.y = 0;
    ship_item.hor = true;
    ship_item.ship = 'A';
    ship_list.push_back(ship_item);

    ship_item.x = 1;
    ship_item.y = 0;
    ship_item.hor = true;
    ship_item.ship = 'B';
    ship_list.push_back(ship_item);

    ship_item.x = 2;
    ship_item.y = 0;
    ship_item.hor = true;
    ship_item.ship = 'C';
    ship_list.push_back(ship_item);

    ship_item.x = 3;
    ship_item.y = 0;
    ship_item.hor = true;
    ship_item.ship = 'D';
    ship_list.push_back(ship_item);

    ship_item.x = 4;
    ship_item.y = 0;
    ship_item.hor = true;
    ship_item.ship = 'S';
    ship_list.push_back(ship_item);

    ship_item.x = 5;
    ship_item.y = 0;
    ship_item.hor = true;
    ship_item.ship = 'D';
    ship_list.push_back(ship_item);

    ship_item.x = 6;
    ship_item.y = 0;
    ship_item.hor = true;
    ship_item.ship = 'S';
    ship_list.push_back(ship_item);

    game.add_ships(ship_list, "HeatBattery");
    game.add_ships(ship_list, "RyzeRaven");
    game.add_ships(ship_list, "OwenOwenO");
    game.startGame();

    game.next_turn();
    coordinates coordinate;
    coordinate.x = 0;
    coordinate.y = 0;
    coordinate.board = 0;
    game.bomb_space(coordinate);

    game.next_turn();
    coordinate.x = 5;
    coordinate.y = 0;
    coordinate.board = 1;
    game.bomb_space(coordinate);

    game.next_turn();
    coordinate.x = 5;
    coordinate.y = 1;
    coordinate.board = 1;
    game.bomb_space(coordinate);
    game.next_turn();

    game.set_player_dead(1);
    game.next_turn();

    // test kurt!
    game.set_player_KURT(2);
    for (size_t i = 0; i < 25; i++)
    {
        game.run_KURT();
        game.set_player_KURT(2);
        game.print_boards();
    }
}