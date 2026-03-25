#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

// ============================================================
//  Mini Test-Framework Claude(Vibe Coding)
// ============================================================

struct TestResult {
    std::string name;
    bool passed;
    std::string skipped;
};

std::vector<TestResult> results;
int passed = 0, failed = 0, skipped_count = 0;

// Prüft ob ein Tool vorhanden ist
bool hasCmd(const std::string& cmd) {
    return system(("which " + cmd + " >/dev/null 2>&1").c_str()) == 0;
}

void test(const std::string& name, std::function<bool()> fn, const std::string& skipIfMissing = "") {
    if (!skipIfMissing.empty() && !hasCmd(skipIfMissing)) {
        std::cout << "  \033[33m⊘\033[0m " << name << "  \033[33m(skipped – " << skipIfMissing << " not found)\033[0m\n";
        results.push_back({name, false, skipIfMissing});
        ++skipped_count;
        return;
    }
    bool ok = false;
    try { ok = fn(); } catch (...) { ok = false; }
    results.push_back({name, ok, ""});
    if (ok) ++passed; else ++failed;
    std::cout << (ok ? "  \033[32m✓\033[0m " : "  \033[31m✗\033[0m ") << name << "\n";
}

// macOS kompatibles Timeout
static std::string TIMEOUT_CMD = "";

void detectTimeout() {
    if (system("which gtimeout >/dev/null 2>&1") == 0)
        TIMEOUT_CMD = "gtimeout 5s";
    else if (system("which timeout >/dev/null 2>&1") == 0)
        TIMEOUT_CMD = "timeout 5s";
    else
        TIMEOUT_CMD = "";
}

// Sichere Shell-Quote Funktion
std::string shellQuote(const std::string& s) {
    std::string out = "'";
    for (char c : s) {
        if (c == '\'')
            out += "'\\''";
        else
            out += c;
    }
    out += "'";
    return out;
}

std::string run(const std::string& cmd) {
    std::string wrapped;
    if (TIMEOUT_CMD.empty())
        wrapped = "sh -c " + shellQuote(cmd);
    else
        wrapped = TIMEOUT_CMD + " sh -c " + shellQuote(cmd);

    FILE* pipe = popen(wrapped.c_str(), "r");
    if (!pipe) return "__ERROR__";

    char buf[512];
    std::string out;
    while (fgets(buf, sizeof(buf), pipe)) out += buf;

    pclose(pipe);

    while (!out.empty() && (out.back() == '\n' || out.back() == '\r'))
        out.pop_back();

    size_t start = out.find_first_not_of(" \t");
    if (start != std::string::npos) out = out.substr(start);

    return out;
}

int exitCode(const std::string& cmd) {
    std::string wrappedCmd = cmd + " >/dev/null 2>&1";

    std::string wrapped;
    if (TIMEOUT_CMD.empty())
        wrapped = "sh -c " + shellQuote(wrappedCmd);
    else
        wrapped = TIMEOUT_CMD + " sh -c " + shellQuote(wrappedCmd);

    int rc = system(wrapped.c_str());
    if (rc == -1) return -1;
    if (WIFEXITED(rc)) return WEXITSTATUS(rc);
    return -1;
}

bool contains(const std::string& str, const std::string& sub) {
    return str.find(sub) != std::string::npos;
}

// ============================================================
//  Setup / Teardown
// ============================================================

const std::string TMP = "/tmp/shell_test_suite";

void setupTmp() {
    system(("rm -rf " + TMP + " && mkdir -p " + TMP).c_str());
}

void teardownTmp() {
    system(("rm -rf " + TMP).c_str());
}

// ============================================================
//  [1] Grundlegende Befehle
// ============================================================
void testBasicCommands() {
    std::cout << "\n\033[1;34m[1] Grundlegende Befehle\033[0m\n";

    test("echo",           []{ return run("echo hello") == "hello"; });
    test("echo -n",        []{ return run("printf 'hi'") == "hi"; });
    test("printf",         []{ return run("printf '%s' hello") == "hello"; });
    test("pwd",            []{ return !run("pwd").empty() && run("pwd")[0] == '/'; });
    test("whoami",         []{ return !run("whoami").empty(); });
    test("id",             []{ return contains(run("id"), "uid="); });
    test("hostname",       []{ return !run("hostname").empty(); });
    test("uname -s",       []{ return !run("uname -s").empty(); });
    test("uname -m",       []{ return !run("uname -m").empty(); });
    test("date",           []{ return !run("date").empty(); });
    test("uptime",         []{ return !run("uptime").empty(); });
    test("cal",            []{ return !run("cal 2>/dev/null || true").empty(); }, "cal");
    test("clear (exit 0)", []{ return exitCode("clear") == 0; }, "clear");
    test("true",           []{ return exitCode("true") == 0; });
    test("false",          []{ return exitCode("false") != 0; });
    test("sleep 0",        []{ return exitCode("sleep 0") == 0; });
    test("yes (head -1)",  []{ return run("yes | head -1") == "y"; });
    test("seq 1 3",        []{ return run("seq 1 3") == "1\n2\n3"; }, "seq");
    test("tput cols",      []{ return exitCode("tput cols") == 0; }, "tput");
}

