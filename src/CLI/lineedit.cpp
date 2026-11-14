#include "headers/lineedit.h"
#include "headers/ui.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <set>
#include <termios.h>
#include <unistd.h>

namespace lineedit {

std::vector<std::string> words;

// Слова из грамматики MONOSCRIPT-vscode (syntaxes/ms.tmLanguage.json)
static const std::set<std::string> KEYWORDS = {"if", "else", "while", "for", "in", "return", "break", "continue", "and", "or", "not", "is",
    "use", "const", "final", "public", "private", "this", "defined", "struct", "attempt", "panic", "namespace"};
static const std::set<std::string> CONSTANTS = {"true", "false", "null", "none"};
static const std::set<std::string> TYPES = {"i1", "i8", "i16", "i32", "i64", "float", "f64", "char", "string", "void", "auto", "bool",
    "array", "map", "func", "result"};
static const std::set<std::string> BUILTINS = {"echo", "eecho", "print", "input", "scat", "substr", "strfind", "startsWith", "count",
    "isNumeric", "isSpace", "strim", "split", "reverse", "lower", "upper", "toInt", "toString_int", "toString_char", "toString_bool",
    "toString_float", "toString_f64", "toString_long", "toArray", "toString_char_array", "gc_collect", "len", "push", "pop", "copy", "cstr",
    "del", "has", "ok", "err", "range", "sort", "indexOf", "mapped", "swapInArray"};

static const char* const C_KEYWORD = "\033[38;5;141m";
static const char* const C_TYPE = "\033[38;5;117m";
static const char* const C_STRING = "\033[38;5;150m";
static const char* const C_NUMBER = "\033[38;5;215m";
static const char* const C_BUILTIN = "\033[38;5;222m";
static const char* const C_LABEL = "\033[38;5;210m";
static const char* const C_COMMENT = "\033[38;5;244m";
static const char* const C_BAR = "\033[38;5;240m";
static const char* const C_RESET = "\033[0m";

std::string highlight(const std::string& code) {
    if (!ui::colorEnabled(stdout)) return code;
    std::string out;
    auto paint = [&](const char* color, const std::string& text) { out += color + text + C_RESET; };
    auto isWord = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; };

    for (size_t i = 0; i < code.size();) {
        const char c = code[i];
        if (c == '/' && i + 1 < code.size() && code[i + 1] == '/') {
            paint(C_COMMENT, code.substr(i));
            break;
        }
        if (c == '"' || c == '\'') {
            size_t end = i + 1;
            while (end < code.size() && code[end] != c) end += code[end] == '\\' ? 2 : 1;
            end = std::min(end + 1, code.size());
            paint(C_STRING, code.substr(i, end - i));
            i = end;
        } else if (std::isdigit(static_cast<unsigned char>(c)) && (i == 0 || !isWord(code[i - 1]))) {
            size_t end = i;
            while (end < code.size() && (isWord(code[end]) || code[end] == '.')) end++;
            paint(C_NUMBER, code.substr(i, end - i));
            i = end;
        } else if (isWord(c) || (c == '@' && i + 1 < code.size() && isWord(code[i + 1]))) {
            size_t end = i + 1;
            while (end < code.size() && isWord(code[end])) end++;
            const std::string word = code.substr(i, end - i);
            if (c == '@') paint(C_LABEL, word);
            else if (KEYWORDS.count(word)) paint(C_KEYWORD, word);
            else if (CONSTANTS.count(word)) paint(C_NUMBER, word);
            else if (TYPES.count(word)) paint(C_TYPE, word);
            else if (BUILTINS.count(word) && end < code.size() && code[end] == '(') paint(C_BUILTIN, word);
            else out += word;
            i = end;
        } else if (c == '|' && !(i + 1 < code.size() && code[i + 1] == '>')) {
            paint(C_BAR, "|");
            i++;
        } else if (c == '^' && i + 1 < code.size() && code[i + 1] == '=') {
            paint(C_KEYWORD, "^=");
            i += 2;
        } else {
            out += c;
            i++;
        }
    }
    return out;
}

