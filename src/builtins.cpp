#include "shell.h"
#include <cstdlib>
#include <cstring>

int run_builtin(char **args, int anzahl_args, char *alias_copy) {
    if (args[0] == NULL) return 0;

    if (strcmp(args[0], "exit") == 0) {
        printf("\033[0m");
        printf("Bye!\n");
        for (int i = 0; i < history_count; i++) { free(myhistory[i]); myhistory[i] = NULL; }
        for (int i = 0; i < alias_count; i++) { free(aliases[i].name); free(aliases[i].value); }
        exit(0);
    }

    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL){
            char *home = getenv("HOME");
            if (home) chdir(home);
        }
        else if (chdir(args[1]) != 0) {
            printf(RED);
            perror("cd");
            printf(RESET);
        }
        return 1;
    }

    if (strcmp(args[0], "reload") == 0) {
        reload_config();
        printf(GREEN "Konfiguration neu geladen!\n" RESET);
        return 1;
    }

    if (strcmp(args[0], "export") == 0) {
        if (args[1] != NULL) {
            char *eq = strchr(args[1], '=');
            if (eq != NULL) {
                *eq = '\0';
                setenv(args[1], eq + 1, 1);
            }
        }
        return 1;
    }

    if (strcmp(args[0], "history") == 0) {
        for (int i = 0; i < history_count; i++)
            printf("%d  %s\n", i + 1, myhistory[i]);
        return 1;
    }

    if (strcmp(args[0], "alias") == 0) {
        parse_alias(alias_copy);
        if (alias_count > 0) {
            save_alias(aliases[alias_count-1].name, aliases[alias_count-1].value);
            printf(GREEN "Alias '%s' gespeichert!\n" RESET, aliases[alias_count-1].name);
        }
        return 1;
    }

    if (strcmp(args[0], "config") == 0) {
        char cmd[1024];
        char *home = getenv("HOME");
        if (!home) { fprintf(stderr, "HOME nicht gesetzt\n"); return 1; }
        snprintf(cmd, sizeof(cmd), "nvim %s/.myshrc", home);
        system(cmd);
        return 1;
    }

    if (strcmp(args[0], "jobs") == 0) {
        print_jobs();
        return 1;
    }

    if (strcmp(args[0], "fg") == 0) {
        int job_id = atoi(args[1]);
        for (int i = 0; i < job_count; i++) {
            if (jobs[i].id == job_id) {
                waitpid(jobs[i].pid, NULL, 0);
                memmove(&jobs[i], &jobs[i+1], sizeof(Job) * (job_count - i - 1));
                job_count--;
                return 1;
            }
        }
    }


    if (strcmp(args[0], "sf") == 0) {
        char find_cmd[256];
        char cmd[1024];
        char result[1024] = {0};
        char *exec_args[MAX_ARGS];

        // mit Filter nur passende Dateien anzeigen
        if (args[1] != NULL)
            snprintf(find_cmd, sizeof(find_cmd),
                     "find . -type f -name *%s*", args[1]);
        else
            snprintf(find_cmd, sizeof(find_cmd), "find . -type f");

        char *befehle[2] = {find_cmd, (char*)"fzf"};
        execute_output(befehle, 2, result, sizeof(result));

        // User hat ESC gedrückt oder nichts ausgewählt
        if (strlen(result) == 0) return 1;

        // $EDITOR benutzen, fallback auf nvim
        char *editor = getenv("EDITOR");
        if (!editor) editor = (char*)"nvim";
        snprintf(cmd, sizeof(cmd), "%s %s", editor, result);
        parse(cmd, exec_args);
        execute(exec_args, background);
        return 1;
    }

    if (strcmp(args[0], "sd") == 0) {
        char find_cmd[256];
        char result[1024] = {0};

        // mit Filter nur passende Ordner anzeigen
        if (args[1] != NULL)
            snprintf(find_cmd, sizeof(find_cmd),
                     "find . -type d -name *%s*", args[1]);
        else
            snprintf(find_cmd, sizeof(find_cmd), "find . -type d");

        char *befehle[2] = {find_cmd, (char*)"fzf"};
        execute_output(befehle, 2, result, sizeof(result));

        if (strlen(result) > 0)
            chdir(result);
        return 1;
    }

    return 0;
}