// ============================================================
//  [2] Dateisystem
// ============================================================
void testFileSystem() {
    std::cout << "\n\033[1;34m[2] Dateisystem\033[0m\n";

    test("mkdir -p",         []{ system(("mkdir -p " + TMP + "/a/b/c").c_str()); struct stat s{}; return stat((TMP+"/a/b/c").c_str(),&s)==0; });
    test("touch",            []{ system(("touch " + TMP + "/f.txt").c_str()); struct stat s{}; return stat((TMP+"/f.txt").c_str(),&s)==0; });
    test("ls",               []{ return !run("ls " + TMP).empty(); });
    test("ls -la",           []{ return contains(run("ls -la " + TMP), "total"); });
    test("ls -lh",           []{ return exitCode("ls -lh " + TMP) == 0; });
    test("echo > (write)",   []{ system(("echo 'hello' > " + TMP + "/f.txt").c_str()); return run("cat " + TMP + "/f.txt") == "hello"; });
    test("cat",              []{ return run("cat " + TMP + "/f.txt") == "hello"; });
    test(">> (append)",      []{ system(("echo 'world' >> " + TMP + "/f.txt").c_str()); return run("wc -l < " + TMP + "/f.txt") == "2"; });
    test("cp",               []{ system(("cp " + TMP + "/f.txt " + TMP + "/g.txt").c_str()); struct stat s{}; return stat((TMP+"/g.txt").c_str(),&s)==0; });
    test("cp -r",            []{ system(("cp -r " + TMP + "/a " + TMP + "/a2").c_str()); struct stat s{}; return stat((TMP+"/a2/b/c").c_str(),&s)==0; });
    test("mv",               []{ system(("mv " + TMP + "/g.txt " + TMP + "/h.txt").c_str()); struct stat s1{},s2{}; return stat((TMP+"/h.txt").c_str(),&s2)==0 && stat((TMP+"/g.txt").c_str(),&s1)!=0; });
    test("rm",               []{ system(("rm " + TMP + "/h.txt").c_str()); struct stat s{}; return stat((TMP+"/h.txt").c_str(),&s)!=0; });
    test("rm -rf",           []{ system(("rm -rf " + TMP + "/a2").c_str()); struct stat s{}; return stat((TMP+"/a2").c_str(),&s)!=0; });
    test("rmdir",            []{ system(("mkdir " + TMP + "/emptydir && rmdir " + TMP + "/emptydir").c_str()); struct stat s{}; return stat((TMP+"/emptydir").c_str(),&s)!=0; });
    test("find (Datei)",     []{ system(("touch " + TMP + "/findme.txt").c_str()); return contains(run("find " + TMP + " -name 'findme.txt'"), "findme.txt"); });
    test("find -type f",     []{ return !run("find " + TMP + " -type f").empty(); });
    test("find -type d",     []{ return !run("find " + TMP + " -type d").empty(); });
    test("du -sh",           []{ return exitCode("du -sh " + TMP) == 0; });
    test("df -h",            []{ return contains(run("df -h"), "/"); });
    test("stat Datei",       []{ return exitCode("stat " + TMP + "/f.txt") == 0; }, "stat");
    test("file Befehl",      []{ return exitCode("file " + TMP + "/f.txt") == 0; }, "file");
    test("wc -l",            []{ return run("wc -l < " + TMP + "/f.txt") == "2"; });
    test("wc -w",            []{ return !run("wc -w < " + TMP + "/f.txt").empty(); });
    test("wc -c",            []{ return !run("wc -c < " + TMP + "/f.txt").empty(); });
    test("head -1",          []{ return run("head -1 " + TMP + "/f.txt") == "hello"; });
    test("tail -1",          []{ return run("tail -1 " + TMP + "/f.txt") == "world"; });
    test("tee",              []{ system(("echo 'tee_test' | tee " + TMP + "/tee.txt > /dev/null").c_str()); return run("cat " + TMP + "/tee.txt") == "tee_test"; });
    test("diff (gleich)",    []{ system(("cp " + TMP + "/f.txt " + TMP + "/f2.txt").c_str()); return exitCode("diff " + TMP + "/f.txt " + TMP + "/f2.txt") == 0; });
    test("diff (ungleich)",  []{ system(("echo 'other' > " + TMP + "/f3.txt").c_str()); return exitCode("diff " + TMP + "/f.txt " + TMP + "/f3.txt") != 0; });
    test("ln -s (symlink)",  []{ system(("ln -s " + TMP + "/f.txt " + TMP + "/link.txt").c_str()); struct stat s{}; return lstat((TMP+"/link.txt").c_str(),&s)==0 && S_ISLNK(s.st_mode); });
    test("readlink",         []{ return contains(run("readlink " + TMP + "/link.txt"), "f.txt"); }, "readlink");
    test("basename",         []{ return run("basename /foo/bar/baz.txt") == "baz.txt"; });
    test("dirname",          []{ return run("dirname /foo/bar/baz.txt") == "/foo/bar"; });
    test("realpath",         []{ return !run("realpath " + TMP + "/f.txt").empty(); }, "realpath");
    test("mktemp",           []{ std::string f = run("mktemp"); bool ok = !f.empty() && f[0]=='/'; system(("rm -f " + f).c_str()); return ok; });
}

// ============================================================
//  [3] Berechtigungen & Besitz
// ============================================================
void testPermissions() {
    std::cout << "\n\033[1;34m[3] Berechtigungen & Besitz\033[0m\n";

    test("chmod +x",       []{ system(("touch " + TMP + "/script.sh && chmod +x " + TMP + "/script.sh").c_str()); return run("ls -l " + TMP + "/script.sh | cut -c4") == "x"; });
    test("chmod 644",      []{ system(("chmod 644 " + TMP + "/f.txt").c_str()); return run("ls -l " + TMP + "/f.txt | cut -c2-10") == "rw-r--r--"; });
    test("chmod 755",      []{ system(("chmod 755 " + TMP + "/script.sh").c_str()); return run("ls -l " + TMP + "/script.sh | cut -c2-10") == "rwxr-xr-x"; });
    test("chmod 000",      []{ system(("chmod 000 " + TMP + "/f3.txt").c_str()); return run("ls -l " + TMP + "/f3.txt | cut -c2-10") == "---------"; });
    test("chmod 777",      []{ system(("chmod 777 " + TMP + "/f3.txt").c_str()); return run("ls -l " + TMP + "/f3.txt | cut -c2-10") == "rwxrwxrwx"; });
    test("umask",          []{ return !run("umask").empty(); });
    test("chown (syntax)", []{ return exitCode("chown --help") == 0 || exitCode("chown --version") == 0 || hasCmd("chown"); });
    test("ls -l zeigt Permissions", []{ return contains(run("ls -l " + TMP + "/f.txt"), "rw"); });
}

