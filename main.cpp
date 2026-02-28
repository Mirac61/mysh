#include "shell.h"
#include <cstring>

char *myhistory[MAX_HISTORY];
int history_count = 0;
Alias aliases[MAX_ALIASES];
int alias_count = 0;

int main() {
    char *args[MAX_ARGS];
    char cwd[1024];
    char prompt[2048];
    char *befehle[MAX_ARGS];
    char git_path[1024];
    char branch[256] = "";
    load_config();

    while (1) {
        getcwd(cwd, sizeof(cwd));
        char *folder = strrchr(cwd, '/');
        folder = folder ? folder + 1 : cwd;

        char uhrzeit[6];
        get_time(uhrzeit);

        if (find_git_root(cwd, git_path)) {
            get_git_branch(git_path, branch);
            int clean = get_git_status(git_path);
            const char *status = clean ? "✓" : "✗";
            const char *bg_status = clean ? "\033[42m" : "\033[41m";
            const char *bg_status_fg = clean ? "\033[32m" : "\033[31m";

            // Mit Git
            snprintf(prompt, sizeof(prompt),
                "\033[48;5;17m\033[97m  %s \033[0m"           // Dunkleres Blau: Ordner
                "\033[38;5;17m\033[48;5;172m" PL_RIGHT         // Pfeil: Blau→Orange
                "\033[97m  %s %s \033[0m"                      // Orange: Branch + Status
                "\033[38;5;172m\033[48;5;237m" PL_RIGHT        // Pfeil: Orange→Grau
                "\033[37m  %s \033[0m"                         // Grau: Uhrzeit
                "\033[38;5;237m" PL_RIGHT "\033[0m"
                " \033[36m❯\033[0m ",
                folder, branch, status, uhrzeit);
        } else {
            // Ohne Git
            snprintf(prompt, sizeof(prompt),
                "\033[48;5;17m\033[97m  %s \033[0m"           // Dunkleres Blau: Ordner
                "\033[38;5;17m\033[48;5;237m" PL_RIGHT         // Pfeil: Blau→Grau
                "\033[37m  %s \033[0m"                         // Grau: Uhrzeit
                "\033[38;5;237m" PL_RIGHT "\033[0m"
                " \033[36m❯\033[0m ",
                folder, uhrzeit);
        }

        char *input = readline(prompt);
        if (input == NULL) break;

        char input_copy[1024];   // für parse
        char alias_copy[1024];   // für parse_alias
        strncpy(input_copy, input, sizeof(input_copy));
        strncpy(alias_copy, input, sizeof(alias_copy));
        input_copy[1023] = '\0';
        alias_copy[1023] = '\0';

        if (strlen(input) > 0) {
            add_history(input);
            myhistory[history_count++] = strdup(input);
        }

        int anzahl = split_pipes(input, befehle);

        if (anzahl > 1) {
            execute_pipe(befehle, anzahl);
        } else {
            parse(input_copy, args);  // input_copy statt input

            if (args[0] == NULL) { free(input); continue; }
            if (strcmp(args[0], "exit") == 0) { free(input); break; }

            if (strcmp(args[0], "cd") == 0) {
                if (args[1] == NULL)
                    chdir(getenv("HOME"));
                else if (chdir(args[1]) != 0) {
                    printf(RED);
                    perror("cd");
                    printf(RESET);
                }
                free(input);
                continue;
            }

            char *datei = NULL;
            char type;
            int anzahl_args = 0;
            while (args[anzahl_args] != NULL) anzahl_args++;

            if (strcmp(args[0], "ls") == 0) {
                bool show_all = false;
                bool show_long = false;
                const char *path = NULL;

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
                free(input);
                continue;
            }

            if (strcmp(args[0], "history") == 0) {
                for (int i = 0; i < history_count; i++)
                    printf("%d  %s\n", i + 1, myhistory[i]);
                free(input);
                continue;
            }

            if (strcmp(args[0], "echo") == 0) {
                for (int i = 1; i < anzahl_args; i++) {
                    if (args[i][0] == '$') {
                        char *varname = args[i] + 1;
                        char *value = getenv(varname);
                        if (value) printf("%s ", value);
                    } else {
                        printf("%s ", args[i]);
                    }
                }
                printf("\n");
                free(input);
                continue;
            }

            if (strcmp(args[0], "alias") == 0) {
                parse_alias(alias_copy);  // alias_copy statt input_copy
                if (alias_count > 0) {
                    save_alias(aliases[alias_count-1].name, aliases[alias_count-1].value);
                    printf(GREEN "Alias '%s' gespeichert!\n" RESET, aliases[alias_count-1].name);
                }
                free(input);
                continue;
            }

            char *alias_value = find_alias(args[0]);
            if (alias_value) {
                parse(alias_value, args);
                anzahl_args = 0;
                while (args[anzahl_args] != NULL) anzahl_args++;
            }

            if (find_redirect(args, anzahl_args, &datei, &type))
                execute_redirect(args, datei, type);
            else
                execute(args);
        }

        free(input);
    }

    printf("Bye!\n");
    return 0;
}
