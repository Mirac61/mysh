#include "shell.h"

const char* get_color(const char *name, struct stat *st) {
    const char *ext = strrchr(name, '.');
    if (S_ISDIR(st->st_mode)) return BLUE BOLD;
    if (st->st_mode & S_IXUSR) return GREEN BOLD;
    if (!ext) return GRAY;
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 || strcmp(ext, ".h") == 0) return CYAN;
    if (strcmp(ext, ".js") == 0 || strcmp(ext, ".ts") == 0 || strcmp(ext, ".jsx") == 0 || strcmp(ext, ".tsx") == 0) return YELLOW;
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return ORANGE;
    if (strcmp(ext, ".css") == 0 || strcmp(ext, ".scss") == 0 || strcmp(ext, ".sass") == 0) return PINK;
    if (strcmp(ext, ".json") == 0) return LIGHT_BLUE;
    if (strcmp(ext, ".md") == 0 || strcmp(ext, ".txt") == 0) return WHITE;
    if (strcmp(ext, ".sh") == 0 || strcmp(ext, ".vue") == 0 || strcmp(ext, ".svelte") == 0) return GREEN;
    if (strcmp(ext, ".zip") == 0 || strcmp(ext, ".tar") == 0 || strcmp(ext, ".gz") == 0) return MAGENTA;
    if (strcmp(ext, ".java") == 0 || strcmp(ext, ".html") == 0 || strcmp(ext, ".rs") == 0) return ORANGE;
    if (strcmp(ext, ".py") == 0) return YELLOW;
    if (strcmp(ext, ".xml") == 0) return LIGHT_BLUE;
    if (strcmp(ext, ".go") == 0) return CYAN;
    return GRAY;
}

void my_ls(const char *path, bool show_all, bool show_long) {
    const char *dir_path = path ? path : ".";
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        printf(RED "ls: Verzeichnis nicht gefunden: %s\n" RESET, path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        stat(full_path, &st);

        const char *color = get_color(entry->d_name, &st);

        if (show_long) {
            // Permissions
            printf("%c%c%c%c%c%c%c%c%c%c  ",
                S_ISDIR(st.st_mode) ? 'd' : '-',
                st.st_mode & S_IRUSR ? 'r' : '-',
                st.st_mode & S_IWUSR ? 'w' : '-',
                st.st_mode & S_IXUSR ? 'x' : '-',
                st.st_mode & S_IRGRP ? 'r' : '-',
                st.st_mode & S_IWGRP ? 'w' : '-',
                st.st_mode & S_IXGRP ? 'x' : '-',
                st.st_mode & S_IROTH ? 'r' : '-',
                st.st_mode & S_IWOTH ? 'w' : '-',
                st.st_mode & S_IXOTH ? 'x' : '-');

            // Größe
            printf("%8lld  ", (long long)st.st_size);

            // Datum
            char timebuf[20];
            strftime(timebuf, sizeof(timebuf), "%d %b %H:%M", localtime(&st.st_mtime));
            printf("%s  ", timebuf);

            // Name mit Farbe
            printf("%s%s" RESET "\n", color, entry->d_name);
        } else {
            printf("%s%s  " RESET, color, entry->d_name);
        }
    }

    if (!show_long) printf("\n");
    closedir(dir);
}