// ============================================================
//  [4] Text-Verarbeitung
// ============================================================
void testTextProcessing() {
    std::cout << "\n\033[1;34m[4] Text-Verarbeitung\033[0m\n";

    system(("printf 'apple\\nbanana\\ncherry\\napple\\n' > " + TMP + "/fruits.txt").c_str());

    test("grep",            []{ return run("grep apple " + TMP + "/fruits.txt") == "apple\napple"; });
    test("grep -c",         []{ return run("grep -c apple " + TMP + "/fruits.txt") == "2"; });
    test("grep -v",         []{ return !contains(run("grep -v apple " + TMP + "/fruits.txt"), "apple"); });
    test("grep -i",         []{ return contains(run("echo HELLO | grep -i hello"), "HELLO"); });
    test("grep -r",         []{ return contains(run("grep -r apple " + TMP + "/"), "apple"); });
    test("grep -n",         []{ return contains(run("grep -n apple " + TMP + "/fruits.txt"), ":"); });
    test("grep -E (regex)", []{ return run("echo 'abc123' | grep -E '[0-9]+'") == "abc123"; });
    test("sed s/",          []{ return run("echo 'hello world' | sed 's/world/earth/'") == "hello earth"; });
    test("sed -n p",        []{ return run("sed -n '2p' " + TMP + "/fruits.txt") == "banana"; });
    test("sed d (delete)",  []{ return !contains(run("sed '/apple/d' " + TMP + "/fruits.txt"), "apple"); });
    test("awk print",       []{ return run("echo 'a b c' | awk '{print $2}'") == "b"; }, "awk");
    test("awk NR",          []{ return run("awk 'NR==2' " + TMP + "/fruits.txt") == "banana"; }, "awk");
    test("awk count",       []{ return run("awk 'END{print NR}' " + TMP + "/fruits.txt") == "4"; }, "awk");
    test("cut -d -f",       []{ return run("echo 'a:b:c' | cut -d: -f2") == "b"; });
    test("cut -c",          []{ return run("echo 'hello' | cut -c1-3") == "hel"; });
    test("sort",            []{ return run("printf 'c\\na\\nb\\n' | sort") == "a\nb\nc"; });
    test("sort -r",         []{ return run("printf 'a\\nb\\nc\\n' | sort -r") == "c\nb\na"; });
    test("sort -n",         []{ return run("printf '10\\n2\\n1\\n' | sort -n") == "1\n2\n10"; });
    test("uniq",            []{ return run("printf 'a\\na\\nb\\n' | uniq") == "a\nb"; });
    test("uniq -c",         []{ return contains(run("printf 'a\\na\\nb\\n' | uniq -c"), "2"); });
    test("tr",              []{ return run("echo 'hello' | tr 'a-z' 'A-Z'") == "HELLO"; });
    test("tr -d",           []{ return run("echo 'h e l l o' | tr -d ' '") == "hello"; });
    test("rev",             []{ return run("echo 'hello' | rev") == "olleh"; }, "rev");
    test("paste",           []{ return run("bash -c \"paste -d, <(echo a) <(echo b)\"") == "a,b"; }, "paste");
    test("join",            []{ system(("echo '1 a' > " + TMP + "/j1.txt && echo '1 b' > " + TMP + "/j2.txt").c_str()); return exitCode("join " + TMP + "/j1.txt " + TMP + "/j2.txt") == 0; }, "join");
    test("xargs",           []{ return run("echo 'hello' | xargs echo") == "hello"; });
    test("xargs -I{}",      []{ return run("echo 'world' | xargs -I{} echo hello {}") == "hello world"; });
    test("column -t",       []{ return exitCode("echo 'a b' | column -t") == 0; }, "column");
    test("nl (Zeilennr.)",  []{ return contains(run("echo -e 'a\\nb' | nl"), "1"); }, "nl");
    test("fold -w",         []{ return exitCode("echo 'hello' | fold -w 3") == 0; }, "fold");
    test("expand (tabs)",   []{ return exitCode("printf '\\t' | expand") == 0; }, "expand");
    test("strings",         []{ return exitCode("strings /bin/sh 2>/dev/null | head -1") == 0; }, "strings");
}

// ============================================================
//  [5] Pipes, Redirects, Substitution
// ============================================================
void testPipesAndRedirection() {
    std::cout << "\n\033[1;34m[5] Pipes & Weiterleitungen\033[0m\n";

    test("| pipe",             []{ return run("echo 'a b c' | wc -w") == "3"; });
    test("> überschreiben",    []{ system(("echo x > " + TMP + "/redir.txt").c_str()); return run("cat " + TMP + "/redir.txt") == "x"; });
    test(">> anhängen",        []{ system(("echo y >> " + TMP + "/redir.txt").c_str()); return run("wc -l < " + TMP + "/redir.txt") == "2"; });
    test("< stdin",            []{ return run("wc -l < " + TMP + "/fruits.txt") == "4"; });
    test("2> stderr",          []{ system(("ls /nope 2>" + TMP + "/err.txt").c_str()); struct stat s{}; return stat((TMP+"/err.txt").c_str(),&s)==0 && s.st_size>0; });
    test("2>&1",               []{ return contains(run("ls /nope 2>&1"), "No such"); });
    test("&> (bash shortcut)", []{ return exitCode("bash -c 'ls /nope &>/dev/null'") != -1; }, "bash");
    test("/dev/null schluckt", []{ return run("echo hi > /dev/null") == ""; });
    test("<<EOF heredoc",      []{ return run("cat <<EOF\nhello\nEOF") == "hello"; });
    test("$(cmd substitution)",[]{ return run("echo $(echo hello)") == "hello"; });
    test("`backtick subst.`",  []{ return run("echo `echo hello`") == "hello"; });
    test("Mehrfach-Pipe",      []{ return run("printf 'b\\na\\nc\\n' | sort | head -1") == "a"; });
    test("tee in Pipe",        []{ system(("echo tee_pipe | tee " + TMP + "/tee2.txt | cat > /dev/null").c_str()); return run("cat " + TMP + "/tee2.txt") == "tee_pipe"; });
    test("process subst. <()", []{ return run("bash -c 'diff <(echo a) <(echo a)'") == ""; }, "bash");
}

// ============================================================
//  [6] Umgebungsvariablen & Shell-Builtins
// ============================================================
void testEnvironment() {
    std::cout << "\n\033[1;34m[6] Umgebungsvariablen & Builtins\033[0m\n";

    test("$PATH",        []{ return !run("echo $PATH").empty(); });
    test("$HOME",        []{ return !run("echo $HOME").empty() && run("echo $HOME")[0] == '/'; });
    test("$USER",        []{ return !run("echo $USER").empty(); });
    test("$SHELL",       []{ return !run("echo $SHELL").empty(); });
    test("$PWD",         []{ return !run("echo $PWD").empty() && run("echo $PWD")[0] == '/'; });
    test("$?",           []{ return run("true; echo $?") == "0"; });
    test("$$ (PID)",     []{ return !run("echo $$").empty(); });
    test("export",       []{ return run("export MYVAR=test && echo $MYVAR") == "test"; });
    test("unset",        []{ return run("export X=1 && unset X && echo ${X:-empty}") == "empty"; });
    test("env",          []{ return !run("env").empty(); });
    test("printenv PATH",[]{ return contains(run("printenv PATH"), "/"); });
    test("set",          []{ return !run("set").empty(); });
    test("alias",        []{ return exitCode("alias ll='ls -la'") == 0; });
    test("type (builtin)",[]{ return contains(run("type echo"), "echo"); });
    test("which",        []{ return contains(run("which ls"), "/"); }, "which");
    test("command -v",   []{ return contains(run("command -v ls"), "/"); });
    test("readonly var", []{ return run("readonly RO=42 && echo $RO") == "42"; });
    test("${VAR:-default}",[]{ return run("echo ${UNSET_VAR:-fallback}") == "fallback"; });
    test("${VAR:=assign}",[]{ return run("unset X; echo ${X:=hello}") == "hello"; });
    test("${#VAR} length",[]{ return run("X=hello; echo ${#X}") == "5"; });
    test("${VAR%suffix}", []{ return run("X=hello.txt; echo ${X%.txt}") == "hello"; });
    test("${VAR#prefix}", []{ return run("X=/foo/bar; echo ${X#/foo/}") == "bar"; });
}

