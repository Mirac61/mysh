# mysh

Eine Unix Shell in C++, gebaut um zu verstehen, was passiert, wenn du Enter drückst.

## Features

- **Prompt mit Infos**
  - aktuelles Verzeichnis
  - Git Branch Status, wenn im Repo bist
- **Farbiges `ls`**
  - Dateien werden nach Typ eingefärbt, z.B. C Cpp JS TS HTML CSS JSON Java Python
  - funktioniert bei `ls`, `ls -a`, `ls -l`, `ls -la` und Kombinationen
- **Pipes**
  - Befehle mit `|` verketten, mehrere Pipes hintereinander möglich
- **I O Umleitung**
  - `>`, `>>`, `<`
- **Command History**
  - vorherige Befehle mit Pfeiltasten
- **Tab Autocomplete**
  - über readline
- **cd**
  - wechselt das Verzeichnis, Prompt aktualisiert sich direkt

## Voraussetzungen

- C++17 oder neuer
- readline 8 oder neuer
- make

## Build

### macOS
```bash
brew install readline
make
./shell

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

<pre>
├── main.cpp       # Hauptschleife
├── shell.h        # Deklarationen und Farb-Definitionen
├── ls.cpp         # eigenes ls mit Dateityp-Färbung
├── parse.cpp      # Input-Parsing, Pipe-Splitting, Redirect-Erkennung
├── execute.cpp    # Prozessausführung, Pipes, I/O Umleitung
├── git.cpp        # Git Erkennung und Branch Status für Prompt
└── Makefile
</pre>

</details>
<details>
<summary>Warum?</summary>
Die meisten Tutorials erklären die Theorie. Ich wollte eine Shell bauen und dabei wirklich verstehen, was zwischen Enter-drücken und Output-sehen passiert — fork(), execvp(), dup2(), pipe(), alles davon.
</details>
