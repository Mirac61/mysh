#include "shell.h"

void my_ls(const char *path) {
    DIR *dir = opendir(path ? path : ".");
    if (dir == NULL) {
        printf(RED "ls: Verzeichnis nicht gefunden: %s\n" RESET, path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char *ext = strrchr(entry->d_name, '.');

        if (entry->d_type == DT_DIR) {
            printf(BLUE BOLD "%s  " RESET, entry->d_name);
            continue;
        }

        if (ext != NULL) {
            if (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 || strcmp(ext, ".h") == 0)
                printf(CYAN "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".js") == 0 || strcmp(ext, ".ts") == 0 || strcmp(ext, ".jsx") == 0 || strcmp(ext, ".tsx") == 0)
                printf(YELLOW "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0)
                printf(ORANGE "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".css") == 0 || strcmp(ext, ".scss") == 0 || strcmp(ext, ".sass") == 0)
                printf(PINK "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".json") == 0)
                printf(LIGHT_BLUE "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".md") == 0 || strcmp(ext, ".txt") == 0)
                printf(WHITE "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".sh") == 0)
                printf(GREEN "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".zip") == 0 || strcmp(ext, ".tar") == 0 || strcmp(ext, ".gz") == 0)
                printf(MAGENTA "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".vue") == 0 || strcmp(ext, ".svelte") == 0)
                printf(GREEN "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".java") == 0)
                printf(ORANGE "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".py") == 0)
                printf(YELLOW "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".xml") == 0)
                printf(LIGHT_BLUE "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".rs") == 0)
                printf(ORANGE "%s  " RESET, entry->d_name);
            else if (strcmp(ext, ".go") == 0)
                printf(CYAN "%s  " RESET, entry->d_name);
            else
                printf(GRAY "%s  " RESET, entry->d_name);
        } else {
            struct stat st;
            stat(entry->d_name, &st);
            if (st.st_mode & S_IXUSR)
                printf(GREEN BOLD "%s  " RESET, entry->d_name);
            else
                printf(GRAY "%s  " RESET, entry->d_name);
        }
    }
    printf("\n");
    closedir(dir);
}
