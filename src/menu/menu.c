#include <stdbool.h>
#include <stdio.h>

#include "menu.h"

bool menu_yes_no(const char* question) {
    char ans[3];
    while (true) {
        printf("%s [Y/n]: ", question);
        fgets(ans,sizeof(ans),stdin);
        puts("");
        for (unsigned i = 0; i < sizeof(ans); ++i) {
            if (ans[i] == 'y' || ans[i] == 'Y')
                return true;
            else if (ans[i] == 'n' || ans[i] == 'N')
                return false;
        }
        puts("Please specify 'y' or 'n'");
    }
}