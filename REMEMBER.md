# mysh — Notizen (Stand 14.10.2026)

## Kurzer Rückblick: seit dem 08.09. erledigt

Nur als Kontext, nichts zu tun:

- `get_git_branch`: prüft jetzt auf `ref: refs/heads/`-Präfix, zeigt bei
  detached HEAD einen 7-Zeichen-Hash; `fgets`-Rückgabe wird geprüft.
- Startup-Performance (4.1 ms → 1.5 ms): `bool interactive =
  isatty(STDIN_FILENO)` in `main.cpp`, Prompt/Git/Animation nur interaktiv,
  nicht-interaktiv `getline` statt `readline`.
- `include/shell.hpp`: readline-Includes sind durch Leerzeile abgetrennt.
- Namenskonflikt `status`: lokale Variable in `main` heißt jetzt
  `status_icon`.
- Makefile: nutzt `CXXFLAGS`-Variable, `make CXXFLAGS=...` funktioniert.
- `compile_commands.json` steht in `.gitignore`.
- `sigchld_handler(int)`: unused-Parameter-Name ist weg.
- Doppelte History: `add_to_history` ist auskommentiert, es läuft nur noch
  readlines `add_history` (weiterhin rein im Speicher, nicht persistiert).

---

## Offene Punkte

### 1. `get_git_status`

Arbeitet implizit im CWD, `git_path` wird ignoriert (Parameter ist jetzt
unbenannt). Rückgabewerte sind mehrdeutig (`1` = clean *und* Fehler).

---

## Entscheidung: readline ersetzen

**Noch nicht umgesetzt** — Makefile linkt `-lreadline`, `main.cpp` nutzt
`readline()` / `add_history()` / `rl_bind_key`.

### Warum überhaupt

- **Lizenz:** readline ist GPL-3, `LICENSE` ist MIT → der Konflikt ist real.
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

1. Erst eigene History fertig: `load_history` / `save_history` sind in
   `main.cpp:39, 126` auskommentiert. Aktuell gibt es nur readlines
   `add_history` — die History überlebt keinen Neustart.
2. Dann Line-Editor tauschen.

Nicht gleichzeitig — sonst zwei bewegliche Teile auf einmal.

Bei der Gelegenheit: `MAX_HISTORY 100` als festes Array (`shell.hpp:18`)
wird beim Persistieren eng. Ringpuffer oder dynamisch wachsen — bewusst
entscheiden.

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

> **Achtung:** Die Zahlen sind vom 08.09.2026. Seitdem wurde der Code
> umstrukturiert (ls/parse/execute/config/process.cpp sind neue Dateien) —
> vor dem Übernehmen ins README neu messen.

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

Valgrind-Lauf (Linux) — Befehl passt weiterhin, `src/*.cpp` deckt die neuen
Dateien automatisch mit ab:
```bash
g++ -g -O0 -Wall -Wextra src/*.cpp -o shell -Iinclude -lreadline
valgrind --leak-check=full --track-origins=yes ./shell < cmds.txt
```
Ergebnis vom 08.09.: `0 errors`, keine definitely/indirectly/possibly lost
Blöcke. Die ~241 KB "still reachable" sind readline-intern, nicht unser
Problem. **Nach den Änderungen neu verifizieren.**
