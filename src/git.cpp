#include "shell.h"
#include <cstdio>
#include <libgen.h>
#include <string.h>
#include <sys/resource.h>

int find_git_root(const char *cwd, char *git_path) {
  char current[1024];
  strncpy(current, cwd, sizeof(current));

  while (strcmp(current, "/") != 0) {
    snprintf(git_path, 1024, "%s/.git", current);
    struct stat st;
    if (stat(git_path, &st) == 0)
      return 1;

    char temp[1024];
    strncpy(temp, current, sizeof(temp));
    char *parent = dirname(temp);
    if (strcmp(parent, current) == 0)
      break;
    strncpy(current, parent, sizeof(current));
  }
  return 0;
}

void get_git_branch(const char *git_path, char *branch) {
  char head_path[1024];
  snprintf(head_path, sizeof(head_path), "%s/HEAD", git_path);

  FILE *file = fopen(head_path, "r");
  if (!file)
    return;

  char line[256];
  fgets(line, sizeof(line), file);
  fclose(file);

  strncpy(branch, line + 16, 256);
  branch[strcspn(branch, "\n")] = '\0';
}

void get_time(char *buf) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  strftime(buf, 6, "%H:%M", t);
}

int get_git_status(const char *git_path) {
  FILE *file = popen("git status --porcelain", "r");
  if (!file)
    return 1;

  char buf[256];
  if (fgets(buf, sizeof(buf), file) == NULL) {
    pclose(file);
    return 1; // clean
  }
  pclose(file);
  return 0; // changes
}
