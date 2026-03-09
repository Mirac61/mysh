#include "shell.h"
int schweif[] = {rain_color - 6, rain_color - 12, rain_color - 18, rain_color - 24, rain_color - 30};

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
void glitch_animation();   // 3 = Japanischer Tempel
void rain_animation();

void startup_sequence(int animation, int info) {
    // Prüfen ob interaktives Terminal
    // Wenn nicht (z.B. benchmark/script) → sofort beenden
    if (!isatty(STDIN_FILENO)) return;

    // Animation zuerst
    if (animation == 1) stars_animations();
    if (animation == 2) boot_animation();
    if (animation == 3) glitch_animation();
    if (animation == 4) rain_animation();

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

void boot_animation() {
    const char *lines[] = {
            "[ OK ] Loading kernel modules...",
            "[ OK ] Starting system logger...",
            "[ OK ] Mounting filesystems...",
            "[ OK ] Starting network manager...",
            "[ OK ] Loading user configuration...",
            "[ OK ] Initializing shell environment...",
            "[ OK ] Starting mysh v1.0...",
            NULL
        };

        printf("\033[2J\033[H");
        for (int i = 0; lines[i] != NULL; i++) {
            printf("\033[38;5;%dm[ OK ]\033[38;5;245m", boot_color);
            printf("%s\n", lines[i] + 5);
            fflush(stdout);
            usleep(100000 + rand() % 80000);
        }
        printf("\033[0m");
        usleep(300000);
        printf("\033[2J\033[3J\033[H");
}

void glitch_animation() {
    const char *logo[] = {
        "███╗   ███╗██╗   ██╗███████╗██╗  ██╗",
        "████╗ ████║╚██╗ ██╔╝██╔════╝██║  ██║",
        "██╔████╔██║ ╚████╔╝ ███████╗███████║",
        "██║╚██╔╝██║  ╚██╔╝  ╚════██║██╔══██║",
        "██║ ╚═╝ ██║   ██║   ███████║██║  ██║",
        "╚═╝     ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═╝",
        NULL
    };

    const char *glitch_chars = "!@#$%^&*<>?/\\|~`+=[]{}";
    int logo_zeilen = 6;
    int cols = 80;
    int logo_x = 20;
    int logo_y = 8;

    srand(time(NULL));

    // Phase 1: Logo erscheint normal
    printf("\033[2J\033[H");
    for (int i = 0; logo[i] != NULL; i++) {
        printf("\033[%d;%dH\033[38;5;255m%s\033[0m", logo_y + i, logo_x, logo[i]);
    }
    fflush(stdout);
    usleep(600000);

    // Phase 2: Glitch — zufällige Zeichen und Farben
    int glitch_farben[] = {196, 46, 21, 226, 201, 255, 51};
    for (int frame = 0; frame < 20; frame++) {
        // Original Logo
        for (int i = 0; logo[i] != NULL; i++) {
            printf("\033[%d;%dH\033[38;5;255m%s\033[0m", logo_y + i, logo_x, logo[i]);
        }

        // Glitch Zeilen — zufällige Zeichen über das Logo
        int anz_glitch = rand() % 4 + 1;
        for (int g = 0; g < anz_glitch; g++) {
            int zeile = rand() % logo_zeilen;
            int farbe = glitch_farben[rand() % 7];
            int len = strlen(logo[zeile]);

            printf("\033[%d;%dH\033[38;5;%dm", logo_y + zeile, logo_x, farbe);
            for (int c = 0; c < len; c++) {
                if (rand() % 3 == 0)
                    printf("%c", glitch_chars[rand() % strlen(glitch_chars)]);
                else
                    printf("%c", logo[zeile][c]);
            }
            printf("\033[0m");
        }

        // Horizontale Glitch-Linie
        if (rand() % 2 == 0) {
            int y = rand() % 24 + 1;
            int x = rand() % 40;
            int len = rand() % 20 + 5;
            int farbe = glitch_farben[rand() % 7];
            printf("\033[%d;%dH\033[38;5;%dm", y, x, farbe);
            for (int c = 0; c < len; c++)
                printf("%c", glitch_chars[rand() % strlen(glitch_chars)]);
            printf("\033[0m");
        }

        fflush(stdout);
        usleep(60000 + rand() % 80000);
    }

    // Phase 3: Logo stabilisiert sich — fade zu normaler Farbe
    int stabilize_farben[] = {196, 255, 196, 255, 255};
    for (int f = 0; f < 5; f++) {
        printf("\033[2J\033[H");
        for (int i = 0; logo[i] != NULL; i++) {
            printf("\033[%d;%dH\033[38;5;%dm%s\033[0m",
                logo_y + i, logo_x, stabilize_farben[f], logo[i]);
        }
        fflush(stdout);
        usleep(100000);
    }

    usleep(300000);
    printf("\033[2J\033[3J\033[H");
}

void rain_animation() {
    srand(time(NULL));
        int rows = 24, cols = 80;

        int pos[80], speed[80];
        for (int i = 0; i < cols; i++) {
            pos[i] = -(rand() % rows);
            speed[i] = rand() % 3 + 1;
        }

        for (int frame = 0; frame < 40; frame++) {
            printf("\033[2J\033[H");
            for (int col = 0; col < cols; col++) {
                for (int row = 0; row < rows; row++) {
                    int dist = pos[col] - row;
                    if (dist == 0) {
                        // Spitze — hell
                        printf("\033[%d;%dH\033[38;5;%dm|\033[0m", row + 1, col + 1, rain_color);
                    } else if (dist > 0 && dist < 6) {
                        // Schweif — automatisch dunkler
                        int schweif_farbe = rain_color - (dist * 6);
                        if (schweif_farbe < 0) schweif_farbe = 0;
                        printf("\033[%d;%dH\033[38;5;%dm|\033[0m",
                            row + 1, col + 1, schweif_farbe);
                    }
                }
                pos[col] += speed[col];
                if (pos[col] > rows + 6)
                    pos[col] = -(rand() % 10);
            }
            fflush(stdout);
            usleep(50000);
        }
        printf("\033[2J\033[3J\033[H");
}
