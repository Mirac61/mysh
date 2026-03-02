# mysh

Eine Unix-Shell in C++ — gebaut um zu verstehen, was wirklich passiert wenn man Enter drückt.

## Features

- **Powerline Prompt** — zeigt aktuelles Verzeichnis, Git Branch, Git Status (`✓`/`✗`) und Uhrzeit
- **Farbiges `ls`** — Dateien werden nach Typ eingefärbt (C++, JS/TS, HTML, CSS, JSON, Java, Python, ...)
  - funktioniert mit `-a`, `-l`, `-la` und Kombinationen
- **Pipes** — Befehle verketten mit `|`, mehrere Pipes hintereinander möglich
- **I/O Umleitung** — `>`, `>>` und `<`
- **Command History** — vorherige Befehle mit Pfeiltasten navigieren
- **Tab Autocomplete** — Dateien und Ordner über readline
- **`alias`** — eigene Abkürzungen definieren, werden in `~/.myshrc` gespeichert
- **`echo`** mit Variablen-Support (`$HOME`, `$PATH`, ...)
- **`history`** — zeigt alle Befehle der Session an
- **Konfigurierbarer Prompt** — Farben in `~/.myshrc` anpassen

## Konfiguration

In `~/.myshrc` kannst du Aliases und Prompt-Farben (0–255) definieren:
```bash
alias ll="ls -la"
alias gs="git status"

prompt_folder=17    # Blau
prompt_branch=172   # Orange
prompt_time=237     # Grau
```

Alle 256 Farben anzeigen:
```bash
for i in {0..255}; do echo -e "\033[48;5;${i}m $i \033[0m"; done
```

## Voraussetzungen

- C++17 oder neuer
- readline >= 8.0
- make

## Build

**macOS**
```bash
brew install readline
make
./shell
```

**Linux (Debian/Ubuntu)**
```bash
sudo apt install libreadline-dev
make
./shell
```

**Windows**
> Windows wird nicht nativ unterstützt. Empfehlung: [WSL2](https://learn.microsoft.com/de-de/windows/wsl/install) verwenden, dann wie Linux vorgehen.

## Benutzung
```bash
shell > ls
shell > ls -la
shell > ls | grep .cpp
shell > ls > output.txt
shell > cat < output.txt
shell > echo $HOME
shell > alias ll="ls -la"
shell > history
shell > cd ..
shell > exit
```

<details>
<summary>Projektstruktur</summary>

<pre>
├── main.cpp       # Hauptschleife und Built-in Befehle
├── shell.h        # Deklarationen und Farb-Definitionen
├── ls.cpp         # eigenes ls mit Dateityp-Färbung
├── parse.cpp      # Input-Parsing, Pipe-Splitting, Redirect-Erkennung
├── execute.cpp    # Prozessausführung, Pipes, I/O Umleitung
├── config.cpp     # ~/.myshrc laden und speichern
├── git.cpp        # Git Erkennung, Branch und Status für Prompt
└── Makefile
</pre>

</details>

<details>
<summary>Warum?</summary>

Die meisten Tutorials erklären die Theorie. Ich wollte eine Shell bauen und dabei wirklich verstehen, was zwischen Enter-drücken und Output-sehen passiert — `fork()`, `execvp()`, `dup2()`, `pipe()`, alles davon.

</details>
