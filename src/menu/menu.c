#include <stdbool.h>
#include <stdio.h>

#include "menu.h"

bool menu_yes_no(const char* question) {
    char ans;
    while (true) {
        printf("%s [Y/n]: ", question);
        fgets(&ans,1,stdin);
        puts("");
        if (ans == 'y' || ans == 'Y')
            return true;
        else if (ans = 'n' || ans == 'N')
            return false;
        else
            puts("Please specify 'y' or 'n'");
    }
}