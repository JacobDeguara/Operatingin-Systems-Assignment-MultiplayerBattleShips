#ifndef __SERVER_UTIL_FUNC_H__
#define __SERVER_UTIL_FUNC_H__

#include <stdio.h>

void print_line()
{
    printf("===============================================================================================\n");
}

int size_of_ship(char pos)
{
    if (pos == 'A')
        return 5;
    else if (pos == 'B')
        return 4;
    else if (pos == 'C')
        return 3;
    else if (pos == 'D')
        return 2;
    else if (pos == 'S')
        return 1;
    return 0;
};

int char_pos(char pos)
{
    return pos - 65; // 'A' - 'A' = 0 , 'A' - 'B' = 1 , ect...
}

int check_format(std::string str) // [0-9][A-J](h/v)
{
    if (str.size() != 3)
    {
        return 1;
    }

    if (!((str.at(0) >= '0') && (str.at(0) <= '9')))
    {
        printf("not 0-9 ");
        return 2;
    }

    if (!((str.at(1) >= 'A') && (str.at(1) <= 'J')))
    {
        printf("not A-J");
        return 3;
    }

    if ((str.at(2) != 'h') && (str.at(2) != 'v'))
    {
        printf("not h|v ");
        return 4;
    }

    return 0;
}

int check_format_hit(std::string str)
{
    if (str.size() < 2)
    {
        return 1;
    }

    if (!((str.at(0) >= '0') && (str.at(0) <= '9')))
    {
        printf("not 0-9 ");
        return 2;
    }

    if (!((str.at(1) >= 'A') && (str.at(1) <= 'J')))
    {
        printf("not A-J");
        return 3;
    }

    for (size_t i = 2; i < str.size(); i++)
    {
        if (!((str.at(i) >= '0') && (str.at(i) <= '9')))
        {
            printf("not 0-9 ");
            return 4;
        }
    }
    return 0;
}
#endif // __SERVER_UTIL_FUNC_H__