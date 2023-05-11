#include <vector>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

struct test
{
    char name_c[15];
};

struct holder
{
    int *ptr;
};

int main()
{
    std::vector<struct test *> list;

    struct test *item_new = (test *)malloc(sizeof(test));
    strcpy(item_new->name_c, "Hello");

    list.push_back(item_new);

    struct test *ptr_new;

    ptr_new = item_new;
    strcpy(ptr_new->name_c, "Jacob");
    printf("vector > %s\n", list.at(0)->name_c);

    list.push_back(item_new);
    list.push_back(item_new);
    list.push_back(item_new);
    printf("vector > %s\n", list.at(0)->name_c);

    int *ptr_ptr_int;
    int *ptr_ptr_ptr_int;
    for (size_t i = 0; i < 3; i++)
    {
        int *ptr_int = (int *)malloc(sizeof(int));
        if (i == 0)
        {
            *ptr_int = 100;
            ptr_ptr_int = ptr_int;
            printf("holder - %d ,new-item %d\n", *ptr_ptr_int, *ptr_int);
        }
        else if (i == 1)
        {
            *ptr_int = 200;
            ptr_ptr_ptr_int = ptr_int;
            printf("holder - %d ,holder2 - %d ,new-item %d\n", *ptr_ptr_int, *ptr_ptr_ptr_int, *ptr_int);
        }
        else
        {
            *ptr_int = 300;
            printf("holder - %d ,holder2 - %d ,new-item %d\n", *ptr_ptr_int, *ptr_ptr_ptr_int, *ptr_int);
        }
    }
    /* Observed results:
     * 1. The item in push_back is passed as a copy,
     * 2. The item can be changed throught a ptr by refrenceing the new space in vector,
     * 3. When the item is erased it doesnt chanage position but it give no output at all 'empty'.
     * */
}