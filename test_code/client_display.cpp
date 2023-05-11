#ifndef __CLIENT_DISPLAY_H__
#define __CLIENT_DISPLAY_H__

#include <ncurses.h>
#include <vector>
#include <string>
#include <array>
#include <cmath>

#include "server_structs.hpp"

class client_display
{
private:
    WINDOW *boards;
    WINDOW *board_display_1;
    WINDOW *board_display_2;
    WINDOW *board_display_3;
    WINDOW *board_display_4;

    WINDOW *settings_menu;
    WINDOW *players_display;

    void windowCreation(int yMax, int xMax);

public:
    client_display(/* args */);
    void update_test_display();

    /* --- temporary Windows --- */

    std::string temporary_name_window();
    void temporary_error_window(std::string err);
    void temporary_player_window(player_data pd);
    void temporary_window(std::string msg);

    void update_player_display(settings_request_playerlist srp);

    ~client_display();
};

client_display::client_display(/* args */)
{
    initscr();
    noecho();
    cbreak();
    curs_set(0);

    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    windowCreation(yMax, xMax);
}

void client_display::windowCreation(int yMax, int xMax)
{

    boards = newwin(yMax, xMax * 2 / 3, 0, 0);
    box(boards, 0, 0); // create box
    refresh();
    wrefresh(boards);
    keypad(boards, true);

    board_display_1 = newwin((yMax * 1 / 2) - 1, (xMax * 1 / 3) - 1, 1, 1);
    board_display_2 = newwin((yMax * 1 / 2) - 1, (xMax * 1 / 3) - 1, (yMax * 1 / 2) + 1, 1);
    board_display_3 = newwin((yMax * 1 / 2) - 1, (xMax * 1 / 3) - 1, (yMax * 1 / 2) + 1, (xMax * 1 / 3) + 1);
    board_display_4 = newwin((yMax * 1 / 2) - 1, (xMax * 1 / 3) - 1, 1, (xMax * 1 / 3) + 1);

    /*  --- this was for testing to see the boarder of the boxes created properly --- */
    box(board_display_1, 0, 0); // create box
    refresh();
    wrefresh(board_display_1);
    keypad(board_display_1, true);

    box(board_display_2, 0, 0); // create box
    refresh();
    wrefresh(board_display_2);
    keypad(board_display_2, true);

    box(board_display_3, 0, 0); // create box
    refresh();
    wrefresh(board_display_3);
    keypad(board_display_3, true);

    box(board_display_4, 0, 0); // create box
    refresh();
    wrefresh(board_display_4);
    keypad(board_display_4, true);

    players_display = newwin(yMax * 2 / 3, xMax * 1 / 3, 0, xMax * 2 / 3);
    box(players_display, 0, 0); // create box
    refresh();
    wrefresh(players_display);
    keypad(players_display, true);

    settings_menu = newwin(yMax * 1 / 3, xMax * 1 / 3, yMax * 2 / 3, xMax * 2 / 3);
    box(settings_menu, 0, 0); // create box
    refresh();
    wrefresh(settings_menu);
    keypad(settings_menu, true);
}

void client_display::update_test_display()
{
    // reset boxes
    noecho();

    box(players_display, 0, 0);
    box(settings_menu, 0, 0);
    box(boards, 0, 0);

    mvwprintw(boards, 0, 1, "Board");
    box(board_display_1, 0, 0);

    /* --- how to add player boards ---  (boards + board_display_1)*/
    mvwprintw(board_display_1, 1, 2, "Player_name__:");
    mvwprintw(board_display_1, 2, 2, "  A B C D E F G H I J");

    for (size_t i = 0; i < 10; i++)
    {
        mvwprintw(board_display_1, 3 + i, 1, "%2d ", (i + 1));

        for (size_t j = 0; j < 10; j++)
        {
            wprintw(board_display_1, "- ");
        }
    }

    mvwprintw(board_display_1, 1, 24, "Ships:");

    refresh();
    wrefresh(boards);
    wrefresh(board_display_1);

    /* --- how to write the display players --- */
    mvwprintw(players_display, 0, 1, "Connected-players");

    mvwprintw(players_display, 1, 1, "Players:");
    mvwprintw(players_display, 2, 1, "Spectators:");
    wrefresh(players_display);

    /* --- how to write the settings --- */
    mvwprintw(settings_menu, 0, 1, "Menu");
    mvwprintw(settings_menu, 1, 1, "> exit");
    mvwprintw(settings_menu, 1, 2, "> update");
    mvwprintw(settings_menu, 1, 3, "> switch to player");

    wrefresh(settings_menu);
}

std::string client_display::temporary_name_window()
{
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    WINDOW *temp_win = newwin(yMax * 1 / 2, xMax * 1 / 2, yMax * 1 / 4, xMax * 1 / 4);
    box(temp_win, 0, 0);
    echo();
    mvwprintw(temp_win, 1, 1, "Write your Name:");

    char name[15];
    wmove(temp_win, 2, 1);
    wrefresh(temp_win);
    wgetstr(temp_win, name);
    delwin(temp_win);
    return name;
}

void client_display::temporary_error_window(std::string err)
{
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    WINDOW *temp_win = newwin(yMax * 1 / 2, xMax * 1 / 2, yMax * 1 / 4, xMax * 1 / 4);
    box(temp_win, 0, 0);

    mvwprintw(temp_win, 1, 1, "Error Occured: %s", err.c_str());

    wrefresh(temp_win);
    getch();

    delwin(temp_win);
}

void client_display::temporary_player_window(player_data pd)
{
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    WINDOW *temp_win = newwin(yMax * 1 / 2, xMax * 1 / 2, yMax * 1 / 4, xMax * 1 / 4);
    box(temp_win, 0, 0);

    char play[15];
    if (pd.state == Spectator)
    {
        strcpy(play, "Spectator");
    }
    else
    {
        strcpy(play, "Player");
    }

    mvwprintw(temp_win, 1, 1, "You are %s %s", play, pd.name);
    wrefresh(temp_win);
    getch();
}

void client_display::temporary_window(std::string msg)
{
}

void client_display::update_player_display(settings_request_playerlist srp)
{
    mvwprintw(players_display, 0, 1, "Connected-players");

    mvwprintw(players_display, 1, 1, "Players:");

    for (size_t i = 0; i < srp.size; i++)
    {
        mvwprintw(players_display, 1 + i, 1, "Player: %s, id %d, status %d\n", srp.player_list[i].name, srp.player_list[i].cli_id, srp.player_list[i].state);
    }

    mvwprintw(players_display, 2, 1, "Spectators:");

    wrefresh(players_display);
    refresh();
}

client_display::~client_display()
{
    endwin();
}

#endif // __CLIENT_DISPLAY_H__