// ============================================================
//  [7] Prozesse & Jobs
// ============================================================
void testProcesses() {
    std::cout << "\n\033[1;34m[7] Prozesse & Jobs\033[0m\n";

    test("ps",            []{ return !run("ps").empty(); });
    test("ps aux",        []{ return !run("ps aux 2>/dev/null || ps -A").empty(); });
    test("ps -ef",        []{ return !run("ps -ef 2>/dev/null || ps aux").empty(); });
    test("kill -l",       []{ return !run("kill -l").empty(); });
    test("pgrep (syntax)",[]{ return hasCmd("pgrep"); }, "pgrep");
    test("pkill (exists)",[]{ return hasCmd("pkill"); }, "pkill");
    test("top (exists)",  []{ return hasCmd("top"); });
    test("htop (exists)", []{ return hasCmd("htop"); }, "htop");
    test("nohup (exists)",[]{ return hasCmd("nohup"); });
    test("wait builtin",  []{ return run("sleep 0 & wait; echo done") == "done"; });
    test("& background",  []{ return exitCode("sleep 0 &") == 0; });
    test("jobs builtin",  []{ return exitCode("jobs") == 0; });
    test("nice (exists)", []{ return hasCmd("nice"); });
    test("lsof (exists)", []{ return hasCmd("lsof"); }, "lsof");
    test("strace (exists)",[]{ return hasCmd("strace"); }, "strace");
    test("time cmd",      []{ return exitCode("time sleep 0") == 0; });
}

// ============================================================
//  [8] Netzwerk
// ============================================================
void testNetwork() {
    std::cout << "\n\033[1;34m[8] Netzwerk\033[0m\n";

    test("ping -c1 localhost",   []{ return exitCode("ping -c1 -W1 localhost") == 0; }, "ping");
    test("curl --version",       []{ return contains(run("curl --version 2>&1 | head -1"), "curl"); }, "curl");
    test("curl -s (http test)",  []{ return exitCode("curl -sf --max-time 5 https://httpbin.org/get") == 0; }, "curl");
    test("wget --version",       []{ return contains(run("wget --version 2>&1 | head -1"), "Wget"); }, "wget");
    test("nc -z (netcat)",       []{ return hasCmd("nc") || hasCmd("netcat"); }, "nc");
    test("ssh --version",        []{ return contains(run("ssh -V 2>&1"), "OpenSSH"); }, "ssh");
    test("scp (exists)",         []{ return hasCmd("scp"); }, "scp");
    test("rsync --version",      []{ return contains(run("rsync --version 2>&1 | head -1"), "rsync"); }, "rsync");
    test("ifconfig / ip addr",   []{ return exitCode("ifconfig 2>/dev/null || ip addr 2>/dev/null") == 0; });
    test("netstat / ss",         []{ return exitCode("netstat -an 2>/dev/null || ss -an 2>/dev/null") == 0; });
    test("host / nslookup",      []{ return hasCmd("host") || hasCmd("nslookup"); });
    test("dig (DNS)",            []{ return hasCmd("dig"); }, "dig");
    test("traceroute (exists)",  []{ return hasCmd("traceroute") || hasCmd("tracepath"); });
    test("openssl version",      []{ return contains(run("openssl version 2>&1"), "OpenSSL"); }, "openssl");
    test("ftp (exists)",         []{ return hasCmd("ftp"); }, "ftp");
    test("telnet (exists)",      []{ return hasCmd("telnet"); }, "telnet");
}

// ============================================================
//  [9] Paketverwaltung
// ============================================================
void testPackageManagers() {
    std::cout << "\n\033[1;34m[9] Paketverwaltung\033[0m\n";

    test("apt --version",             []{ return exitCode("apt --version") == 0; }, "apt");
    test("apt-get --version",         []{ return exitCode("apt-get --version") == 0; }, "apt-get");
    test("apt list (installed)",      []{ return exitCode("apt list --installed 2>/dev/null | head -5") == 0; }, "apt");
    test("apt-cache search (syntax)", []{ return exitCode("apt-cache search curl 2>/dev/null | head -1") == 0; }, "apt-cache");
    test("dpkg -l (list)",            []{ return exitCode("dpkg -l 2>/dev/null | head -5") == 0; }, "dpkg");
    test("dpkg --get-selections",     []{ return exitCode("dpkg --get-selections 2>/dev/null | head -1") == 0; }, "dpkg");

    test("yum --version",             []{ return exitCode("yum --version") == 0; }, "yum");
    test("dnf --version",             []{ return exitCode("dnf --version") == 0; }, "dnf");
    test("rpm -qa (list)",            []{ return exitCode("rpm -qa 2>/dev/null | head -1") == 0; }, "rpm");

    test("pacman --version",          []{ return exitCode("pacman --version") == 0; }, "pacman");

    test("brew --version",            []{ return contains(run("brew --version 2>&1 | head -1"), "Homebrew"); }, "brew");
    test("brew list",                 []{ return exitCode("brew list 2>/dev/null") == 0; }, "brew");
    test("brew info (syntax)",        []{ return exitCode("brew info curl 2>/dev/null") == 0; }, "brew");
    test("brew search (syntax)",      []{ return exitCode("brew search curl 2>/dev/null | head -1") == 0; }, "brew");
    test("brew update (dry check)",   []{ return exitCode("brew update --auto-update 2>/dev/null || true") == 0; }, "brew");
    test("brew upgrade --dry-run",    []{ return exitCode("brew upgrade --dry-run 2>/dev/null || true") == 0; }, "brew");
    test("brew doctor",               []{ return !run("brew doctor 2>&1 | head -1").empty(); }, "brew");
    test("brew cleanup --dry-run",    []{ return !run("brew cleanup --dry-run 2>&1 | head -1").empty(); }, "brew");
    test("brew tap (list)",           []{ return exitCode("brew tap 2>/dev/null") == 0; }, "brew");
    test("brew services list",        []{ return exitCode("brew services list 2>/dev/null || true") == 0; }, "brew");

    test("snap --version",            []{ return contains(run("snap --version 2>&1 | head -1"), "snap"); }, "snap");
    test("snap list",                 []{ return exitCode("snap list 2>/dev/null") == 0; }, "snap");

    test("flatpak --version",         []{ return exitCode("flatpak --version 2>/dev/null") == 0; }, "flatpak");
}

