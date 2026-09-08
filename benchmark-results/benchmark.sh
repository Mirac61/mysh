#!/usr/bin/env bash
set -euo pipefail

SHELL_PATH="./mysh"
[[ -x "$SHELL_PATH" ]] || { echo "Binary fehlt: $SHELL_PATH"; exit 1; }
command -v hyperfine >/dev/null || { echo "hyperfine fehlt"; exit 1; }

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
echo exit > "$TMP/exit.txt"
printf 'pwd\nexit\n'       > "$TMP/pwd.txt"
printf 'echo hello\nexit\n' > "$TMP/echo.txt"
printf 'ls\nexit\n'        > "$TMP/ls.txt"

echo "=== 1. Startup vs. Referenz-Shells ==="
hyperfine -N --warmup 20 --input "$TMP/exit.txt" \
  "$SHELL_PATH" /bin/dash /bin/bash /bin/zsh \
  --export-markdown "$TMP/startup.md" 2>/dev/null || \
hyperfine -N --warmup 20 --input "$TMP/exit.txt" \
  "$SHELL_PATH" /bin/bash --export-markdown "$TMP/startup.md"

echo -e "\n=== 2. Builtin vs. fork+exec ==="
hyperfine -N --warmup 20 \
  -n "pwd (builtin)"  --input "$TMP/pwd.txt"  "$SHELL_PATH" \
  -n "echo (builtin)" --input "$TMP/echo.txt" "$SHELL_PATH" \
  -n "ls (extern)"    --input "$TMP/ls.txt"   "$SHELL_PATH"

echo -e "\n=== 3. Ressourcen ==="
if [[ "$(uname)" == "Darwin" ]]; then
  /usr/bin/time -l "$SHELL_PATH" < "$TMP/exit.txt" 2>&1 \
    | grep -E "real|maximum resident"
  echo "(RSS in Bytes)"
else
  /usr/bin/time -v "$SHELL_PATH" < "$TMP/exit.txt" 2>&1 \
    | grep -E "Elapsed|Maximum resident"
  echo "(RSS in KB)"
fi

echo -e "\n=== 4. Speicher ==="
if [[ "$(uname)" == "Darwin" ]]; then
  leaks --atExit -- "$SHELL_PATH" < "$TMP/exit.txt" | tail -5
else
  valgrind --leak-check=full --error-exitcode=1 \
    "$SHELL_PATH" < "$TMP/exit.txt" 2>&1 | tail -15
fi
