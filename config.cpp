#include "shell.h"
#include <cstdio>

void load_config() {
    char path[1024];
    snprintf(path, sizeof(path), "%s/.myshrc", getenv("HOME"));

    FILE *file = fopen(path, "r");
    if (!file) return;

    char line[1024];
    while(fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "alias", 5) == 0)
        {
            parse_alias(line);
        }
    }
    fclose(file);
}

void save_alias(char *name, char *value) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/.myshrc", getenv("HOME"));
    FILE *file = fopen(path, "a");
    if (!file) return;

    fprintf(file, "alias %s=\"%s\"\n", name, value);

    fclose(file);
}
