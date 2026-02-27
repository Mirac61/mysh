#pragma once
#include <cstdio>
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

#define MAX_ARGS    64
#define MAX_HISTORY 100
#define MAX_ALIASES 50


// Farben
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

extern char *myhistory[MAX_HISTORY];
extern int history_count;

typedef struct {
    char *name;
    char *value;
} Alias;

extern Alias aliases[MAX_ALIASES];
extern int alias_count;


// ls.cpp
void my_ls(const char *path);

// parse.cpp
void parse(char *input, char **args);
int split_pipes(char *input, char **befehle);
int find_redirect(char **args, int anzahl, char **datei, char *type);
void parse_alias(char *input);
char *find_alias(char *name);

// execute.cpp
void execute(char **args);
void execute_pipe(char **befehle, int anzahl);
void execute_redirect(char **args, char *datei, char type);

// config.cpp
void load_config();
void save_alias(char *name, char *value);
