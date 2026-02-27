#include "shell.h"

char *myhistory[MAX_HISTORY];
int history_count = 0;

int main() {
    char *args[MAX_ARGS];
    char cwd[1024];
    char prompt[1200];
    char *befehle[MAX_ARGS];

    while (1) {
        // Prompt
        getcwd(cwd, sizeof(cwd));
        char *folder = strrchr(cwd, '/');
        folder = folder ? folder + 1 : cwd;
        snprintf(prompt, sizeof(prompt), BOLD "%s" RESET " > ", folder);

        // Input lesen
        char *input = readline(prompt);
        if (input == NULL) break;

        if (strlen(input) > 0) {
            add_history(input);
            myhistory[history_count++] = strdup(input);
        }

        // Pipe check
        int anzahl = split_pipes(input, befehle);

        if (anzahl > 1) {
            execute_pipe(befehle, anzahl);
        } else {
            parse(input, args);

            if (args[0] == NULL) { free(input); continue; }
            if (strcmp(args[0], "exit") == 0) { free(input); break; }

            // cd
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

            // Redirect check
            char *datei = NULL;
            char type;
            int anzahl_args = 0;
            while (args[anzahl_args] != NULL) anzahl_args++;

            // ls
            if (strcmp(args[0], "ls") == 0) {
                if (find_redirect(args, anzahl_args, &datei, &type))
                    execute_redirect(args, datei, type);
                else
                    my_ls(args[1]);
                free(input);
                continue;
            }

            // history
            if (strcmp(args[0], "history") == 0) {
                for (int i = 0; i < history_count; i++) {
                    printf("%d  %s\n", i + 1, myhistory[i]);
                }
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

            // Alle anderen Befehle
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
