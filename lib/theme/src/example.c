#include "theme.h"

int main(char argc, char **argv){
    if(argc != 2){
        printf("example requires theme file!\n");
        printf("usage: %s default.theme.svg\n",argv[0]);
        return 1;
    }
    theme_t theme;
    load_theme_from_file(&theme, argv[1]);
    free_theme(&theme);
    return 0;
}