// --- Терминал ---

static termios original;
static bool rawMode = false;

void cooked() {
    if (rawMode) tcsetattr(STDIN_FILENO, TCSANOW, &original);
    rawMode = false;
}

static void raw() {
    if (rawMode) return;
    static bool saved = false;
    if (!saved) {
        tcgetattr(STDIN_FILENO, &original);
        std::atexit(cooked);
        saved = true;
    }
    termios mode = original;
    mode.c_iflag &= ~(ICRNL | IXON | BRKINT | ISTRIP | INPCK);
    mode.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    mode.c_cc[VMIN] = 1;
    mode.c_cc[VTIME] = 0;
    // TCSANOW, а не TCSAFLUSH: вставленные из буфера следующие строки не должны пропасть
    tcsetattr(STDIN_FILENO, TCSANOW, &mode);
    rawMode = true;
}

static void put(const std::string& text) {
    for (size_t done = 0; done < text.size();) {
        ssize_t n = write(STDOUT_FILENO, text.data() + done, text.size() - done);
        if (n <= 0) return;
        done += n;
    }
}

static bool readByte(char& c) {
    return ::read(STDIN_FILENO, &c, 1) == 1;
}

static size_t columns(const std::string& text) {
    size_t count = 0;
    for (unsigned char c : text) count += (c & 0xC0) != 0x80; // Байты продолжения UTF-8 не занимают места
    return count;
}

// shortcut: строка длиннее терминала переносится и перерисовка ломается. Если понадобится, считать строки как linenoise
static void refresh(const std::string& prompt, const std::string& line, size_t cursor) {
    std::string out = "\r" + prompt + highlight(line) + "\033[0m\033[K";
    if (size_t tail = columns(line.substr(cursor))) out += "\033[" + std::to_string(tail) + "D";
    put(out);
}

// --- История ---

static std::vector<std::string> history;
static std::string historyPath;

void loadHistory(const std::string& path) {
    historyPath = path;
    std::ifstream file(path);
    for (std::string line; std::getline(file, line);)
        if (!line.empty()) history.push_back(line);
}

void addHistory(const std::string& line) {
    if (line.find_first_not_of(" \t|") == std::string::npos) return;
    if (!history.empty() && history.back() == line) return;
    history.push_back(line);
    if (historyPath.empty()) return;
    std::ofstream file(historyPath, std::ios::app);
    file << line << "\n";
}

// --- Tab ---

static void complete(const std::string& prompt, std::string& line, size_t& cursor) {
    size_t start = cursor;
    while (start > 0 && (std::isalnum(static_cast<unsigned char>(line[start - 1])) || line[start - 1] == '_')) start--;
    const std::string prefix = line.substr(start, cursor - start);
    if (prefix.empty()) {
        line.insert(cursor, "    ");
        cursor += 4;
        return;
    }

    std::set<std::string> matches;
    for (const auto* list : {&KEYWORDS, &TYPES, &BUILTINS, &CONSTANTS})
        for (const auto& word : *list)
            if (word.rfind(prefix, 0) == 0) matches.insert(word);
    for (const auto& word : words)
        if (word.rfind(prefix, 0) == 0) matches.insert(word);
    if (matches.empty()) return;

    // Общее начало всех вариантов
    std::string common = *matches.begin();
    for (const auto& word : matches) {
        size_t n = 0;
        while (n < common.size() && n < word.size() && common[n] == word[n]) n++;
        common.resize(n);
    }
    line.insert(cursor, common.substr(prefix.size()));
    cursor += common.size() - prefix.size();

    if (matches.size() > 1 && common == prefix) {
        std::string list = "\r\n";
        for (const auto& word : matches) list += ui::style(stdout, ui::DIM) + word + ui::style(stdout, ui::RESET) + "  ";
        put(list + "\r\n");
    }
}

// --- Чтение строки ---

static bool onlyIndent(const std::string& text) {
    return text.find_first_not_of(" \t|") == std::string::npos;
}

