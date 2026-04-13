#!/bin/bash
SHELL_BIN="./shell"
PASS=0
FAIL=0

run_test() {
    local name="$1"
    local cmd="$2"
    local expected="$3"
    local result=$(printf "%s\nexit\n" "$cmd" | $SHELL_BIN 2>/dev/null | grep -v "❯\|███\|████\|██╔\|██║\|╚═\|╗\|║\|╝\|╔\|master\|mysh\|Bye\|^$\|  " | tail -1)
    if [ "$result" = "$expected" ]; then
        echo "✓ $name"
        PASS=$((PASS + 1))
    else
        echo "✗ $name (erwartet: '$expected', bekommen: '$result')"
        FAIL=$((FAIL + 1))
    fi
}

echo "=== Builtins ==="
run_test "echo einfach"          "echo Hallo"                          "Hallo "
run_test "echo mehrere Args"     "echo Hallo Welt"                     "Hallo Welt "
run_test "export + echo"         "export X=42; echo \$X"               "42 "
run_test "export überschreiben"  "export X=1; export X=2; echo \$X"    "2 "
run_test "cd home"               "cd; pwd"                             "$HOME"
run_test "cd tilde"              "cd ~; pwd"                           "$HOME"

echo ""
echo "=== Operatoren ==="
run_test "&& Erfolg"             "true && echo ok"                     "ok"
run_test "&& Fehler"             "false && echo ok"                    ""
run_test "|| Fehler"             "false || echo ok"                    "ok"
run_test "|| Erfolg"             "true || echo ok"                     ""
run_test "Semikolon"             "echo a; echo b"                      "b "

echo ""
echo "=== Pipes ==="
run_test "pipe einfach"          "echo Hallo | grep Hallo"             "Hallo"
run_test "pipe kein Match"       "echo Hallo | grep xyz"               ""

echo ""
echo "=== Expansion ==="
run_test "Tilde"                 "echo ~"                              "$HOME "
run_test "Tilde in Pfad"         "cd ~/; pwd"                          "$HOME"

echo ""
echo "=== Redirect ==="
run_test "Redirect >"            "echo hi > /tmp/mysh_test.txt; cat /tmp/mysh_test.txt"   "hi"
run_test "Redirect >>"           "echo a > /tmp/mysh_test.txt; echo b >> /tmp/mysh_test.txt; cat /tmp/mysh_test.txt" "b"
run_test "Redirect <"            "echo testinhalt > /tmp/mysh_test.txt; cat < /tmp/mysh_test.txt" "testinhalt"

echo ""
echo "Ergebnis: $PASS bestanden, $FAIL fehlgeschlagen"
rm -f /tmp/mysh_test.txt
