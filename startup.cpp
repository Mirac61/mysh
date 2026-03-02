#include "shell.h"

const char *logo[] = {
    "███╗   ███╗██╗   ██╗███████╗██╗  ██╗",
    "████╗ ████║╚██╗ ██╔╝██╔════╝██║  ██║",
    "██╔████╔██║ ╚████╔╝ ███████╗███████║",
    "██║╚██╔╝██║  ╚██╔╝  ╚════██║██╔══██║",
    "██║ ╚═╝ ██║   ██║   ███████║██║  ██║",
    "╚═╝     ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═╝",
    NULL
};

void logo_instant() {
    for (int i = 0; logo[i] != NULL; i++) {
        printf("%s\n", logo[i]);
    }
}

void logo_typewriter() {
    for (int i = 0; logo[i] != NULL; i++) {
        for (int j = 0; logo[i][j] != '\0'; j++) {
            printf("%c", logo[i][j]);
            fflush(stdout);
            usleep(2250);
        }
        printf("\n");
    }
}


void logo_fadein() {
    // 236 = sehr dunkelgrau, 255 = weiß
    // pro Zeile wird die Farbe heller
    int farben[] = {236, 238, 240, 242, 244, 255};

    for (int i = 0; logo[i] != NULL; i++) {
        printf("\033[38;5;%dm%s\033[0m\n", farben[i], logo[i]);
        usleep(80000); // kurze Pause zwischen Zeilen
    }
}


void startup_animation(int method) {
    if (method == 0) return;           // kein Logo
    if (method == 1) logo_instant();
    if (method == 2) logo_typewriter();
    if (method == 3) logo_fadein();
}
