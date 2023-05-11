#include "server_Battleships.cpp"

int main()
{
    int print_num = 0;
    Battleships game = Battleships(&print_num);

    char name[15];

    strcpy(name, "Dante_Viego");
    auto player4 = game.add_player(name, 3);

    strcpy(name, "RyzeRaven");
    auto player2 = game.add_player(name, 1);

    strcpy(name, "HeatBattery");
    auto player1 = game.add_player(name, 0);

    strcpy(name, "OwenOweno");
    auto player3 = game.add_player(name, 2);

    game.display_players();

    game.game_start_ship(1);
    game.game_start_ship(3);

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

    game.add_ships(ship_list, 0);
    game.add_ships(ship_list, 1);
    game.add_ships(ship_list, 0);
    game.add_ships(ship_list, 2);

    game.game_start();

    game.add_ships(ship_list, 3);

    game.game_start();

    /*
    game.build_next_player_nodes();

    printf("%d - player\n", game.get_next_player_node());
    printf("%d - player\n", game.get_next_player_node());
    printf("%d - player\n", game.get_next_player_node());
    printf("%d - player\n", game.get_next_player_node());
    game.move_back_player_node();
    printf("%d - player\n", game.get_next_player_node());
    game.remove_next_player_node(3);
    game.move_back_player_node();
    printf("%d - player\n", game.get_next_player_node());
    game.move_back_player_node();
    printf("%d - player\n", game.get_next_player_node());

    game.player_surrenders(1);

    printf("%d - player\n", game.get_next_player_node());
    printf("%d - player\n", game.get_next_player_node());
    printf("%d - player\n", game.get_next_player_node());

    printf("%d - player\n", game.get_next_player_node());
    printf("%d - player\n", game.get_next_player_node());
    */

    printf("%d - player\n", game.get_next_player_node()); // player 0 turn

    cord_board bomb;
    bomb.cli_id = 3;
    bomb.x = 6;
    bomb.y = 6;

    game.bomb_space(bomb, 0);

    for (size_t i = 0; i < 5; i++)
    {
        for (size_t j = 0; j < 7; j++)
        {
            bomb.cli_id = 3;
            bomb.x = i;
            bomb.y = j;
            game.bomb_space(bomb, 0);
            game.display_boards();
        }
    }
}