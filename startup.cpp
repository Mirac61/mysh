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
    for (int i = 0; logo[i] != NULL; i++)
        printf("%s\n", logo[i]);
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
    int farben[] = {236, 238, 240, 242, 244, 255};
    for (int i = 0; logo[i] != NULL; i++) {
        printf("\033[38;5;%dm%s\033[0m\n", farben[i], logo[i]);
        usleep(80000);
    }
}

// Animationen (startup_animation=)
void stars_animations();    // 1 = Sternenhimmel
void boot_animation();     // 2 = Boot-Sequenz
void temple_animation();   // 3 = Japanischer Tempel

void startup_sequence(int animation, int info) {
    // Prüfen ob interaktives Terminal
    // Wenn nicht (z.B. benchmark/script) → sofort beenden
    if (!isatty(STDIN_FILENO)) return;
    
    // Animation zuerst
    if (animation == 1) stars_animations();
    if (animation == 2) boot_animation();
    if (animation == 3) temple_animation();
    
    // Dann Info anzeigen
    if (info == 0) return;
    if (info == 1) logo_instant();
    if (info == 2) logo_fadein();
    if (info == 3) logo_typewriter();
}

void stars_animations() {
    srand(time(NULL));
    int rows = 24, cols = 80;

    // Sterne-Positionen generieren
    int anzahl = 80;
    int sx[80], sy[80];
    for (int i = 0; i < anzahl; i++) {
        sx[i] = rand() % cols;
        sy[i] = rand() % rows;
    }

    // 1,5 Sekunden animieren
    for (int frame = 0; frame < 15; frame++) {
        printf("\033[2J\033[H"); // clear screen

        for (int i = 0; i < anzahl; i++) {
            printf("\033[%d;%dH", sy[i] + 1, sx[i] + 1);
            // Sterne blinken
            if (rand() % 3 == 0)
                printf("\033[38;5;255m*\033[0m");
            else if (rand() % 2 == 0)
                printf("\033[38;5;245m.\033[0m");
            else
                printf("\033[38;5;240m·\033[0m");
        }

        // Shooting Star gelegentlich
        if (frame % 8 == 0) {
            int y = rand() % (rows / 2);
            int x = rand() % (cols - 10);
            printf("\033[%d;%dH\033[38;5;226m✦\033[0m", y + 1, x + 1);
        }

        fflush(stdout);
        usleep(100000); // 100ms pro Frame
    }
    printf("\033[2J\033[3J\033[H"); // clear vor Logo
}

void boot_animation() {}
void temple_animation() {}
