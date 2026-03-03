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

int startup_method = 1;

// ~ durch HOME ersetzen
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
    signal(SIGINT, SIG_IGN); // Shell ignoriert Ctrl+C, Kindprozesse nicht

    while (1) {
        getcwd(cwd, sizeof(cwd));
        char *folder = strrchr(cwd, '/');
        folder = folder ? folder + 1 : cwd;

        char uhrzeit[6];
        get_time(uhrzeit);

        // Prompt mit oder ohne Git-Info aufbauen
        if (find_git_root(cwd, git_path)) {
            get_git_branch(git_path, branch);
            int clean = get_git_status(git_path);
            const char *status = clean ? "✓" : "✗";
            snprintf(prompt, sizeof(prompt),
                "\001\033[48;5;%dm\033[97m\002  %s \001\033[0m\002\001\033[38;5;%dm\033[48;5;%dm\002" PL_RIGHT
                "\001\033[97m\002  %s %s \001\033[0m\002\001\033[38;5;%dm\033[48;5;%dm\002" PL_RIGHT
                "\001\033[37m\002  %s \001\033[0m\002\001\033[38;5;%dm\002" PL_RIGHT "\001\033[0m\002"
                " \001\033[36m\002❯\001\033[0m\002 ",
                color_folder, folder,
                color_folder, clean ? color_git_clean : color_git_dirty,
                branch, status,
                clean ? color_git_clean : color_git_dirty, color_time,
                uhrzeit, color_time);
        } else {
            snprintf(prompt, sizeof(prompt),
                "\001\033[48;5;%dm\033[97m\002  %s \001\033[0m\002\001\033[38;5;%dm\033[48;5;%dm\002" PL_RIGHT
                "\001\033[37m\002  %s \001\033[0m\002\001\033[38;5;%dm\002" PL_RIGHT "\001\033[0m\002"
                " \001\033[36m\002❯\001\033[0m\002 ",
                color_folder, folder,
                color_folder, color_time,
                uhrzeit, color_time);
        }

        char *input = readline(prompt);
        if (input == NULL) break;

        // Kopie für parse_alias da parse() den String verändert
        char alias_copy[1024];
        strncpy(alias_copy, input, sizeof(alias_copy));
        alias_copy[1023] = '\0';

        if (strlen(input) > 0) {
            add_history(input);
            myhistory[history_count++] = strdup(input);
        }

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

            // Normaler Befehl
            char input_copy[1024];
            strncpy(input_copy, semi_befehle[s], sizeof(input_copy));
            input_copy[1023] = '\0';

            parse(input_copy, args);
            expand_tilde(args);

            if (args[0] == NULL) continue;

            int anzahl_args = 0;
            while (args[anzahl_args] != NULL) anzahl_args++;

            // Builtins (cd, echo, export, alias, history, config, exit)
            if (run_builtin(args, anzahl_args, alias_copy)) continue;

            // echo mit $VAR Support und Redirect
            if (strcmp(args[0], "echo") == 0) {
                char *datei = NULL;
                char type;
                // $VAR expandieren
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
                continue;
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

            // Redirect oder normale Ausführung
            char *datei = NULL;
            char type;
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
