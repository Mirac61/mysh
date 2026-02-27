#include "shell.h"

void execute(char **args) {
    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1)
            printf(RED "Befehl nicht gefunden: %s\n" RESET, args[0]);
        exit(1);
    } else {
        wait(NULL);
    }
}

void execute_pipe(char **befehle, int anzahl) {
    char *args[MAX_ARGS];
    int pipes[anzahl - 1][2];

    for (int i = 0; i < anzahl - 1; i++)
        pipe(pipes[i]);

    for (int j = 0; j < anzahl; j++) {
        pid_t pid = fork();

        if (pid == 0) {
            if (j > 0)
                dup2(pipes[j - 1][0], STDIN_FILENO);
            if (j < anzahl - 1)
                dup2(pipes[j][1], STDOUT_FILENO);

            for (int k = 0; k < anzahl - 1; k++) {
                close(pipes[k][0]);
                close(pipes[k][1]);
            }

            parse(befehle[j], args);
            execvp(args[0], args);
            exit(1);
        }
    }

    for (int i = 0; i < anzahl - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    for (int i = 0; i < anzahl; i++)
        wait(NULL);
}

void execute_redirect(char **args, char *datei, char type) {
    pid_t pid = fork();

    if (pid == 0) {
        int fd;

        if (type == 'o')
            fd = open(datei, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        else if (type == 'a')
            fd = open(datei, O_WRONLY | O_CREAT | O_APPEND, 0644);
        else
            fd = open(datei, O_RDONLY);

        dup2(fd, (type == 'i') ? STDIN_FILENO : STDOUT_FILENO);
        close(fd);
        execvp(args[0], args);
        exit(1);
    } else {
        wait(NULL);
    }
}
