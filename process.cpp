#include "shell.h"

void process_input(char *cmd, char *alias_copy) {
    char *args[MAX_ARGS];
    char input_copy[1024];
    strncpy(input_copy, cmd, sizeof(input_copy));
    input_copy[1023] = '\0';

    parse(input_copy, args);
    expand_tilde(args);

    if (args[0] == NULL) return;

    int anzahl_args = 0;
    while (args[anzahl_args] != NULL) anzahl_args++;

    // Builtins (cd, echo, export, alias, history, config, exit)
    if (run_builtin(args, anzahl_args, alias_copy)) return;

    // echo mit $VAR Support und Redirect
    if (strcmp(args[0], "echo") == 0) {
        char *datei = NULL;
        char type;
        for (int i = 1; i < anzahl_args; i++) {
            if (args[i] && args[i][0] == '$') {
                char *value = getenv(args[i] + 1);
                args[i] = value ? value : (char *)"";
            }
        }
        if (find_redirect(args, anzahl_args, &datei, &type))
            execute_redirect(args, datei, type);
        else {
            for (int i = 1; args[i] != NULL; i++)
                printf("%s ", args[i]);
            printf("\n");
        }
        return;
    }

    // ls separat wegen Redirect und Flags
    if (strcmp(args[0], "ls") == 0) {
        bool show_all = false;
        bool show_long = false;
        const char *path = NULL;
        char *datei = NULL;
        char type;
        for (int i = 1; i < anzahl_args; i++) {
            if (strcmp(args[i], "-la") == 0 || strcmp(args[i], "-al") == 0) { show_all = true; show_long = true; }
            else if (strcmp(args[i], "-l") == 0) show_long = true;
            else if (strcmp(args[i], "-a") == 0) show_all = true;
            else path = args[i];
        }
        if (find_redirect(args, anzahl_args, &datei, &type))
            execute_redirect(args, datei, type);
        else
            my_ls(path, show_all, show_long);
        return;
    }

    // Alias auflösen falls vorhanden
    char *alias_value = find_alias(args[0]);
    if (alias_value) {
        parse(alias_value, args);
        expand_tilde(args);
        anzahl_args = 0;
        while (args[anzahl_args] != NULL) anzahl_args++;
    }

    // Redirect oder normale Ausführung
    char *datei = NULL;
    char type;
    if (find_redirect(args, anzahl_args, &datei, &type))
        execute_redirect(args, datei, type);
    else
        execute(args);
}

void run_command(char *input, char *alias_copy) {
    // Erst Semikolon, dann && / ||, dann Pipes
    char *semi_befehle[MAX_ARGS];
    int anzahl_semi = split_semicolon(input, semi_befehle);

    for (int s = 0; s < anzahl_semi; s++) {
        // && — nächster Befehl nur bei Erfolg
        if (strstr(semi_befehle[s], "&&")) {
            char *and_befehle[MAX_ARGS];
            int anzahl_and = split_and(semi_befehle[s], and_befehle);
            int last_exit = 0;
            for (int a = 0; a < anzahl_and; a++) {
                if (last_exit != 0) break;
                char tmp[1024]; strncpy(tmp, and_befehle[a], 1024);
                char *args[MAX_ARGS];
                parse(tmp, args);
                expand_tilde(args);
                if (args[0]) last_exit = execute(args);
            }
            continue;
        }

        // || — nächster Befehl nur bei Fehler
        if (strstr(semi_befehle[s], "||")) {
            char *or_befehle[MAX_ARGS];
            int anzahl_or = split_or(semi_befehle[s], or_befehle);
            int last_exit = 1;
            for (int o = 0; o < anzahl_or; o++) {
                if (last_exit == 0) break;
                char tmp[1024]; strncpy(tmp, or_befehle[o], 1024);
                char *args[MAX_ARGS];
                parse(tmp, args);
                expand_tilde(args);
                if (args[0]) last_exit = execute(args);
            }
            continue;
        }

        // Pipes
        char *pipes[MAX_ARGS];
        int anzahl_pipes = split_pipes(semi_befehle[s], pipes);
        if (anzahl_pipes > 1) {
            execute_pipe(pipes, anzahl_pipes);
            continue;
        }

        process_input(semi_befehle[s], alias_copy);
    }
}
