#include "shell.hpp"

#include <string>
#include <vector>

#include <readline/history.h>

static std::vector<std::string> entries;
static char history_path[1024] = "";

// Pfad zu ~/.mysh_history ermitteln – einmal pro Prozess
static bool init_history_path() {
  if (history_path[0] != '\0') {
    return true;
  }
  const char *home = getenv("HOME");
  if (!home) {
    return false;
  }
  snprintf(history_path, sizeof(history_path), "%s/.mysh_history", home);
  return true;
}

void history_load() {
  entries.clear();
  if (!init_history_path()) {
    return;
  }

  FILE *file = fopen(history_path, "r");
  if (!file) {
    return;
  }

  char line[1024];
  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = '\0';
    if (line[0] == '\0') {
      continue;
    }
    entries.push_back(line);
    add_history(line);
  }
  fclose(file);
}

void history_add(const char *line) {
  if (line == NULL || line[0] == '\0') {
    return;
  }
  entries.push_back(line);
  add_history(line);

  if (!init_history_path()) {
    return;
  }
  FILE *file = fopen(history_path, "a");
  if (!file) {
    return;
  }
  fprintf(file, "%s\n", line);
  fclose(file);
}

int history_count() {
  return (int)entries.size();
}

const char *history_at(int n) {
  if (n < 1 || (size_t)n > entries.size()) {
    return NULL;
  }
  return entries[n - 1].c_str();
}

// Lässt den Benutzer per fzf einen alten Befehl auswählen.
// Rückgabe ist ein malloc'ter String oder NULL (Abbruch/keine History).
char *history_pick() {
  if (entries.empty()) {
    return NULL;
  }

  char path[] = "/tmp/mysh_history_XXXXXX";
  int fd = mkstemp(path);
  if (fd < 0) {
    return NULL;
  }
  FILE *file = fdopen(fd, "w");
  if (!file) {
    close(fd);
    unlink(path);
    return NULL;
  }
  // neueste zuerst – so steht der letzte Befehl oben in der Auswahl
  for (size_t i = entries.size(); i > 0; i--) {
    fprintf(file, "%s\n", entries[i - 1].c_str());
  }
  fclose(file);

  char cat_cmd[1080];
  snprintf(cat_cmd, sizeof(cat_cmd), "cat %s", path);
  char *befehle[2] = {cat_cmd, (char *)"fzf"};
  char result[1024] = {0};
  execute_output(befehle, 2, result, sizeof(result) - 1);
  unlink(path);

  if (result[0] == '\0') {
    return NULL;
  }
  return strdup(result);
}
