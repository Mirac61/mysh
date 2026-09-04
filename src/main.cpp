#include "shell.hpp"
#include <cstdlib>
#include <cstring>

char *myhistory[MAX_HISTORY];
int history_count = 0;
Alias aliases[MAX_ALIASES];
int alias_count = 0;

int color_folder = DEFAULT_COLOR_FOLDER;
int color_branch = DEFAULT_COLOR_BRANCH;
int color_time = DEFAULT_COLOR_TIME;
int color_git_clean = DEFAULT_COLOR_GIT_CLEAN;
int color_git_dirty = DEFAULT_COLOR_GIT_DIRTY;

int startup_animation_method = DEFAULT_STARTUP_ANIMATION;
int startup_info_method = DEFAULT_STARTUP_INFO;
int accent_color = DEFAULT_ACCENT_COLOR;
int accent_color_term = DEFAULT_ACCENT_COLOR_TERM;

bool background = false;
Job jobs[MAX_JOBS];
int job_count = 0;
int status = 0;

void sigchld_handler(int sig) {
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}

int main() {
  char cwd[1024];
  char prompt[2048];
  char git_path[1024];
  char branch[256] = "";

  load_config();
  // load_history();
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
      snprintf(
          prompt, sizeof(prompt),
          "\001\033[48;5;%dm\033[97m\002  %s "
          "\001\033[0m\002\001\033[38;5;%dm\033[48;5;%dm\002" PL_RIGHT
          "\001\033[97m\002  %s %s "
          "\001\033[0m\002\001\033[38;5;%dm\033[48;5;%dm\002" PL_RIGHT
          "\001\033[37m\002  %s \001\033[0m\002\001\033[38;5;%dm\002" PL_RIGHT
          "\001\033[0m\002"
          " \001\033[36m\002❯\001\033[0m\002 ",
          color_folder, folder, color_folder,
          clean ? color_git_clean : color_git_dirty, branch, status,
          clean ? color_git_clean : color_git_dirty, color_time, uhrzeit,
          color_time);
    } else {
      snprintf(
          prompt, sizeof(prompt),
          "\001\033[48;5;%dm\033[97m\002  %s "
          "\001\033[0m\002\001\033[38;5;%dm\033[48;5;%dm\002" PL_RIGHT
          "\001\033[37m\002  %s \001\033[0m\002\001\033[38;5;%dm\002" PL_RIGHT
          "\001\033[0m\002"
          " \001\033[36m\002❯\001\033[0m\002 ",
          color_folder, folder, color_folder, color_time, uhrzeit, color_time);
    }

    char *input = readline(prompt);
    if (input == NULL)
      break;

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
      // add_to_history(input);
    }

    run_command(input, alias_copy);
    free(input);
  }

  // save_history();
  for (int i = 0; i < history_count; i++) {
    free(myhistory[i]);
    myhistory[i] = NULL;
  }
  for (int i = 0; i < alias_count; i++) {
    free(aliases[i].name);
    free(aliases[i].value);
  }
  printf("Bye!\n");
  printf("\033[0m");
  return 0;
}