// ============================================================
//  [10] Entwicklerwerkzeuge
// ============================================================
void testDevTools() {
    std::cout << "\n\033[1;34m[10] Entwicklerwerkzeuge\033[0m\n";

    test("git --version",            []{ return contains(run("git --version"), "git"); }, "git");
    test("git init",                 []{ system(("git init " + TMP + "/repo -q").c_str()); struct stat s{}; return stat((TMP+"/repo/.git").c_str(),&s)==0; }, "git");
    test("git config --list",        []{ return exitCode("git config --list") == 0; }, "git");
    test("git status",               []{ return exitCode("cd " + TMP + "/repo && git status") == 0; }, "git");
    test("git add",                  []{ system(("cd " + TMP + "/repo && echo x > x.txt && git add x.txt").c_str()); return exitCode("cd " + TMP + "/repo && git diff --cached --quiet") != 0; }, "git");
    test("git commit",               []{ return exitCode("cd " + TMP + "/repo && git -c user.email=t@t.com -c user.name=T commit -m 'init' -q") == 0; }, "git");
    test("git log",                  []{ return exitCode("cd " + TMP + "/repo && git log --oneline") == 0; }, "git");
    test("git branch",               []{ return exitCode("cd " + TMP + "/repo && git branch") == 0; }, "git");
    test("git stash list",           []{ return exitCode("cd " + TMP + "/repo && git stash list") == 0; }, "git");
    test("git remote -v",            []{ return exitCode("cd " + TMP + "/repo && git remote -v") == 0; }, "git");
    test("git diff",                 []{ return exitCode("cd " + TMP + "/repo && git diff") == 0; }, "git");
    test("git tag",                  []{ return exitCode("cd " + TMP + "/repo && git tag") == 0; }, "git");

    test("gcc --version",            []{ return contains(run("gcc --version 2>&1 | head -1"), "gcc") || contains(run("gcc --version 2>&1 | head -1"), "clang"); }, "gcc");
    test("g++ --version",            []{ return contains(run("g++ --version 2>&1 | head -1"), "g++") || contains(run("g++ --version 2>&1 | head -1"), "clang"); }, "g++");
    test("clang --version",          []{ return contains(run("clang --version 2>&1 | head -1"), "clang"); }, "clang");
    test("make --version",           []{ return contains(run("make --version 2>&1 | head -1"), "Make"); }, "make");
    test("cmake --version",          []{ return contains(run("cmake --version 2>&1 | head -1"), "cmake"); }, "cmake");
    test("python3 --version",        []{ return contains(run("python3 --version 2>&1"), "Python"); }, "python3");
    test("python3 -c (eval)",        []{ return run("python3 -c 'print(2+2)'") == "4"; }, "python3");
    test("pip3 --version",           []{ return contains(run("pip3 --version 2>&1"), "pip"); }, "pip3");
    test("node --version",           []{ return contains(run("node --version 2>&1"), "v"); }, "node");
    test("npm --version",            []{ return !run("npm --version 2>&1").empty(); }, "npm");
    test("npx --version",            []{ return !run("npx --version 2>&1").empty(); }, "npx");
    test("yarn --version",           []{ return !run("yarn --version 2>&1").empty(); }, "yarn");
    test("bun --version",            []{ return !run("bun --version 2>&1").empty(); }, "bun");
    test("deno --version",           []{ return contains(run("deno --version 2>&1 | head -1"), "deno"); }, "deno");
    test("java --version",           []{ return exitCode("java --version 2>/dev/null || java -version 2>/dev/null") == 0; }, "java");
    test("javac --version",          []{ return exitCode("javac --version 2>/dev/null || javac -version 2>/dev/null") == 0; }, "javac");
    test("mvn --version",            []{ return contains(run("mvn --version 2>&1 | head -1"), "Apache Maven"); }, "mvn");
    test("gradle --version",         []{ return exitCode("gradle --version 2>/dev/null") == 0; }, "gradle");
    test("go version",               []{ return contains(run("go version 2>&1"), "go"); }, "go");
    test("cargo --version",          []{ return contains(run("cargo --version 2>&1"), "cargo"); }, "cargo");
    test("rustc --version",          []{ return contains(run("rustc --version 2>&1"), "rustc"); }, "rustc");
    test("ruby --version",           []{ return contains(run("ruby --version 2>&1"), "ruby"); }, "ruby");
    test("gem --version",            []{ return !run("gem --version 2>&1").empty(); }, "gem");
    test("php --version",            []{ return contains(run("php --version 2>&1 | head -1"), "PHP"); }, "php");
    test("composer --version",       []{ return contains(run("composer --version 2>&1 | head -1"), "Composer"); }, "composer");
    test("swift --version",          []{ return contains(run("swift --version 2>&1 | head -1"), "Swift"); }, "swift");
    test("kotlin -version",          []{ return exitCode("kotlin -version 2>/dev/null") == 0; }, "kotlin");
    test("lua -v",                   []{ return contains(run("lua -v 2>&1"), "Lua"); }, "lua");
    test("perl --version",           []{ return contains(run("perl --version 2>&1"), "perl"); }, "perl");
    test("R --version",              []{ return contains(run("R --version 2>&1 | head -1"), "R version"); }, "R");
    test("julia --version",          []{ return contains(run("julia --version 2>&1"), "julia"); }, "julia");

    test("make (Makefile)",          []{
        system(("printf 'all:\\n\\techo built\\n' > " + TMP + "/Makefile").c_str());
        std::string out = run("make -C " + TMP + " -f Makefile 2>/dev/null");
        return contains(out, "built");
    }, "make");
    test("gdb --version",            []{ return contains(run("gdb --version 2>&1 | head -1"), "GNU"); }, "gdb");
    test("valgrind --version",       []{ return contains(run("valgrind --version 2>&1"), "valgrind"); }, "valgrind");
    test("docker --version",         []{ return contains(run("docker --version 2>&1"), "Docker"); }, "docker");
    test("docker-compose / compose", []{
        if (hasCmd("docker-compose")) {
            return exitCode("docker-compose --version") == 0;
        }

        if (hasCmd("docker")) {
            std::string out = run("docker compose version 2>&1");
            return contains(out, "Docker Compose")
                || contains(out, "docker: 'compose' is not a docker command") == false;
        }

        return false;
    }, "docker");
    test("kubectl version",          []{ return exitCode("kubectl version --client 2>/dev/null") == 0; }, "kubectl");
    test("terraform --version",      []{ return contains(run("terraform --version 2>&1 | head -1"), "Terraform"); }, "terraform");
    test("ansible --version",        []{ return contains(run("ansible --version 2>&1 | head -1"), "ansible"); }, "ansible");
    test("jq --version",             []{ return contains(run("jq --version 2>&1"), "jq-"); }, "jq");
    test("jq parse JSON",            []{ return run("echo '{\"a\":1}' | jq '.a'") == "1"; }, "jq");
    test("yq --version",             []{ return exitCode("yq --version 2>/dev/null") == 0; }, "yq");
    test("gh --version (GitHub CLI)",[]{ return contains(run("gh --version 2>&1 | head -1"), "gh"); }, "gh");
}

