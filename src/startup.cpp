#include "shell.h"
#include <string.h>

const char *logo[] = {"███╗   ███╗██╗   ██╗███████╗██╗  ██╗",
                      "████╗ ████║╚██╗ ██╔╝██╔════╝██║  ██║",
                      "██╔████╔██║ ╚████╔╝ ███████╗███████║",
                      "██║╚██╔╝██║  ╚██╔╝  ╚════██║██╔══██║",
                      "██║ ╚═╝ ██║   ██║   ███████║██║  ██║",
                      "╚═╝     ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═╝",
                      NULL};

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

void stars_animations(int clear_after);
void boot_animation(int clear_after);
void glitch_animation(int clear_after);
void rain_animation(int clear_after);

void startup_sequence(int animation, int info) {
  if (!isatty(STDIN_FILENO))
    return;

  int clear_after = (info != 4);

  if (animation == 1)
    stars_animations(clear_after);
  if (animation == 2)
    boot_animation(clear_after);
  if (animation == 3)
    glitch_animation(clear_after);
  if (animation == 4)
    rain_animation(clear_after);

  if (info == 0)
    return;
  if (info == 1)
    logo_instant();
  if (info == 2)
    logo_fadein();
  if (info == 3)
    logo_typewriter();
  if (info == 4) {
    printf("\033[2J\033[3J\033[H");
    fflush(stdout);
    char cmd[1024];
    char *home = getenv("HOME");
    if (home) {
      snprintf(cmd, sizeof(cmd),
               "neofetch --source %s/.mysh_logo.txt --ascii_colors %d %d "
               "--colors %d 7 %d %d %d %d --color_blocks off",
               home, accent_color_term, accent_color_term,
               accent_color_term, // c1 = Labels lila
               // 7 = weiß für Werte
               accent_color_term, accent_color_term, accent_color_term,
               accent_color_term);
      system(cmd);
    }
    printf("\n");
    fflush(stdout);
    return;
  }
}

void stars_animations(int clear_after) {
  srand(time(NULL));
  int rows = 24, cols = 80;

  int anzahl = 80;
  int sx[80], sy[80];
  for (int i = 0; i < anzahl; i++) {
    sx[i] = rand() % cols;
    sy[i] = rand() % rows;
  }

  for (int frame = 0; frame < 15; frame++) {
    printf("\033[2J\033[H");

    for (int i = 0; i < anzahl; i++) {
      printf("\033[%d;%dH", sy[i] + 1, sx[i] + 1);
      if (rand() % 3 == 0)
        printf("\033[38;5;255m*\033[0m");
      else if (rand() % 2 == 0)
        printf("\033[38;5;245m.\033[0m");
      else
        printf("\033[38;5;240m·\033[0m");
    }

    if (frame % 8 == 0) {
      int y = rand() % (rows / 2);
      int x = rand() % (cols - 10);
      printf("\033[%d;%dH\033[38;5;226m✦\033[0m", y + 1, x + 1);
    }

    fflush(stdout);
    usleep(100000);
  }
  if (clear_after)
    printf("\033[2J\033[3J\033[H");
}

void boot_animation(int clear_after) {
  const char *lines[] = {"[ OK ] Loading kernel modules...",
                         "[ OK ] Starting system logger...",
                         "[ OK ] Mounting filesystems...",
                         "[ OK ] Starting network manager...",
                         "[ OK ] Loading user configuration...",
                         "[ OK ] Initializing shell environment...",
                         "[ OK ] Starting mysh v1.0...",
                         NULL};

  printf("\033[2J\033[H");
  for (int i = 0; lines[i] != NULL; i++) {
    printf("\033[38;5;%dm[ OK ]\033[38;5;245m", accent_color);
    printf("%s\n", lines[i] + 5);
    fflush(stdout);
    usleep(100000 + rand() % 80000);
  }
  printf("\033[0m");
  usleep(300000);
  if (clear_after)
    printf("\033[2J\033[3J\033[H");
}

void glitch_animation(int clear_after) {
  const char *glogo[] = {"███╗   ███╗██╗   ██╗███████╗██╗  ██╗",
                         "████╗ ████║╚██╗ ██╔╝██╔════╝██║  ██║",
                         "██╔████╔██║ ╚████╔╝ ███████╗███████║",
                         "██║╚██╔╝██║  ╚██╔╝  ╚════██║██╔══██║",
                         "██║ ╚═╝ ██║   ██║   ███████║██║  ██║",
                         "╚═╝     ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═╝",
                         NULL};

  const char *glitch_chars = "!@#$%^&*<>?/\\|~`+=[]{}";
  int logo_zeilen = 6;
  int cols = 80;
  int logo_x = 20;
  int logo_y = 8;

  srand(time(NULL));

  printf("\033[2J\033[H");
  for (int i = 0; glogo[i] != NULL; i++)
    printf("\033[%d;%dH\033[38;5;255m%s\033[0m", logo_y + i, logo_x, glogo[i]);
  fflush(stdout);
  usleep(600000);

  int glitch_farben[] = {196, 46, 21, 226, 201, 255, 51};
  for (int frame = 0; frame < 20; frame++) {
    for (int i = 0; glogo[i] != NULL; i++)
      printf("\033[%d;%dH\033[38;5;255m%s\033[0m", logo_y + i, logo_x,
             glogo[i]);

    int anz_glitch = rand() % 4 + 1;
    for (int g = 0; g < anz_glitch; g++) {
      int zeile = rand() % logo_zeilen;
      int farbe = glitch_farben[rand() % 7];
      int len = strlen(glogo[zeile]);
      printf("\033[%d;%dH\033[38;5;%dm", logo_y + zeile, logo_x, farbe);
      for (int c = 0; c < len; c++) {
        if (rand() % 3 == 0)
          printf("%c", glitch_chars[rand() % strlen(glitch_chars)]);
        else
          printf("%c", glogo[zeile][c]);
      }
      printf("\033[0m");
    }

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

  int stabilize_farben[] = {196, 255, 196, 255, 255};
  for (int f = 0; f < 5; f++) {
    printf("\033[2J\033[H");
    for (int i = 0; glogo[i] != NULL; i++)
      printf("\033[%d;%dH\033[38;5;%dm%s\033[0m", logo_y + i, logo_x,
             stabilize_farben[f], glogo[i]);
    fflush(stdout);
    usleep(100000);
  }

  usleep(300000);
  if (clear_after)
    printf("\033[2J\033[3J\033[H");
}

void rain_animation(int clear_after) {
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
          printf("\033[%d;%dH\033[38;5;%dm|\033[0m", row + 1, col + 1,
                 accent_color);
        } else if (dist > 0 && dist < 6) {
          int schweif_farbe = accent_color - (dist * 6);
          if (schweif_farbe < 0)
            schweif_farbe = 0;
          printf("\033[%d;%dH\033[38;5;%dm|\033[0m", row + 1, col + 1,
                 schweif_farbe);
        }
      }
      pos[col] += speed[col];
      if (pos[col] > rows + 6)
        pos[col] = -(rand() % 10);
    }
    fflush(stdout);
    usleep(50000);
  }
  if (clear_after)
    printf("\033[2J\033[3J\033[H");
}
