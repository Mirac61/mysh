#include "shell.h"

void parse(char *input, char **args) {
    int i = 0;
    args[i] = strtok(input, " \n");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " \n");
    }
}

int split_pipes(char *input, char **befehle) {
    int anzahl = 0;
    befehle[anzahl] = strtok(input, "|");
    while (befehle[anzahl] != NULL) {
        anzahl++;
        befehle[anzahl] = strtok(NULL, "|");
    }
    return anzahl;
}

int find_redirect(char **args, int anzahl, char **datei, char *type) {
    for (int i = 0; i < anzahl; i++) {
        if (strcmp(args[i], ">>") == 0) {
            *type = 'a';
            *datei = args[i + 1];
            args[i] = NULL;
            return 1;
        } else if (strcmp(args[i], ">") == 0) {
            *type = 'o';
            *datei = args[i + 1];
            args[i] = NULL;
            return 1;
        } else if (strcmp(args[i], "<") == 0) {
            *type = 'i';
            *datei = args[i + 1];
            args[i] = NULL;
            return 1;
        }
    }
    return 0;
}
