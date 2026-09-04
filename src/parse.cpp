#include "shell.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

void parse(char *input, char **args) {
  int i = 0;
  char *p = input;
  while (*p != '\0') {
    while (*p == ' ' || *p == '\t' || *p == '\n') {
      p++;
      if (*p == '\0')
        break;
    }
    if (*p == '\0')
      break;
    if (i >= MAX_ARGS - 1) {
      fprintf(stderr, "Zu viele Argumente (max %d)\n", MAX_ARGS - 1);
      break;
    }
    if (*p == '"') {
      p++;
      args[i++] = p;
      while (*p != '"' && *p != '\0')
        p++;
      if (*p == '"')
        *p++ = '\0';
    } else {
      args[i++] = p;
      while (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\0')
        p++;
      if (*p != '\0')
        *p++ = '\0';
    }
  }
  args[i] = NULL;
}

int split_pipes(char *input, char **befehle) {
  int anzahl = 0;
  befehle[anzahl] = strtok(input, "|");
  while (befehle[anzahl] != NULL) {
    anzahl++;
    befehle[anzahl] = strtok(NULL, "|");
  }
  return anzahl;
}

int find_redirect(char **args, int anzahl, char **datei, char *type) {
  for (int i = 0; i < anzahl; i++) {
    if (strcmp(args[i], ">>") == 0) {
      *type = 'a';
      *datei = args[i + 1];
      args[i] = NULL;
      return 1;
    } else if (strcmp(args[i], ">") == 0) {
      *type = 'o';
      *datei = args[i + 1];
      args[i] = NULL;
      return 1;
    } else if (strcmp(args[i], "<") == 0) {
      *type = 'i';
      *datei = args[i + 1];
      args[i] = NULL;
      return 1;
    } else if (strncmp(args[i], "2>", 2) == 0) {
      *type = 'e';
      *datei = args[i] + 2;
      args[i] = NULL;
      return 1;
    }
  }
  return 0;
}

void parse_alias(char *input) {
  input += 6;
  input[strcspn(input, "\n")] = '\0';

  char *eq = strchr(input, '=');
  if (eq == NULL) {
    printf(RED "alias: falsches Format. Beispiel: alias ll=\"ls -la\"\n" RESET);
    return;
  }
  if (alias_count >= MAX_ALIASES) {
    fprintf(stderr, "Maximale Anzahl Aliases (%d) erreicht\n", MAX_ALIASES);
    return;
  }

  *eq = '\0';
  char *name = input;
  char *value = eq + 1;

  if (strlen(name) > 64) {
    fprintf(stderr, "Alias-Name zu lang (max 64 Zeichen)\n");
    return;
  }
  if (strlen(value) > 512) {
    fprintf(stderr, "Alias-Wert zu lang (max 512 Zeichen)\n");
    return;
  }

  if (value[0] == '"') {
    value++;
    value[strlen(value) - 1] = '\0';
  }

  for (int i = 0; i < alias_count; i++) {
    if (strcmp(aliases[i].name, name) == 0) {
      free(aliases[i].value);
      aliases[i].value = strdup(value);
      return;
    }
  }

  aliases[alias_count].name = strdup(name);
  aliases[alias_count].value = strdup(value);
  alias_count++;
}

char *find_alias(char *name) {
  for (int i = 0; i < alias_count; i++) {
    if (strcmp(aliases[i].name, name) == 0)
      return aliases[i].value;
  }
  return NULL;
}

int split_semicolon(char *input, char **befehle) {
  int anzahl = 0;
  befehle[anzahl] = strtok(input, ";\n");
  while (befehle[anzahl] != NULL) {
    anzahl++;
    befehle[anzahl] = strtok(NULL, ";\n");
  }
  return anzahl;
}

int split_and(char *input, char **befehle) {
  int anzahl = 0;
  char *p = input;
  befehle[anzahl++] = p;
  while ((p = strstr(p, "&&")) != NULL) {
    *p = '\0';
    *(p + 1) = '\0';
    p += 2;
    befehle[anzahl++] = p;
  }
  return anzahl;
}

int split_or(char *input, char **befehle) {
  int anzahl = 0;
  char *p = input;
  befehle[anzahl++] = p;
  while ((p = strstr(p, "||")) != NULL) {
    *p = '\0';
    *(p + 1) = '\0';
    p += 2;
    befehle[anzahl++] = p;
  }
  return anzahl;
}

// ~ durch HOME ersetzen
void expand_tilde(char **args) {
  char *home = getenv("HOME");
  if (!home)
    return;
  static char bufs[MAX_ARGS][1024];
  for (int i = 0; args[i] != NULL; i++) {
    if (args[i][0] == '~') {
      snprintf(bufs[i], sizeof(bufs[i]), "%s%s", home, args[i] + 1);
      args[i] = bufs[i];
    }
  }
}

// Splittet einen String nur by Space – keine Glob Expansion wie parse()
void parse_simple(char *cmd, char **args) {
  int i = 0;
  char *token = strtok(cmd, " ");
  while (token != NULL) {
    args[i++] = token;
    token = strtok(NULL, " ");
  }
  // execvp erwartet NULL am Ende
  args[i] = NULL;
}

void expand_variables(char **args) {
  static char buffer[MAX_ARGS][1024];

  for (int i = 0; args[i] != NULL; i++) {
    const char *source = args[i];
    if (!strchr(source, '$'))
      continue;

    char *destination = buffer[i];
    size_t remaining = sizeof(buffer[i]) - 1;
    while (*source && remaining > 0) {

      if (*source == '$') {
        // '$' überspringen
        source++;
        char name[256];
        size_t n = 0;
        if (*source == '$') {
          source++;
          char pid_str[32];
          snprintf(pid_str, sizeof(pid_str), "%d", getpid());
          const char *val = pid_str;
          while (*val && remaining > 0) {
            *destination++ = *val++;
            remaining--;
          }
          continue;
        }

        while (*source && (isalnum((unsigned char)*source) || *source == '_')) {
          if (n + 1 < sizeof(name)) {
            name[n++] = *source;
          }
          source++;
        }

        name[n] = '\0';

        // Wert holen, Wenn !Wert -> leerer String
        const char *val = getenv(name);
        if (!val)
          val = "";

        while (*val && remaining > 0) {
          *destination++ = *val++;
          remaining--;
        }
      } else {
        *destination++ = *source++;
        remaining--;
      }
    }
    // String terminieren
    *destination = '\0';
    args[i] = buffer[i];
  }
}
