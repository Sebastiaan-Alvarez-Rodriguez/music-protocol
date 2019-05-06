#include <stdbool.h>
#include <stdio.h>

#include "menu.h"

bool menu_yes_no(const char* question) {
    char ans[2];
    while (true) {
        printf("%s [Y/n]: ", question);
        fgets(ans,sizeof(ans),stdin);
        puts("");
        if (ans[0] == 'y' || ans[0] == 'Y')
            return true;
        else if (ans[0] == 'n' || ans[0] == 'N')
            return false;
        else
            puts("Please specify 'y' or 'n'");
    }
}