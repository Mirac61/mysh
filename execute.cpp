#include "shell.h"

int execute(char **args) {
    int status;
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        if (execvp(args[0], args) == -1)
            perror(args[0]);
        exit(1);
    } else {
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

void execute_pipe(char **befehle, int anzahl) {
    char *args[MAX_ARGS];
    int pipes[anzahl - 1][2];

    for (int i = 0; i < anzahl - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return;
        }
    }

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
            perror(args[0]);
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

        if (fd < 0) {
            perror(datei);
            exit(1);
        }
        dup2(fd, (type == 'i') ? STDIN_FILENO : STDOUT_FILENO);
        close(fd);
        execvp(args[0], args);
        perror(args[0]);
        exit(1);
    } else {
        wait(NULL);
    }
}