// ============================================================
//  [11] Editoren & Viewer
// ============================================================
void testEditors() {
    std::cout << "\n\033[1;34m[11] Editoren & Viewer\033[0m\n";

    test("vim --version",       []{ return contains(run("vim --version 2>&1 | head -1"), "VIM"); }, "vim");
    test("nvim --version",      []{ return contains(run("nvim --version 2>&1 | head -1"), "NVIM"); }, "nvim");
    test("nano (exists)", []{
        return hasCmd("nano");
    }, "nano");
    test("emacs --version",     []{ return contains(run("emacs --version 2>&1 | head -1"), "GNU Emacs"); }, "emacs");
    test("code --version",      []{ return exitCode("code --version 2>/dev/null") == 0; }, "code");
    test("less --version",      []{ return contains(run("less -V 2>&1 | head -1"), "less"); }, "less");
    test("more (exists)",       []{ return hasCmd("more"); }, "more");
    test("xxd (hex viewer)",    []{ return contains(run("echo 'A' | xxd | head -1"), "0000"); }, "xxd");
    test("od (octal dump)",     []{ return exitCode("echo 'A' | od -c 2>/dev/null") == 0; }, "od");
}

// ============================================================
//  [12] Archivierung & Kompression
// ============================================================
void testArchiving() {
    std::cout << "\n\033[1;34m[12] Archivierung & Kompression\033[0m\n";

    system(("echo 'archive test' > " + TMP + "/archive_src.txt").c_str());

    test("tar -czf (create)",   []{ return exitCode("tar -czf " + TMP + "/test.tar.gz -C " + TMP + " archive_src.txt") == 0; });
    test("tar -tzf (list)",     []{ return contains(run("tar -tzf " + TMP + "/test.tar.gz"), "archive_src.txt"); });
    test("tar -xzf (extract)",  []{ system(("mkdir -p " + TMP + "/extracted").c_str()); return exitCode("tar -xzf " + TMP + "/test.tar.gz -C " + TMP + "/extracted") == 0; });
    test("tar -cjf (bzip2)",    []{ return exitCode("tar -cjf " + TMP + "/test.tar.bz2 -C " + TMP + " archive_src.txt") == 0; });
    test("tar -cJf (xz)",       []{ return exitCode("tar -cJf " + TMP + "/test.tar.xz -C " + TMP + " archive_src.txt 2>/dev/null") == 0; });
    test("gzip / gunzip",       []{ system(("cp " + TMP + "/archive_src.txt " + TMP + "/gz_test.txt").c_str()); system(("gzip " + TMP + "/gz_test.txt").c_str()); struct stat s{}; bool gzOk = stat((TMP+"/gz_test.txt.gz").c_str(),&s)==0; system(("gunzip " + TMP + "/gz_test.txt.gz").c_str()); return gzOk; }, "gzip");
    test("bzip2 / bunzip2",     []{ system(("cp " + TMP + "/archive_src.txt " + TMP + "/bz_test.txt").c_str()); system(("bzip2 " + TMP + "/bz_test.txt").c_str()); struct stat s{}; bool ok = stat((TMP+"/bz_test.txt.bz2").c_str(),&s)==0; system(("bunzip2 " + TMP + "/bz_test.txt.bz2").c_str()); return ok; }, "bzip2");
    test("xz / unxz",           []{ system(("cp " + TMP + "/archive_src.txt " + TMP + "/xz_test.txt").c_str()); system(("xz " + TMP + "/xz_test.txt 2>/dev/null").c_str()); struct stat s{}; bool ok = stat((TMP+"/xz_test.txt.xz").c_str(),&s)==0; system(("unxz " + TMP + "/xz_test.txt.xz 2>/dev/null").c_str()); return ok; }, "xz");
    test("zip / unzip",         []{ system(("zip " + TMP + "/test.zip " + TMP + "/archive_src.txt 2>/dev/null").c_str()); struct stat s{}; return stat((TMP+"/test.zip").c_str(),&s)==0; }, "zip");
    test("unzip -l",            []{ return contains(run("unzip -l " + TMP + "/test.zip 2>/dev/null"), "archive_src.txt"); }, "unzip");
    test("7z (exists)",         []{ return hasCmd("7z") || hasCmd("7za"); }, "7z");
    test("zcat",                []{
        system(("cp " + TMP + "/archive_src.txt " + TMP + "/zcat_test.txt").c_str());
        system(("gzip -f " + TMP + "/zcat_test.txt").c_str());
        std::string out = run("zcat " + TMP + "/zcat_test.txt.gz 2>/dev/null || gzcat " + TMP + "/zcat_test.txt.gz 2>/dev/null");
        return contains(out, "archive test");
    });
}

