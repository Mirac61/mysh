#!/bin/bash

SHELL_PATH="./shell"

echo "=========================================="
echo "        mysh Benchmark Suite"
echo "=========================================="

# 1. STARTUP ZEIT
echo ""
echo "1. Startup Zeit:"
hyperfine --warmup 3 "$SHELL_PATH <<< 'exit'" --export-markdown startup.md
cat startup.md

# 2. CPU & RAM mit /usr/bin/time
echo ""
echo "2. CPU & RAM Verbrauch:"
/usr/bin/time -l $SHELL_PATH <<< 'exit' 2>&1 | grep -E "real|user|sys|maximum resident"

# 3. BEFEHLE TESTEN
echo ""
echo "3. Befehl Performance (ls, pwd, echo):"
hyperfine --warmup 3 \
  "$SHELL_PATH <<< 'ls'" \
  "$SHELL_PATH <<< 'pwd'" \
  "$SHELL_PATH <<< 'echo hello'" \
  --export-markdown commands.md
cat commands.md

# 4. LEAKS mit leaks (Mac built-in)
echo ""
echo "4. Memory Leaks:"
leaks --atExit -- $SHELL_PATH <<< 'exit'

echo ""
echo "=========================================="
echo "        Fertig!"
echo "=========================================="
