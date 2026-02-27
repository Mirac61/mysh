# mysh

Eine Unix-Shell in C++ — gebaut um zu verstehen, was wirklich passiert wenn man Enter drückt.

## Features

- **Farbiges `ls`** — Dateien werden nach Typ eingefärbt (C++, JS/TS, HTML, CSS, JSON, Java, Python, ...)
- **Pipes** — Befehle verketten mit `|`, mehrere Pipes hintereinander möglich
- **I/O Umleitung** — `>`, `>>` und `<`
- **Command History** — vorherige Befehle mit Pfeiltasten navigieren
- **Tab Autocomplete** — über readline
- **`cd`** mit aktuellem Verzeichnis im Prompt

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

> Windows wird nicht nativ unterstützt. Empfehlung: [WSL2](https://learn.microsoft.com/de-de/windows/wsl/install) (Windows Subsystem for Linux) verwenden, dann wie Linux vorgehen.

## Benutzung
```bash
shell > ls
shell > ls | grep .cpp
shell > ls > output.txt
shell > cat < output.txt
shell > echo "hallo" >> output.txt
shell > cd ..
shell > exit
```

<details>
<summary>Projektstruktur</summary>
```
├── main.cpp       # Hauptschleife
├── shell.h        # Deklarationen und Farb-Definitionen
├── ls.cpp         # eigenes ls mit Dateityp-Färbung
├── parse.cpp      # Input-Parsing, Pipe-Splitting, Redirect-Erkennung
├── execute.cpp    # Prozessausführung, Pipes, I/O Umleitung
└── Makefile
```
</details>
<details>
<summary>Warum?</summary>
Die meisten Tutorials erklären die Theorie. Ich wollte eine Shell bauen und dabei wirklich verstehen, was zwischen Enter-drücken und Output-sehen passiert — fork(), execvp(), dup2(), pipe(), alles davon.
</details>