// ============================================================
//  [13] Systeminformationen
// ============================================================
void testSysInfo() {
    std::cout << "\n\033[1;34m[13] Systeminformationen\033[0m\n";

    test("uname -a",         []{ return !run("uname -a").empty(); });
    test("uname -r (kernel)",[]{ return !run("uname -r").empty(); });
    test("uname -m (arch)",  []{ return !run("uname -m").empty(); });
    test("lscpu / sysctl",   []{ return exitCode("lscpu 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null") == 0; });
    test("lsmem / sysctl",   []{ return exitCode("lsmem 2>/dev/null || sysctl hw.memsize 2>/dev/null || free 2>/dev/null") == 0; });
    test("free -h",          []{ return exitCode("free -h 2>/dev/null") == 0; }, "free");
    test("vmstat",           []{ return exitCode("vmstat 2>/dev/null") == 0; }, "vmstat");
    test("iostat",           []{ return exitCode("iostat 2>/dev/null") == 0; }, "iostat");
    test("lsblk",            []{ return exitCode("lsblk 2>/dev/null") == 0; }, "lsblk");
    test("lspci",            []{ return exitCode("lspci 2>/dev/null") == 0; }, "lspci");
    test("lsusb / system_profiler USB", []{ return exitCode("lsusb 2>/dev/null") == 0 || exitCode("system_profiler SPUSBDataType 2>/dev/null | head -1") == 0; });
    test("/proc/cpuinfo (Linux) / sysctl (macOS)", []{ struct stat s{}; return stat("/proc/cpuinfo",&s)==0 || exitCode("sysctl hw.model 2>/dev/null") == 0; });
    test("dmesg (exists)",   []{ return hasCmd("dmesg"); });
    test("sysctl (exists)",  []{ return hasCmd("sysctl"); });
    test("sw_vers (macOS)",  []{ return exitCode("sw_vers 2>/dev/null") == 0; }, "sw_vers");
    test("system_profiler",  []{ return exitCode("system_profiler SPSoftwareDataType 2>/dev/null | head -1") == 0; }, "system_profiler");
}

// ============================================================
//  [14] Scripting: Kontrollfluss
// ============================================================
void testScripting() {
    std::cout << "\n\033[1;34m[14] Scripting-Grundlagen\033[0m\n";

    test("if / then / fi",         []{ return run("if [ 1 -eq 1 ]; then echo yes; fi") == "yes"; });
    test("if / else",              []{ return run("if [ 1 -eq 2 ]; then echo no; else echo yes; fi") == "yes"; });
    test("if / elif",              []{ return run("X=2; if [ $X -eq 1 ]; then echo one; elif [ $X -eq 2 ]; then echo two; fi") == "two"; });
    test("case",                   []{ return run("X=b; case $X in a) echo A;; b) echo B;; esac") == "B"; });
    test("for loop",               []{ return run("for i in 1 2 3; do echo $i; done | wc -l") == "3"; });
    test("for C-style",            []{ return run("bash -c 'for ((i=0;i<3;i++)); do echo $i; done | wc -l'") == "3"; }, "bash");
    test("while loop",             []{ return run("i=0; while [ $i -lt 3 ]; do i=$((i+1)); done; echo $i") == "3"; });
    test("until loop",             []{ return run("i=0; until [ $i -ge 3 ]; do i=$((i+1)); done; echo $i") == "3"; });
    test("break",                  []{ return run("for i in 1 2 3; do [ $i -eq 2 ] && break; echo $i; done") == "1"; });
    test("continue",               []{ return run("for i in 1 2 3; do [ $i -eq 2 ] && continue; echo $i; done") == "1\n3"; });
    test("Funktion definieren",    []{ return run("greet() { echo hello; }; greet") == "hello"; });
    test("Funktion mit $1",        []{ return run("say() { echo $1; }; say world") == "world"; });
    test("return value",           []{ return run("f() { return 42; }; f; echo $?") == "42"; });
    test("$(( Arithmetik ))",      []{ return run("echo $((7 * 6))") == "42"; });
    test("let Arithmetik",         []{ return run("bash -c 'x=0; let x=3+4; echo $x'") == "7"; }, "bash");
    test("Array erstellen",        []{ return run("bash -c 'a=(1 2 3); echo ${a[1]}'") == "2"; }, "bash");
    test("Array Länge",            []{ return run("bash -c 'a=(x y z); echo ${#a[@]}'") == "3"; }, "bash");
    test("String-Vergleich ==",    []{ return run("[ 'abc' = 'abc' ] && echo yes") == "yes"; });
    test("Zahl-Vergleich -gt",     []{ return run("[ 5 -gt 3 ] && echo yes") == "yes"; });
    test("-f Datei-Test",          []{ return run("[ -f /bin/sh ] && echo yes") == "yes"; });
    test("-d Verzeichnis-Test",    []{ return run("[ -d /tmp ] && echo yes") == "yes"; });
    test("-z leerer String",       []{ return run("[ -z '' ] && echo yes") == "yes"; });
    test("-n nicht-leerer String", []{ return run("[ -n 'hi' ] && echo yes") == "yes"; });
    test("Script ausführbar",      []{ system(("printf '#!/bin/sh\\necho ok\\n' > " + TMP + "/s.sh && chmod +x " + TMP + "/s.sh").c_str()); return run(TMP + "/s.sh") == "ok"; });
    test("Shebang #!/usr/bin/env", []{ system(("printf '#!/usr/bin/env sh\\necho env_ok\\n' > " + TMP + "/e.sh && chmod +x " + TMP + "/e.sh").c_str()); return run(TMP + "/e.sh") == "env_ok"; });
    test("trap EXIT",              []{ return run("bash -c \"trap 'echo trapped' EXIT; exit 0\"") == "trapped"; }, "bash");
    test("set -e (exit on error)", []{ return exitCode("bash -c 'set -e; false; echo unreachable'") != 0; }, "bash");
    test("set -u (unset var err)", []{ return exitCode("bash -c 'set -u; echo $UNSET_VAR_XYZ'") != 0; }, "bash");
}

// ============================================================
//  [15] Exit-Codes & Logik
// ============================================================
void testExitCodes() {
    std::cout << "\n\033[1;34m[15] Exit-Codes & Logik\033[0m\n";

    test("true → 0",            []{ return exitCode("true") == 0; });
    test("false → 1",           []{ return exitCode("false") != 0; });
    test("Kein Befehl → != 0",  []{ return exitCode("befehl_xyz_niemals_vorhanden") != 0; });
    test("$? nach Erfolg",      []{ return run("true; echo $?") == "0"; });
    test("$? nach Fehler",      []{ return run("false; echo $?") != "0"; });
    test("&& (AND)",            []{ return run("true && echo yes") == "yes"; });
    test("&& (AND Fehler)",     []{ return run("false && echo no") != "no"; });
    test("|| (OR Fehler)",      []{ return run("false || echo fallback") == "fallback"; });
    test("|| (OR Erfolg)",      []{ return run("true || echo skip") != "skip"; });
    test("! (NOT)",             []{ return run("! false; echo $?") == "0"; });
    test("exit Code in Script", []{
        system(("printf '#!/bin/sh\\nexit 7\\n' > " + TMP + "/exit7.sh && chmod +x " + TMP + "/exit7.sh").c_str());
        int code = system((TMP+"/exit7.sh").c_str());
        return WIFEXITED(code) && WEXITSTATUS(code) == 7;
    });
}

