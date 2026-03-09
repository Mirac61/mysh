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

int startup_animation_method = 0;
int startup_info_method = 1;
int rain_color = 51;
int boot_color = 46;

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    char cwd[1024];
    char prompt[2048];
    char git_path[1024];
    char branch[256] = "";

    load_config();
    rl_bind_key('\t', rl_complete);
    startup_sequence(startup_animation_method, startup_info_method);
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, sigchld_handler);

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

        if (strlen(input) > 1023) {
            fprintf(stderr, "Eingabe zu lang (max 1023 Zeichen)\n");
            free(input);
            continue;
        }

        char alias_copy[1024];
        strncpy(alias_copy, input, sizeof(alias_copy));
        alias_copy[1023] = '\0';

        if (strlen(input) > 0) {
            add_history(input);
            if (history_count < MAX_HISTORY)
                myhistory[history_count++] = strdup(input);
        }

        run_command(input, alias_copy);
        free(input);
    }

    for (int i = 0; i < history_count; i++) { free(myhistory[i]); myhistory[i] = NULL; }
    for (int i = 0; i < alias_count; i++) { free(aliases[i].name); free(aliases[i].value); }
    printf("Bye!\n");
    printf("\033[0m");
    return 0;
}
