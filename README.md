# mysh

> Eine Unix-Shell in C++ — gebaut um zu verstehen, was wirklich passiert wenn man Enter drückt.

---

## Features

### Prompt
- **Powerline-Style** — zeigt Verzeichnis, Git Branch, Git Status (`✓`/`✗`) und Uhrzeit
- **Vollständig konfigurierbar** — alle Farben über `~/.myshrc` anpassbar
- **Startup Animation** — Logo beim Start (instant, typewriter oder fade-in)

### Befehle
- **Farbiges `ls`** — Dateien nach Typ eingefärbt (C++, JS/TS, HTML, CSS, JSON, Java, Python, ...)
  - unterstützt `-a`, `-l`, `-la` und Kombinationen
- **`echo`** — mit Variablen-Support (`$HOME`, `$PATH`, ...)
- **`alias`** — Abkürzungen definieren, werden automatisch in `~/.myshrc` gespeichert
- **`history`** — alle Befehle der Session anzeigen
- **`config`** — öffnet `~/.myshrc` direkt in nano

### Shell-Funktionen
- **Pipes** — Befehle verketten mit `|`, mehrere hintereinander möglich
- **I/O Umleitung** — `>`, `>>` und `<`
- **Command History** — vorherige Befehle mit Pfeiltasten navigieren
- **Tab Autocomplete** — Dateien und Ordner via readline
- **Quoted Arguments** — `git commit -m "Nachricht mit Leerzeichen"` funktioniert korrekt

---

## Voraussetzungen

- C++17 oder neuer
- readline ≥ 8.0
- make

---

## Installation

**macOS**
```bash
brew install readline
git clone https://github.com/dein-user/mysh
cd mysh
make
./shell
```

**Linux (Debian/Ubuntu)**
```bash
sudo apt install libreadline-dev
git clone https://github.com/dein-user/mysh
cd mysh
make
./shell
```

**Windows**
> Windows wird nicht nativ unterstützt. Empfehlung: [WSL2](https://learn.microsoft.com/de-de/windows/wsl/install) verwenden, dann wie Linux vorgehen.

---

## Konfiguration

Kopiere die Beispiel-Config in dein Home-Verzeichnis:
```bash
cp example.myshrc ~/.myshrc
```
Danach mit `config` direkt in der Shell bearbeiten.

### Alle Optionen

| Option | Werte | Beschreibung |
|--------|-------|--------------|
| `prompt_folder` | 0–255 | Farbe des Ordnernamens |
| `prompt_time` | 0–255 | Farbe der Uhrzeit |
| `prompt_git_clean` | 0–255 | Farbe bei sauberem Repo |
| `prompt_git_dirty` | 0–255 | Farbe bei uncommitted Änderungen |
| `startup` | 0–3 | 0 = kein Logo, 1 = sofort, 2 = typewriter, 3 = fade-in |
| `alias` | `name="befehl"` | Eigene Shortcuts |

Alle 256 Farben anzeigen:
```bash
for i in {0..255}; do echo -e "\033[48;5;${i}m $i \033[0m"; done
```

---

## Benutzung
```bash
ls -la
ls | grep .cpp
ls > output.txt
git commit -m "Nachricht mit Leerzeichen"
alias ll="ls -la"
history
config
```

---

## Projektstruktur
```
mysh/
├── main.cpp       # Hauptschleife und Built-in Befehle
├── shell.h        # Deklarationen und Farb-Definitionen
├── ls.cpp         # eigenes ls mit Dateityp-Färbung
├── parse.cpp      # Input-Parsing, Pipes, Redirect, Quote-Handling
├── execute.cpp    # Prozessausführung, Pipes, I/O Umleitung
├── config.cpp     # ~/.myshrc laden und speichern
├── git.cpp        # Git-Erkennung, Branch und Status für Prompt
├── startup.cpp    # Startup-Animation
├── example.myshrc # Beispiel-Konfiguration
└── Makefile
```

---

<details>
<summary>Warum?</summary>
<br>
Die meisten Tutorials erklären die Theorie. Ich wollte eine Shell bauen und dabei wirklich verstehen, was zwischen Enter-drücken und Output-sehen passiert — <code>fork()</code>, <code>execvp()</code>, <code>dup2()</code>, <code>pipe()</code>, alles davon.
</details>