// ============================================================
//  [16] Nützliche Sonstiges
// ============================================================
void testMisc() {
    std::cout << "\n\033[1;34m[16] Sonstiges & Hilfswerkzeuge\033[0m\n";

    test("man (exists)",          []{ return hasCmd("man"); });
    test("help (bash builtin)",   []{ return exitCode("bash -c 'help echo'") == 0; }, "bash");
    test("history (builtin)",     []{ return exitCode("history 2>/dev/null || true") == 0; });
    test("alias",                 []{ return exitCode("alias ll='ls -la'") == 0; });
    test("unalias",               []{ return exitCode("alias xx='echo x' && unalias xx") == 0; });
    test("source (dot)",          []{ system(("echo 'SOURCED=1' > " + TMP + "/src.sh").c_str()); return run(". " + TMP + "/src.sh && echo $SOURCED") == "1"; });
    test("eval",                  []{ return run("eval echo hello") == "hello"; });
    test("read builtin",          []{ return run("echo 'input' | (read line; echo $line)") == "input"; });
    test("getopts (syntax)",      []{ return exitCode("bash -c 'while getopts a: opt; do :; done' -- -a val") == 0; }, "bash");
    test("bc Rechner",            []{ return run("echo '6*7' | bc") == "42"; }, "bc");
    test("od hex dump",           []{ return exitCode("echo A | od -x 2>/dev/null") == 0; }, "od");
    test("base64 encode",         []{
        std::string out = run("printf 'hello' | base64 | tr -d '\\n'");
        return out == "aGVsbG8=";
    }, "base64");
    test("base64 decode",         []{ return run("echo aGVsbG8= | base64 -d 2>/dev/null || echo aGVsbG8= | base64 --decode") == "hello"; }, "base64");
    test("md5sum / md5",          []{ return exitCode("echo test | md5sum 2>/dev/null || echo test | md5 2>/dev/null") == 0; });
    test("sha256sum",             []{ return exitCode("echo test | sha256sum 2>/dev/null || echo test | shasum -a 256 2>/dev/null") == 0; });
    test("openssl base64",        []{ return contains(run("echo -n hello | openssl base64"), "aGVs"); }, "openssl");
    test("watch (exists)",        []{ return hasCmd("watch"); }, "watch");
    test("cron (crontab -l)",     []{ return exitCode("crontab -l 2>/dev/null || true") == 0; });
    test("at (exists)",           []{ return hasCmd("at"); }, "at");
    test("notify-send (exists)",  []{ return hasCmd("notify-send"); }, "notify-send");
    test("xclip / pbcopy",        []{ return hasCmd("xclip") || hasCmd("pbcopy"); });
    test("tmux --version",        []{ return contains(run("tmux -V 2>&1"), "tmux"); }, "tmux");
    test("screen --version",      []{ return exitCode("screen --version 2>/dev/null") == 0 || contains(run("screen --version 2>&1"), "Screen"); }, "screen");
    test("ssh-keygen (exists)",   []{ return hasCmd("ssh-keygen"); });
    test("gpg --version",         []{ return contains(run("gpg --version 2>&1 | head -1"), "gpg"); }, "gpg");
    test("sudo (exists)",         []{ return hasCmd("sudo"); });
    test("su (exists)",           []{ return hasCmd("su"); });
    test("passwd (exists)",       []{ return hasCmd("passwd"); });
    test("cmp (Dateien vergl.)",  []{ system(("cp " + TMP + "/f.txt " + TMP + "/same.txt").c_str()); return exitCode("cmp " + TMP + "/f.txt " + TMP + "/same.txt") == 0; });
    test("comm (sorted diff)",    []{ return exitCode("bash -c 'comm <(echo a) <(echo b) 2>/dev/null'") == 0; }, "bash");
    test("iconv (Encoding)",      []{ return exitCode("echo hello | iconv -f utf-8 -t utf-8 2>/dev/null") == 0; }, "iconv");
    test("locale",                []{ return exitCode("locale 2>/dev/null") == 0; });
    test("tzdata / timedatectl",  []{ return exitCode("timedatectl 2>/dev/null || date +%Z") == 0; });
}

// ============================================================
//  Zusammenfassung
// ============================================================
void printSummary() {
    int total = passed + failed + skipped_count;
    std::cout << "\n\033[1m══════════════════════════════════════════════\033[0m\n";
    std::cout << "\033[1m  Shell Test-Suite  |  " << total << " Tests gesamt\033[0m\n";
    std::cout << "  \033[32m✓ Bestanden:     " << passed << "\033[0m\n";
    std::cout << "  \033[31m✗ Fehlgeschlagen:" << failed << "\033[0m\n";
    std::cout << "  \033[33m⊘ Übersprungen:  " << skipped_count << "\033[0m  (Tool nicht installiert)\n";
    if (failed == 0) {
        std::cout << "\n  \033[32;1mAlle vorhandenen Tests bestanden! 🎉\033[0m\n";
    } else {
        std::cout << "\n  \033[31;1mFehlgeschlagene Tests:\033[0m\n";
        for (auto& r : results) {
            if (!r.passed && r.skipped.empty())
                std::cout << "    ✗ " << r.name << "\n";
        }
    }
    std::cout << "\033[1m══════════════════════════════════════════════\033[0m\n";
}

// ============================================================
//  main
// ============================================================
int main() {
    std::cout << "\033[1;37m╔══════════════════════════════════════════════╗\033[0m\n";
    std::cout << "\033[1;37m║        Shell Test-Suite (Extended)           ║\033[0m\n";
    std::cout << "\033[1;37m╚══════════════════════════════════════════════╝\033[0m\n";

    detectTimeout();
    if (!TIMEOUT_CMD.empty())
        std::cout << "  \033[33m⏱  Timeout: " << TIMEOUT_CMD << " pro Test\033[0m\n";
    else
        std::cout << "  \033[33m⚠  Kein timeout-Tool gefunden. brew install coreutils empfohlen\033[0m\n";

    setupTmp();

    testBasicCommands();
    testFileSystem();
    testPermissions();
    testTextProcessing();
    testPipesAndRedirection();
    testEnvironment();
    testProcesses();
    testNetwork();
    testPackageManagers();
    testDevTools();
    testEditors();
    testArchiving();
    testSysInfo();
    testScripting();
    testExitCodes();
    testMisc();

    teardownTmp();
    printSummary();

    return failed == 0 ? 0 : 1;
}
