#include "shell.h"
#include <cstdio>

void load_config() {
    char path[1024];
    char *home = getenv("HOME");
    if (!home) return;
    snprintf(path, sizeof(path), "%s/.myshrc", home);

    FILE *file = fopen(path, "r");
    if (!file) return;

    char line[1024];
    while(fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "alias", 5) == 0)
        {
            parse_alias(line);
        }
        if (strncmp(line, "prompt_folder=", 14) == 0)
            color_folder = atoi(line + 14);
        if (strncmp(line, "prompt_branch=", 14) == 0)
            color_branch = atoi(line + 14);
        if (strncmp(line, "prompt_time=", 12) == 0)
            color_time = atoi(line + 12);
        if (strncmp(line, "prompt_git_clean=", 17) == 0)
            color_git_clean = atoi(line + 17);
        if (strncmp(line, "prompt_git_dirty=", 17) == 0)
            color_git_dirty = atoi(line + 17);
        if (strncmp(line, "startup_animation=", 18) == 0)
            startup_animation_method = atoi(line + 18);
        if (strncmp(line, "startup_info=", 13) == 0)
            startup_info_method = atoi(line + 13);
        if (strncmp(line, "accent_color=", 11) == 0)
            accent_color = atoi(line + 11);
        if (strncmp(line, "accent_color_term=", 18) == 0)
            accent_color_term = atoi(line + 18);
    }
    fclose(file);
}

void save_alias(char *name, char *value) {
    char path[1024];
    char *home = getenv("HOME");
    if (!home) return;
    snprintf(path, sizeof(path), "%s/.myshrc", home);
    FILE *file = fopen(path, "a");
    if (!file) return;

    fprintf(file, "alias %s=\"%s\"\n", name, value);

    fclose(file);
}
