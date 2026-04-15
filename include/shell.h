#pragma once
#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_ARGS    64
#define MAX_HISTORY 100
#define MAX_ALIASES 50

// Farben (Text)
#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define ORANGE      "\033[38;5;214m"
#define PINK        "\033[38;5;213m"
#define LIGHT_BLUE  "\033[38;5;117m"
#define GRAY        "\033[38;5;245m"
#define BOLD        "\033[1m"

// Prompt Farben
#define BG_CYAN     "\033[46m"
#define BG_YELLOW   "\033[43m"
#define FG_CYAN     "\033[36m"
#define FG_YELLOW   "\033[33m"
#define FG_BLACK    "\033[30m"

// Powerline
#define PL_RIGHT    "\ue0b0"
#define PL_LEFT     "\ue0b2"

extern char *myhistory[MAX_HISTORY];
extern int history_count;

typedef struct {
    char *name;
    char *value;
} Alias;

#define MAX_JOBS 100

typedef struct {
    int id;
    pid_t pid;
    char command[1024];
    bool running;
} Job;

extern Job jobs[MAX_JOBS];
extern int job_count;

extern Alias aliases[MAX_ALIASES];
extern int alias_count;

extern int color_folder;
extern int color_branch;
extern int color_time;
extern int color_git_clean;
extern int color_git_dirty;

extern int startup_animation_method;
extern int startup_info_method;
extern int accent_color;
extern int accent_color_term;

extern bool background;

// ls.cpp
void my_ls(const char *path, bool show_all, bool show_long);

// parse.cpp
void parse(char *input, char **args);
int split_pipes(char *input, char **befehle);
int find_redirect(char **args, int anzahl, char **datei, char *type);
void parse_alias(char *input);
char *find_alias(char *name);
int split_semicolon(char *input, char **befehle);
int split_and(char *input, char **befehle);
int split_or(char *input, char **befehle);
void expand_tilde(char **args);
void parse_simple(char *cmd, char **args);
void expand_variables(char **args);

// execute.cpp
int execute(char **args, bool background);
void execute_pipe(char **befehle, int anzahl);
void execute_redirect(char **args, char *datei, char type);
void execute_output(char **befehle, int anzahl, char *result, int result_size);

// config.cpp
void load_config();
void save_alias(char *name, char *value);

// git.cpp
int find_git_root(const char *cwd, char *git_path);
void get_git_branch(const char *git_path, char *branch);
void get_time(char *buf);
int get_git_status(const char *git_path);

// startup.cpp
void startup_sequence(int animation, int info);

// builtins.cpp
int run_builtin(char **args, int anzahl_args, char *alias_copy);

// process.cpp
void process_input(char *input, char *alias_copy);
void run_command(char *cmd, char *alias_copy);
void print_jobs();
void remove_finished_jobs();
