# mysh — Notizen vom 08.09.2026

## Was erledigt ist

**Valgrind-Lauf sauber.**
```bash
g++ -g -O0 -Wall -Wextra src/*.cpp -o shell -Iinclude -lreadline
valgrind --leak-check=full --track-origins=yes ./shell < cmds.txt
```
Ergebnis: `0 errors`, keine definitely/indirectly/possibly lost Blöcke.
Die ~241 KB "still reachable" sind readline-intern, nicht unser Problem.

**Startup-Performance: 4.1 ms → 1.5 ms.**

Ursache war `git status --porcelain` via `popen()` bei jedem Prompt-Aufbau —
auch wenn stdin gar kein Terminal ist. `strace -c` zeigte 67 % der Syscall-Zeit
in `wait4`.

Fix: `bool interactive = isatty(STDIN_FILENO);` und Prompt-Rendering
(inkl. Git-Aufruf und Startup-Animation) nur im interaktiven Fall.

| Shell | Startup |
|---|---|
| dash | 0.53 ms |
| bash | 0.85 ms |
| zsh | 0.92 ms |
| **mysh (nicht-interaktiv)** | **1.5 ms** |
| mysh mit Git-Prompt | 4.0 ms |

**clangd-Setup.** `bear -- make` erzeugt `compile_commands.json`.
Gehört in `.gitignore`.

---

## Offene Punkte

### 1. Bugs in `git.cpp`

**`strncpy` ohne Nullterminator** (3× in `find_git_root`).
Bei Pfaden ≥ 1024 Zeichen bleibt der String nicht terminiert → Overread.
```cpp
snprintf(current, sizeof(current), "%s", cwd);  // terminiert immer
```

**`line + 16` in `get_git_branch`** bricht bei detached HEAD.
Dort steht ein roher 40-Zeichen-Hash ohne `ref: refs/heads/`-Präfix.
Auf Präfix prüfen statt blind zu schneiden. Ausserdem `fgets`-Rückgabe prüfen.

**`snprintf`-Truncation** in `git.cpp:11` und `builtins.cpp:122`.
Rückgabewert prüfen:
```cpp
int n = snprintf(buf, size, ...);
if (n < 0 || n >= (int)size) { /* abgeschnitten */ }
```

**`get_git_status`** ignoriert seinen `git_path`-Parameter und arbeitet
implizit im CWD. Rückgabewerte sind mehrdeutig (`1` = clean *und* Fehler).

### 2. `_exit()` statt `exit()` im Kindprozess

Wenn `execvp` fehlschlägt, läuft das Kind mit dem geerbten Heap weiter.
`exit()` flusht dabei die vom Elternprozess geerbten stdio-Puffer → Doppelausgabe.
```cpp
if (execvp(args[0], args) == -1) {
    perror(args[0]);
    _exit(127);   // 127 = command not found, 126 = not executable
}
```

### 3. `popen` → `fork` + `execvp`

`popen("git status --porcelain")` startet erst `/bin/sh`, dann `git` —
ein Prozess umsonst. Spart ~0.5–1 ms im interaktiven Pfad.

### 4. Kleinkram

- `include/shell.hpp`: readline-Includes durch Leerzeile abtrennen, sonst
  sortiert clang-format sie vor `stdio.h` und der Build bricht (readline
  braucht `FILE`).
- Lokale `const char *status` in `main` verdeckt globale `int status` → umbenennen.
- Globale `int status` fehlt im Header — entweder `extern` rein oder `static` machen.
- `-Wall -Wextra` ins Makefile, aktuell fehlen sie ganz.
- Makefile nutzt keine `CXXFLAGS`-Variable → `make CXXFLAGS=...` wird ignoriert.
  Debug-Target wäre praktisch.
- unused-Parameter (`sig`, `anzahl_args`, `git_path`): Namen weglassen
  oder auskommentieren. `cols` in `startup.cpp:156` ist tot.

---

## Entscheidung: readline ersetzen

### Warum überhaupt

- **Lizenz:** readline ist GPL-3. Prüfen, was in `LICENSE` steht — bei MIT
  gibt es einen Konflikt.
- **macOS:** systemseitig nur libedit, Homebrew-Pfade im Makefile sind
  ausserdem hart auf Apple Silicon (`/opt/homebrew`) verdrahtet.
- **Ghost Text:** mit readline gegen `rl_redisplay_function` zu kämpfen
  lohnt nicht.

Das Argument "readline ist gross und muss nachinstalliert werden" trägt
auf Linux weniger — es ist überall da, weil bash dagegen linkt.

### Optionen

| Option | Aufwand | Lizenz | Ghost Text | Ctrl-R |
|---|---|---|---|---|
| readline behalten + `pkg-config` | 5 min | GPL-3 | umständlich | ✓ |
| libedit (`-ledit`) | ~30 min | BSD | nein | ✓ |
| **linenoise ins Repo** | **halber Tag** | **BSD** | **✓ (Hints)** | **✗** |
| replxx | ~1 Tag | BSD | ✓ | ✓ |
| komplett selbst | Wochenende+ | — | ✓ | selbst bauen |

**Empfehlung: linenoise.** Zwei Dateien direkt ins Repo → null externe
Abhängigkeiten, `git clone && make` funktioniert überall. Der Hints-Callback
ist genau das Ghost-Text-Feature.

```c
char *line = linenoise(prompt);
linenoiseHistoryAdd(line);
linenoiseSetHintsCallback(hints_cb);
```

Fehlt: Ctrl-R (inkrementelle Rückwärtssuche in der History). UTF-8 je nach
Fork — nicht blind den ersten Treffer nehmen, sondern einen aktiv gepflegten.

replxx ist die Alternative, wenn Ctrl-R und UTF-8 wichtiger sind als
Minimalismus.

### Reihenfolge

1. Erst eigene History fertig (`load_history` / `save_history` sind
   auskommentiert). Aktuell laufen `add_history` (readline) und
   `add_to_history` (eigene) parallel.
2. Dann Line-Editor tauschen.

Nicht gleichzeitig — sonst zwei bewegliche Teile auf einmal.

Bei der Gelegenheit: `MAX_HISTORY 100` als festes Array wird beim
Persistieren eng. Ringpuffer oder dynamisch wachsen — bewusst entscheiden.

---

## Was noch fehlt: eigener Line-Editor

Falls es doch mal komplett selbst werden soll, das ist der Umfang:

- termios raw mode, sauber wiederherstellen auch bei Signalen/Crash
- Cursor-Bewegung, Wortsprünge, Ctrl-A/E/W/U/K
- History-Navigation (Pfeiltasten) + Ctrl-R
- Tab-Completion
- Zeilenumbruch bei langen Eingaben, Terminal-Resize
- UTF-8: ein Zeichen ≠ ein Byte, Breite ≠ Länge
- Bracketed Paste (sonst führt eingefügter Text mit Newlines Kommandos aus)

Realistisch 1500+ Zeilen. Eigenes Projekt, kein Refactoring.

---

## Fürs README

Beide Zahlen nennen, mit Erklärung der Differenz:

```
Startup (nicht-interaktiv):  1.5 ms
Startup mit Git-Prompt:      4.0 ms   (Differenz = git status Subprozess)
```

Fairness-Hinweis dazu: bash/zsh lesen bei nicht-interaktivem stdin ihre
rc-Dateien gar nicht ein, und haben in diesem Vergleich keinen Git-Prompt.

Benchmark-Befehl:
```bash
echo exit > exit.txt
hyperfine -N --warmup 20 --input exit.txt ./shell /bin/dash /bin/bash /bin/zsh
```
