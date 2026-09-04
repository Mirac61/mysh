#include "shell.hpp"
#include <array>
#include <vector>

int execute(char **args, bool background) {
  int status;
  sigset_t mask, oldmask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask, &oldmask);

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
    return 1;
  }
  if (pid == 0) {
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
    if (execvp(args[0], args) == -1)
      perror(args[0]);
    exit(1);
  }
  if (background) {
    jobs[job_count].pid = pid;
    strncpy(jobs[job_count].command, args[0],
            sizeof(jobs[job_count].command) - 1);
    jobs[job_count].id = job_count + 1;
    job_count++;
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
    return 0;
  }
  waitpid(pid, &status, 0);
  sigprocmask(SIG_SETMASK, &oldmask, NULL);
  return WEXITSTATUS(status);
}

void execute_pipe(char **befehle, int anzahl) {
  char *args[MAX_ARGS];
  std::vector<std::array<int, 2>> pipes(anzahl - 1);

  for (int i = 0; i < anzahl - 1; i++) {
    if (pipe(pipes[i].data()) < 0) {
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
      expand_tilde(args);
      expand_variables(args);
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
    if (type == 'o' || type == 'e')
      fd = open(datei, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    else if (type == 'a')
      fd = open(datei, O_WRONLY | O_CREAT | O_APPEND, 0644);
    else
      fd = open(datei, O_RDONLY);

    if (fd < 0) {
      perror(datei);
      exit(1);
    }
    dup2(fd, (type == 'i')   ? STDIN_FILENO
             : (type == 'e') ? STDERR_FILENO
                             : STDOUT_FILENO);
    close(fd);
    execvp(args[0], args);
    perror(args[0]);
    exit(1);
  } else {
    wait(NULL);
  }
}

// Wie execute_pipe, aber der Output des letzten Prozesses
// wird über eine extra Pipe an den Elternprozess zurückgegeben
void execute_output(char **befehle, int anzahl, char *result, int result_size) {
  char *args[MAX_ARGS];
  std::vector<std::array<int, 2>> pipes(anzahl - 1);
  // Rückkanal: letzter Prozess → Eltern
  int output_pipe[2];
  pipe(output_pipe);

  for (int i = 0; i < anzahl - 1; i++) {
    if (pipe(pipes[i].data()) < 0) {
      perror("pipe");
      return;
    }
  }

  for (int j = 0; j < anzahl; j++) {
    pid_t pid = fork();
    if (pid == 0) {
      // stdin vom vorherigen Prozess lesen
      if (j > 0)
        dup2(pipes[j - 1][0], STDIN_FILENO);

      // stdout in nächste Pipe schreiben
      if (j < anzahl - 1)
        dup2(pipes[j][1], STDOUT_FILENO);

      // letzter Prozess schreibt in output_pipe statt stdout
      if (j == anzahl - 1)
        dup2(output_pipe[1], STDOUT_FILENO);

      // alle Pipes schließen – Kind braucht sie nicht mehr
      for (int k = 0; k < anzahl - 1; k++) {
        close(pipes[k][0]);
        close(pipes[k][1]);
      }
      close(output_pipe[0]);
      close(output_pipe[1]);

      // parse_simple statt parse() – verhindert Glob Expansion von *
      parse_simple(befehle[j], args);
      execvp(args[0], args);
      perror(args[0]);
      exit(1);
    }
  }

  for (int i = 0; i < anzahl - 1; i++) {
    close(pipes[i][0]);
    close(pipes[i][1]);
  }

  // write-Ende schließen damit read() nicht ewig wartet
  close(output_pipe[1]);
  read(output_pipe[0], result, result_size);
  close(output_pipe[0]);

  // fzf hängt einen Zeilenumbruch an – den entfernen
  result[strcspn(result, "\n")] = 0;

  for (int i = 0; i < anzahl; i++)
    wait(NULL);
}
