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

void parse_alias(char *input) {
    input += 6;
    input[strcspn(input, "\n")] = '\0';

    char *eq = strchr(input, '=');
    if (eq == NULL) {
        printf(RED "alias: falsches Format. Beispiel: alias ll=\"ls -la\"\n" RESET);
        return;
    }
    *eq = '\0';
    char *name = input;
    char *value = eq + 1;

    if (value[0] == '"') {
        value++;
        value[strlen(value) - 1] = '\0';
    }

    aliases[alias_count].name = strdup(name);
    aliases[alias_count].value = strdup(value);
    alias_count++;
}

char *find_alias(char *name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0)
            return aliases[i].value;
    }
    return NULL;
}
