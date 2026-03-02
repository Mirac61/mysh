#include "shell.h"
#include <cstdlib>
#include <cstring>

char *myhistory[MAX_HISTORY];
int history_count = 0;
Alias aliases[MAX_ALIASES];
int alias_count = 0;

int color_folder = 17;
int color_branch = 172;
int color_time = 237;
int color_git_clean = 34;
int color_git_dirty = 196;

int startup_method = 1; // 0=kein Logo, 1=sofort, 2=typewriter, 3=fade-in

void expand_tilde(char **args) {
    char *home = getenv("HOME");
    if (!home) return;
    static char bufs[MAX_ARGS][1024];
    for (int i = 0; args[i] != NULL; i++) {
        if (args[i][0] == '~') {
            snprintf(bufs[i], sizeof(bufs[i]), "%s%s", home, args[i] + 1);
            args[i] = bufs[i];
        }
    }
}

int main() {
    char *args[MAX_ARGS];
    char cwd[1024];
    char prompt[2048];
    char *befehle[MAX_ARGS];
    char git_path[1024];
    char branch[256] = "";
    load_config();
    rl_bind_key('\t', rl_complete);
    startup_animation(startup_method);
    // Richtiger Abbruch, damit bei CTRL + C nicht komplette shell abbricht
    signal(SIGINT, SIG_IGN);

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

            // Prompt mit Git
            snprintf(prompt, sizeof(prompt),
                "\001\033[48;5;%dm\033[97m\002  %s \001\033[0m\033[38;5;%dm\033[48;5;%dm\002" PL_RIGHT
                "\001\033[97m\002  %s %s \001\033[0m\033[38;5;%dm\033[48;5;%dm\002" PL_RIGHT
                "\001\033[37m\002  %s \001\033[0m\033[38;5;%dm\002" PL_RIGHT "\001\033[0m\002"
                " \001\033[36m\002❯\001\033[0m\002 ",
                color_folder, folder,
                color_folder, clean ? color_git_clean : color_git_dirty,
                branch, status,
                clean ? color_git_clean : color_git_dirty, color_time,
                uhrzeit, color_time);
        } else {
            // Prompt ohne Git
            snprintf(prompt, sizeof(prompt),
                "\001\033[48;5;%dm\033[97m\002  %s \001\033[0m\033[38;5;%dm\033[48;5;%dm\002" PL_RIGHT
                "\001\033[37m\002  %s \001\033[0m\033[38;5;%dm\002" PL_RIGHT "\001\033[0m\002"
                " \001\033[36m\002❯\001\033[0m\002 ",
                color_folder, folder,
                color_folder, color_time,
                uhrzeit, color_time);
        }

        char *input = readline(prompt);
        if (input == NULL) break;

        char alias_copy[1024]; // für parse_alias
        strncpy(alias_copy, input, sizeof(alias_copy));
        alias_copy[1023] = '\0';

        if (strlen(input) > 0) {
            add_history(input);
            myhistory[history_count++] = strdup(input);
        }

        // Erst nach Semikolon splitten, dann jedes Stück nach Pipes
        char *semi_befehle[MAX_ARGS];
        int anzahl_semi = split_semicolon(input, semi_befehle);

        for (int s = 0; s < anzahl_semi; s++) {
            if (strstr(semi_befehle[s], "&&")) {
                char *and_befehle[MAX_ARGS];
                int anzahl_and = split_and(semi_befehle[s], and_befehle);
                int last_exit = 0;
                for (int a = 0; a < anzahl_and; a++) {
                    if (last_exit != 0) break;
                    char tmp[1024]; strncpy(tmp, and_befehle[a], 1024);
                    parse(tmp, args);
                    expand_tilde(args);
                    if (args[0]) last_exit = execute(args);
                }
                continue;
            }

            if (strstr(semi_befehle[s], "||")) {
                char *or_befehle[MAX_ARGS];
                int anzahl_or = split_or(semi_befehle[s], or_befehle);
                int last_exit = 1;
                for (int o = 0; o < anzahl_or; o++) {
                    if (last_exit == 0) break;
                    char tmp[1024]; strncpy(tmp, or_befehle[o], 1024);
                    parse(tmp, args);
                    expand_tilde(args);
                    if (args[0]) last_exit = execute(args);
                }
                continue;
            }
            // Pipes innerhalb eines Semikolon-Abschnitts
            char *pipes[MAX_ARGS];
            int anzahl_pipes = split_pipes(semi_befehle[s], pipes);

            if (anzahl_pipes > 1) {
                execute_pipe(pipes, anzahl_pipes);
                continue;
            }

            // Normaler Befehl — eigene Kopie pro Befehl da parse() den String verändert
            char input_copy[1024];
            strncpy(input_copy, semi_befehle[s], sizeof(input_copy));
            input_copy[1023] = '\0';

            parse(input_copy, args);

            expand_tilde(args);

            if (args[0] == NULL) continue;

            if (strcmp(args[0], "exit") == 0) {
                printf("\033[0m");
                free(input);
                printf("Bye!\n");
                printf("\033[0m");
                return 0;
            }

            if (strcmp(args[0], "cd") == 0) {
                if (args[1] == NULL)
                    chdir(getenv("HOME"));
                else if (chdir(args[1]) != 0) {
                    printf(RED);
                    perror("cd");
                    printf(RESET);
                }
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
                continue;
            }

            if (strcmp(args[0], "history") == 0) {
                for (int i = 0; i < history_count; i++)
                    printf("%d  %s\n", i + 1, myhistory[i]);
                continue;
            }

            if (strcmp(args[0], "echo") == 0) {
                for (int i = 1; i < anzahl_args; i++) {
                    if (args[i][0] == '$') {
                        // Umgebungsvariable auflösen
                        char *varname = args[i] + 1;
                        char *value = getenv(varname);
                        if (value) printf("%s ", value);
                    } else {
                        printf("%s ", args[i]);
                    }
                }
                printf("\n");
                continue;
            }

            if (strcmp(args[0], "alias") == 0) {
                parse_alias(alias_copy);
                if (alias_count > 0) {
                    save_alias(aliases[alias_count-1].name, aliases[alias_count-1].value);
                    printf(GREEN "Alias '%s' gespeichert!\n" RESET, aliases[alias_count-1].name);
                }
                continue;
            }

            if (strcmp(args[0], "config") == 0) {
                // Config-Datei direkt in nvim öffnen
                char cmd[1024];
                snprintf(cmd, sizeof(cmd), "nvim %s/.myshrc", getenv("HOME"));
                system(cmd);
                continue;
            }

            if (strcmp(args[0], "export") == 0) {
                if (args[1] != NULL) {
                    char *eq = strchr(args[1], '=');
                    if (eq != NULL) {
                        *eq = '\0';
                        setenv(args[1], eq + 1, 1);
                    }
                }
                continue;
            }

            // Alias auflösen falls vorhanden
            char *alias_value = find_alias(args[0]);
            if (alias_value) {
                parse(alias_value, args);
                expand_tilde(args);
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
    printf("\033[0m");
    return 0;
}