Result read(const std::string& prompt, std::string& line, const std::string& prefill) {
    raw();
    line = prefill;
    size_t cursor = line.size();
    size_t historyIndex = history.size();
    std::string draft;
    refresh(prompt, line, cursor);

    auto prevChar = [&]() { size_t p = cursor - 1; while (p > 0 && (static_cast<unsigned char>(line[p]) & 0xC0) == 0x80) p--; return p; };
    auto nextChar = [&]() { size_t p = cursor + 1; while (p < line.size() && (static_cast<unsigned char>(line[p]) & 0xC0) == 0x80) p++; return p; };
    auto showHistory = [&](size_t index) {
        if (historyIndex == history.size()) draft = line;
        historyIndex = index;
        line = index == history.size() ? draft : history[index];
        cursor = line.size();
    };

    for (char c; readByte(c);) {
        switch (c) {
        case '\r':
        case '\n':
            refresh(prompt, line, line.size());
            put("\r\n");
            return Result::Line;
        case 3: // Ctrl+C
            put(ui::style(stdout, ui::DIM) + "^C" + ui::style(stdout, ui::RESET) + "\r\n");
            return Result::Cancel;
        case 4: // Ctrl+D
            if (line.empty()) {
                put("\r\n");
                return Result::Eof;
            }
            if (cursor < line.size()) line.erase(cursor, nextChar() - cursor);
            break;
        case 127:
        case 8: // Backspace. В отступе стирает весь шаг "|   "
            if (cursor == 0) break;
            if (cursor >= 4 && line.compare(cursor - 4, 4, "|   ") == 0 && onlyIndent(line.substr(0, cursor))) {
                line.erase(cursor - 4, 4);
                cursor -= 4;
            } else {
                size_t p = prevChar();
                line.erase(p, cursor - p);
                cursor = p;
            }
            break;
        case '\t':
            if (onlyIndent(line.substr(0, cursor))) {
                line.insert(cursor, "|   ");
                cursor += 4;
            } else {
                complete(prompt, line, cursor);
            }
            break;
        case 1: cursor = 0; break;                 // Ctrl+A
        case 5: cursor = line.size(); break;       // Ctrl+E
        case 11: line.erase(cursor); break;        // Ctrl+K
        case 21: line.erase(0, cursor); cursor = 0; break; // Ctrl+U
        case 23: { // Ctrl+W
            size_t p = cursor;
            while (p > 0 && line[p - 1] == ' ') p--;
            while (p > 0 && line[p - 1] != ' ') p--;
            line.erase(p, cursor - p);
            cursor = p;
            break;
        }
        case 12: put("\033[H\033[2J"); break;      // Ctrl+L
        case 27: { // Escape-последовательности: стрелки, Home, End, Delete
            char a, b;
            if (!readByte(a) || !readByte(b)) break;
            if (a != '[' && a != 'O') break;
            if (b >= '0' && b <= '9') {
                char tilde;
                if (!readByte(tilde) || tilde != '~') break;
                if (b == '1' || b == '7') cursor = 0;
                else if (b == '4' || b == '8') cursor = line.size();
                else if (b == '3' && cursor < line.size()) line.erase(cursor, nextChar() - cursor);
                break;
            }
            if (b == 'A' && historyIndex > 0) showHistory(historyIndex - 1);
            else if (b == 'B' && historyIndex < history.size()) showHistory(historyIndex + 1);
            else if (b == 'C' && cursor < line.size()) cursor = nextChar();
            else if (b == 'D' && cursor > 0) cursor = prevChar();
            else if (b == 'H') cursor = 0;
            else if (b == 'F') cursor = line.size();
            break;
        }
        default:
            if (static_cast<unsigned char>(c) < 32) break;
            // Свой отступ (например, вставленный текст) заменяет подставленный
            if (c == '|' && line == prefill && cursor == line.size()) {
                line.clear();
                cursor = 0;
            }
            line.insert(cursor, 1, c);
            cursor++;
        }
        refresh(prompt, line, cursor);
    }
    put("\r\n");
    return Result::Eof;
}

}
