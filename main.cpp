#include "shell.h"
#include <cstring>

char *myhistory[MAX_HISTORY];
int history_count = 0;
Alias aliases[MAX_ALIASES];
int alias_count = 0;

int main() {
    char *args[MAX_ARGS];
    char cwd[1024];
    char prompt[1200];
    char *befehle[MAX_ARGS];
    load_config();

    while (1) {
        getcwd(cwd, sizeof(cwd));
        char *folder = strrchr(cwd, '/');
        folder = folder ? folder + 1 : cwd;
        snprintf(prompt, sizeof(prompt), BOLD "%s" RESET " > ", folder);

        char *input = readline(prompt);
        if (input == NULL) break;

        char input_copy[1024];
        strncpy(input_copy, input, sizeof(input_copy));
        input_copy[1023] = '\0';

        if (strlen(input) > 0) {
            add_history(input);
            myhistory[history_count++] = strdup(input);
        }

        int anzahl = split_pipes(input, befehle);

        if (anzahl > 1) {
            execute_pipe(befehle, anzahl);
        } else {
            parse(input, args);

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
                if (find_redirect(args, anzahl_args, &datei, &type))
                    execute_redirect(args, datei, type);
                else
                    my_ls(args[1]);
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
                parse_alias(input_copy);
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
