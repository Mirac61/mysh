#include "shell.h"
#include <cstring>

void parse(char *input, char **args) {
    int i = 0;
    char *p = input;
    while (*p != '\0') {
        while (*p == ' ' || *p == '\t' || *p == '\n') {
            p++;
            if (*p == '\0') break;
        }
        if (*p == '\0') break;
        if (i >= MAX_ARGS - 1) {
            fprintf(stderr, "Zu viele Argumente (max %d)\n", MAX_ARGS - 1);
            break;
        }
        if (*p == '"') {
            p++;
            args[i++] = p;
            while (*p != '"' && *p != '\0') p++;
            if (*p == '"') *p++ = '\0';
        } else {
            args[i++] = p;
            while (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\0') p++;
            if (*p != '\0') *p++ = '\0';
        }
    }
    args[i] = NULL;
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
    if (alias_count >= MAX_ALIASES) {
        fprintf(stderr, "Maximale Anzahl Aliases (%d) erreicht\n", MAX_ALIASES);
        return;
    }

    *eq = '\0';
    char *name  = input;
    char *value = eq + 1;

    if (strlen(name) > 64) {
        fprintf(stderr, "Alias-Name zu lang (max 64 Zeichen)\n");
        return;
    }
    if (strlen(value) > 512) {
        fprintf(stderr, "Alias-Wert zu lang (max 512 Zeichen)\n");
        return;
    }

    if (value[0] == '"') {
        value++;
        value[strlen(value) - 1] = '\0';
    }

    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            free(aliases[i].value);
            aliases[i].value = strdup(value);
            return;
        }
    }

    aliases[alias_count].name  = strdup(name);
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

int split_semicolon(char *input, char **befehle) {
    int anzahl = 0;
    befehle[anzahl] = strtok(input, ";");
    while (befehle[anzahl] != NULL) {
        anzahl++;
        befehle[anzahl] = strtok(NULL, ";");
    }
    return anzahl;
}

int split_and(char *input, char **befehle) {
    int anzahl = 0;
    char *p = input;
    befehle[anzahl++] = p;
    while ((p = strstr(p, "&&")) != NULL) {
        *p = '\0';
        *(p+1) = '\0';
        p += 2;
        befehle[anzahl++] = p;
    }
    return anzahl;
}

int split_or(char *input, char **befehle) {
    int anzahl = 0;
    char *p = input;
    befehle[anzahl++] = p;
    while ((p = strstr(p, "||")) != NULL) {
        *p = '\0';
        *(p+1) = '\0';
        p += 2;
        befehle[anzahl++] = p;
    }
    return anzahl;
}
