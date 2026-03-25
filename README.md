# mysh
> Eine Unix-Shell in C++ — gebaut um zu verstehen, was wirklich passiert wenn man Enter drückt.

![Demo](mysh_aufnahme.gif)

---

## Features

### Prompt
- **Powerline-Style** — zeigt Verzeichnis, Git Branch, Git Status (`✓`/`✗`) und Uhrzeit
- **Vollständig konfigurierbar** — alle Farben über `~/.myshrc` anpassbar
- **Startup Animation** — Logo beim Start (instant, typewriter oder fade-in)

### Befehle
- **Farbiges `ls`** — Dateien nach Typ eingefärbt (C++, JS/TS, HTML, CSS, JSON, Java, Python, ...)
- **`echo`** — mit Variablen-Support (`$HOME`, `$PATH`, ...)
- **`export`** — Umgebungsvariablen setzen
- **`alias`** — Abkürzungen definieren, werden in `~/.myshrc` gespeichert
- **`history`** — alle Befehle der Session anzeigen
- **`config`** — öffnet `~/.myshrc` direkt in nvim
- **`sf [filter]`** — Datei fuzzy suchen und in `$EDITOR` öffnen
- **`sd [filter]`** — Ordner fuzzy suchen und direkt navigieren

### Shell-Funktionen
- Pipes `|`, Sequenz `;`, bedingte Ausführung `&&` / `||`
- I/O Umleitung `>`, `>>`, `<`
- Tilde-Expansion, Command History, Tab Autocomplete
- Quoted Arguments, Ctrl+C

## Aktueller Stand (Variablen-Expansion)
- `$VAR` wird jetzt in allen Pfaden expandiert: normale Kommandos, Pipes `|`, Redirects `> >> <`, Sequenzen `;`, Bedingungen `&& ||`, Aliases.
- Tilde-Expansion (`~`) läuft ebenfalls überall nach dem Parsen.
- Die Expansion arbeitet pro Argument mit statischem Puffer, sodass keine ungültigen Zeiger entstehen.


---

## Performance

Gemessen mit [hyperfine](https://github.com/sharkdp/hyperfine):

| Command | Mean [ms] |
|:---|---:|
| Startup | 9.5 ± 0.4 |
| ls | 16.5 ± 1.2 |
| pwd | 18.1 ± 0.6 |
| echo | 16.8 ± 1.1 |

> Schneller als fish (~14ms Startup) · 0 Memory Leaks

---

## Voraussetzungen

- C++17 oder neuer
- readline ≥ 8.0
- make
- fzf (für `sf` und `sd`)

---

## Installation

**macOS**
```bash
brew install readline fzf
git clone https://github.com/Mirac61/mysh
cd mysh && make && ./shell
```

**Linux (Debian/Ubuntu)**
```bash
sudo apt install libreadline-dev fzf
git clone https://github.com/Mirac61/mysh
cd mysh && make && ./shell
```

---

## Konfiguration

```bash
cp example.myshrc ~/.myshrc
```

<details>
<summary>Alle Optionen</summary>
<br>

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
</details>

---

## Tests

```bash
./test.sh
```

---

<details>
<summary>Benutzung</summary>
<br>

```bash
# Navigation
cd ~/Projekte
ls -la

# Fuzzy Search
sf main       # Datei suchen → in $EDITOR öffnen
sd Proj       # Ordner suchen → direkt navigieren

# Pipes und Umleitung
ls | grep .cpp
ls > output.txt

# Bedingte Ausführung
make && ./shell
cd /nicht/vorhanden || echo "Verzeichnis nicht gefunden"

# Aliases und Variablen
alias ll="ls -la"
export NAME=Mirac
echo $NAME
```
</details>

---

<details>
<summary>Projektstruktur</summary>
<br>

```
mysh/
├── main.cpp       # Hauptschleife und Befehlsausführung
├── builtins.cpp   # Built-in Befehle (cd, export, alias, sf, sd, ...)
├── shell.h        # Deklarationen und Farb-Definitionen
├── ls.cpp         # eigenes ls mit Dateityp-Färbung
├── parse.cpp      # Input-Parsing, Pipes, Redirect, Quote-Handling
├── execute.cpp    # Prozessausführung, Pipes, I/O Umleitung
├── config.cpp     # ~/.myshrc laden und speichern
├── git.cpp        # Git-Erkennung für Prompt
├── startup.cpp    # Startup-Animation
├── test.sh        # automatisierte Tests
├── example.myshrc # Beispiel-Konfiguration
└── Makefile
```
</details>

---

<details>
<summary>Warum?</summary>
<br>
Die meisten Tutorials erklären die Theorie. Ich wollte eine Shell bauen und dabei wirklich verstehen, was zwischen Enter-drücken und Output-sehen passiert — <code>fork()</code>, <code>execvp()</code>, <code>dup2()</code>, <code>pipe()</code>, alles davon.
</details>

## Startup Animationen
Die Animationen wurden mit Unterstützung von Claude (Anthropic) entwickelt.
