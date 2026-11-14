#include "headers/repl.h"
#include "headers/ast_tools.h"
#include "headers/lineedit.h"
#include "headers/symantic.h"
#include "headers/ui.h"
#include "../loader/headers/loader.h"
#include "../runtime/headers/ASTVisitors.h"
#include "../runtime/headers/CodeGenContext.h"
#include "../runtime/headers/CodeGenHandlers.h"
#include <llvm/ExecutionEngine/Orc/AbsoluteSymbols.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <chrono>
#include <csetjmp>
#include <csignal>
#include <cstdlib>
#include <dlfcn.h>
#include <fcntl.h>
#include <poll.h>
#include <filesystem>
#include <iostream>
#include <regex>
#include <sstream>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

// Сессия построчная: каждая строка компилируется в свой модуль и сразу выполняется в одном и том же JIT.
// Переменные верхнего уровня становятся глобальными переменными JIT, модули следующих строк ссылаются на них, а не создают заново.
// Объявления копятся текстом: семантике нужен весь контекст. Функции кодоген копирует в каждый модуль, это просто код

namespace {

sigjmp_buf jumpBack;
volatile sig_atomic_t running = 0;

// Паника программы не должна убивать сессию: тот же текст, что у ms_panic из rt.d, но вместо exit возврат в REPL
extern "C" void replPanic(const char* message, int line) {
    std::fflush(stdout);
    std::fprintf(stderr, "%sRuntime Error%s: %s", ui::style(stderr, "\033[1;31m").c_str(), ui::style(stderr, ui::RESET).c_str(), message);
    if (line > 0) std::fprintf(stderr, " at line %d", line);
    std::fprintf(stderr, "\n");
    siglongjmp(jumpBack, 1);
}

// shortcut: прыжок из обработчика сигнала может застать сборщик мусора с захваченной блокировкой. Если начнутся зависания, прерывать через флаг в прологах функций
void onInterrupt(int) {
    if (running) siglongjmp(jumpBack, 2);
}

const std::string KW = "(?:if|else|while|for|in|return|break|continue|and|or|not|is|use|const|final|public|private|this|defined|true|false|null|none|struct|attempt|panic|namespace)\\b";
// Объявление переменной, как в грамматике MONOSCRIPT-vscode: `i32 n = 5`, `const array<i32> a = []`, `n ^= 5`
const std::regex TYPED_DECL("^(?:(?:const|final)\\s+)?(?!" + KW + ")[A-Za-z_][\\w.]*(?:<[^=]*>)?\\s+(?!" + KW + ")[A-Za-z_]\\w*\\s*(?:$|//|=(?!=))");
const std::regex INFERRED_DECL("^(?:final\\s+)?[A-Za-z_]\\w*\\s*\\^=");
const std::regex ASSIGNMENT("(^|[^=!<>])=($|[^=])");
const std::regex STARTS_WITH_KEYWORD("^" + KW);
const std::regex VOID_CALL("^(?:echo|eecho|print|push|del|gc_collect|panic|sort|swapInArray)\\s*\\(");
const std::regex DECL_NAME("^\\[[^\\]]*\\]\\s*([A-Za-z_][\\w.]*)");

std::string stripBars(const std::string& line) {
    size_t at = line.find_first_not_of(" \t|");
    return at == std::string::npos ? "" : line.substr(at);
}

// Отступ строки, приведённый к "|   " на уровень
std::string barsOf(const std::string& line) {
    std::string out;
    for (char c : line) {
        if (c == '|') out += "|   ";
        else if (c != ' ' && c != '\t') break;
    }
    return out;
}

bool startsWithWord(const std::string& text, const std::string& word) {
    if (text.rfind(word, 0) != 0) return false;
    return text.size() == word.size() || !(std::isalnum(static_cast<unsigned char>(text[word.size()])) || text[word.size()] == '_');
}

// Строка открывает блок: объявление или if/for/while/attempt без однострочного тела через ':'
bool opensBlock(const std::string& line) {
    const std::string code = stripBars(line);
    if (code.rfind("[", 0) == 0) return true;
    for (const char* word : {"if", "else", "while", "for", "attempt"})
        if (startsWithWord(code, word)) return code.find(':') == std::string::npos;
    return false;
}

int countLines(const std::string& text) {
    return static_cast<int>(std::count(text.begin(), text.end(), '\n'));
}

std::string wrapInMain(const std::string& body) {
    std::string program = "[i32]main()\n";
    std::istringstream lines(body);
    for (std::string line; std::getline(lines, line);) program += "|   " + line + "\n";
    return program + "|   return 0\n";
}

struct Session {
    std::unique_ptr<llvm::orc::LLJIT> jit;
    std::string items;  // Объявления сессии в порядке ввода
    int generation = 0; // Номер строки: из него имена её main и новых глобальных переменных
};

// Компилятор местами делает exit(), а его глобальное состояние не рассчитано на повторный запуск,
// поэтому разбор, семантика и кодоген идут в дочернем процессе. Обратно приходит текст LLVM IR.
// offset: столько строк источника до введённой, чтобы ошибки показывали номер внутри неё
bool compile(const std::string& source, int offset, int generation, bool quiet, std::string& ir) {
    int out[2], err[2];
    if (pipe(out) != 0) return false;
    if (pipe(err) != 0) return false;
    std::cout.flush();
    std::fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) return false;
    if (pid == 0) {
        close(out[0]);
        close(err[0]);
        ui::forceColor = ui::colorEnabled(stderr);
        int devnull = open("/dev/null", O_WRONLY);
        dup2(devnull, STDOUT_FILENO); // Отладочная печать компилятора в сессии не нужна
        dup2(quiet ? devnull : err[1], STDERR_FILENO);
        Declarations::replPrefix = "__repl.g." + std::to_string(generation) + ".";
        try {
            auto tokens = tokenizeSource(source, false);
            for (auto& row : tokens)
                for (auto& token : row) token.line -= offset;
            auto ast = symanticParseModule(parseAndLinkModules(tokens, "", false), false);
            llvm::LLVMContext llvmContext;
            CodeGenContext context(std::filesystem::current_path().string(), &llvmContext);
            ASTGen codeGen(context);
            ast->accept(codeGen);
            if (!context.TheModule || !context.TheModule->getFunction("main")) _exit(1);
            llvm::raw_fd_ostream stream(out[1], false);
            context.TheModule->print(stream, nullptr);
            stream.flush();
            _exit(0);
        } catch (const std::exception& e) {
            if (e.what()[0] != '\0') std::cerr << ui::style(stderr, ui::RED) << e.what() << ui::style(stderr, ui::RESET) << std::endl;
            _exit(1);
        }
    }
    close(out[1]);
    close(err[1]);

    // Полоса живёт, пока ребёнок компилирует. Его ошибки копятся и печатаются, когда полоса стёрта.
    // Обе трубы читаются вместе: отладочная сборка пишет в stderr тысячи строк и иначе встала бы на полной трубе
    ui::pipelineBegin(2, false);
    ui::stage("Compiling");
    ir.clear();
    std::string errors;
    pollfd fds[2] = {{out[0], POLLIN, 0}, {err[0], POLLIN, 0}};
    std::string* sinks[2] = {&ir, &errors};
    char buffer[65536];
    for (int open = 2; open > 0;) {
        if (poll(fds, 2, -1) < 0) {
            if (errno == EINTR) continue;
            break;
        }
        for (int i = 0; i < 2; i++) {
            if (fds[i].fd < 0 || !(fds[i].revents & (POLLIN | POLLHUP | POLLERR))) continue;
            ssize_t n = read(fds[i].fd, buffer, sizeof buffer);
            if (n > 0) {
                sinks[i]->append(buffer, n);
                continue;
            }
            close(fds[i].fd);
            fds[i].fd = -1;
            open--;
        }
    }
    int status = 0;
    waitpid(pid, &status, 0);
    const bool ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;
    if (!ok || !errors.empty()) ui::pipelineStop();
    std::fputs(errors.c_str(), stderr);
    return ok;
}

// Кладёт модуль в JIT и выполняет его main. Новые глобальные переменные запоминаются, только если строка выполнилась целиком
bool execute(Session& session, const std::string& ir, int generation, const char* outputColor) {
    ui::stage("JIT: machine code");
    auto llvmContext = std::make_unique<llvm::LLVMContext>();
    llvm::SMDiagnostic diagnostic;
    auto module = llvm::parseIR(llvm::MemoryBufferRef(ir, "repl"), diagnostic, *llvmContext);
    if (!module) {
        ui::pipelineStop();
        diagnostic.print("ms", llvm::errs());
        return false;
    }

    // Наружу из модуля видны только его main и новые переменные, остальное у каждой строки своё
    const std::string entry = "__repl.main." + std::to_string(generation);
    const std::string prefix = "__repl.g." + std::to_string(generation) + ".";
    for (auto& function : *module) {
        if (function.isDeclaration()) continue;
        if (function.getName() == "main") function.setName(entry);
        else if (function.hasExternalLinkage()) function.setLinkage(llvm::GlobalValue::InternalLinkage);
    }
    std::vector<std::pair<std::string, std::string>> fresh;
    for (auto& global : module->globals()) {
        if (global.isDeclaration()) continue;
        const std::string name = global.getName().str();
        if (name.rfind(prefix, 0) == 0) fresh.emplace_back(name.substr(prefix.size()), name);
        else if (global.hasExternalLinkage()) global.setLinkage(llvm::GlobalValue::InternalLinkage);
    }

    if (auto error = session.jit->addIRModule(llvm::orc::ThreadSafeModule(std::move(module), std::move(llvmContext)))) {
        ui::pipelineStop();
        std::cerr << "JIT: " << llvm::toString(std::move(error)) << std::endl;
        return false;
    }
    auto symbol = session.jit->lookup(entry); // Здесь JIT и переводит модуль в машинный код
    ui::pipelineStop();
    if (!symbol) {
        std::cerr << "JIT: " << llvm::toString(symbol.takeError()) << std::endl;
        return false;
    }
    auto mainFunction = symbol->toPtr<int (*)()>();

    lineedit::cooked(); // Программе нужен обычный терминал: input(), Ctrl+C
    if (outputColor) std::fputs(ui::style(stdout, outputColor).c_str(), stdout);
    volatile bool finished = false;
    const int jumped = sigsetjmp(jumpBack, 1);
    if (jumped == 0) {
        running = 1;
        mainFunction();
        running = 0;
        finished = true;
    } else {
        running = 0;
    }
    if (outputColor) std::fputs(ui::style(stdout, ui::RESET).c_str(), stdout);
    std::fflush(stdout);
    if (jumped == 2) std::cerr << ui::style(stderr, ui::DIM) << "Interrupted" << ui::style(stderr, ui::RESET) << std::endl;

    if (finished)
        for (auto& [name, symbolName] : fresh) Declarations::replGlobals[name] = symbolName;
    return finished;
}

bool startJIT(Session& session) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    const std::string library = loader::findLibraryPath();
    void* handle = library.empty() ? nullptr : dlopen(library.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!handle) handle = dlopen("libm_std.so", RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        std::cerr << "Error loading standard library (libm_std.so): " << dlerror() << std::endl;
        return false;
    }

    auto jit = llvm::orc::LLJITBuilder().create();
    if (!jit) {
        std::cerr << "JIT: " << llvm::toString(jit.takeError()) << std::endl;
        return false;
    }
    session.jit = std::move(*jit);
    auto& dylib = session.jit->getMainJITDylib();
    dylib.addGenerator(llvm::cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(session.jit->getDataLayout().getGlobalPrefix())));
    llvm::orc::SymbolMap overrides;
    overrides[session.jit->mangleAndIntern("ms_panic")] = {llvm::orc::ExecutorAddr::fromPtr(&replPanic), llvm::JITSymbolFlags::Exported | llvm::JITSymbolFlags::Callable};
    llvm::cantFail(dylib.define(llvm::orc::absoluteSymbols(std::move(overrides))));
    return true;
}

void refreshWords(const Session& session) {
    lineedit::words.clear();
    std::istringstream lines(session.items);
    std::smatch match;
    for (std::string line; std::getline(lines, line);)
        if (std::regex_search(line, match, DECL_NAME)) lineedit::words.push_back(match[1]);
    for (const auto& [name, symbol] : Declarations::replGlobals) lineedit::words.push_back(name);
}

// Печатает текст по символу, как набирает человек. Не в терминале: сразу целиком
void typeOut(const std::string& text) {
    if (!ui::isTerminal(stdout)) {
        std::cout << text;
        return;
    }
    for (size_t i = 0; i < text.size();) {
        const unsigned char first = text[i];
        const size_t length = first < 0x80 ? 1 : first < 0xE0 ? 2 : first < 0xF0 ? 3 : 4;
        std::cout << text.substr(i, length) << std::flush;
        i += length;
        if (first != 0x1b) std::this_thread::sleep_for(std::chrono::milliseconds(6));
    }
}

// Объявление принято: спиннер на миг, затем галочка
void popCheck(const std::string& name) {
    const std::string green = ui::style(stdout, "\033[38;5;150m"), dim = ui::style(stdout, ui::DIM), reset = ui::style(stdout, ui::RESET);
    if (ui::isTerminal(stdout)) {
        for (const char* frame : {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴"}) {
            std::cout << "\r  " << green << frame << reset << " " << dim << name << reset << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }
    }
    std::cout << "\r  " << green << "✓" << reset << " " << dim << name << reset << "\n";
}

void printHelp() {
    const std::string accent = ui::style(stdout, ui::ACCENT), dim = ui::style(stdout, ui::DIM), reset = ui::style(stdout, ui::RESET);
    auto row = [&](const std::string& key, const std::string& text) { std::cout << "  " << accent << key << reset << std::string(10 - key.size(), ' ') << text << "\n"; };
    row(":defs", "show what this session declared");
    row(":clear", "forget all declarations and variables");
    row(":q", "quit, same as Ctrl+D");
    std::cout << dim << "  Tab completes names and indents blocks. Up and Down walk the history.\n"
              << "  A block ends with an empty line. A bare expression prints its value." << reset << "\n";
}

}

int runRepl() {
    Session session;
    if (!startJIT(session)) return 1;
    std::signal(SIGINT, onInterrupt);
    if (const char* home = std::getenv("HOME")) lineedit::loadHistory(std::string(home) + "/.ms_history");

    const std::string accent = ui::style(stdout, ui::ACCENT), dim = ui::style(stdout, ui::DIM),
                      reset = ui::style(stdout, ui::RESET);
    const bool color = ui::colorEnabled(stdout);
    // Стрелка помнит прошлую строку: фиолетовая, если всё прошло, красная после ошибки
    auto prompt = [&](bool ok) { return color ? dim + "ms " + reset + ui::style(stdout, ok ? ui::ACCENT : ui::RED) + "❯ " + reset : "ms> "; };
    bool ok = true;
    const std::string more = color ? dim + "   · " + reset : "...  ";

    ui::printBanner(stdout);
    std::cout << "\n" << dim;
    typeOut("  Every line runs at once and keeps its variables. :help for commands, Ctrl+D to quit.");
    std::cout << reset << "\n\n";

    std::string line;
    while (true) {
        const auto read = lineedit::read(prompt(ok), line);
        if (read == lineedit::Result::Eof) break;
        if (read == lineedit::Result::Cancel) continue;
        lineedit::addHistory(line);

        const std::string command = stripBars(line);
        if (command.empty()) continue;
        if (command == ":q" || command == ":quit" || command == "exit" || command == "quit") break;
        if (command == ":help") {
            printHelp();
            continue;
        }
        if (command == ":defs") {
            if (session.items.empty() && Declarations::replGlobals.empty()) std::cout << dim << "  nothing declared yet" << reset << "\n";
            std::istringstream lines(session.items);
            for (std::string item; std::getline(lines, item);) std::cout << "  " << lineedit::highlight(item) << "\n";
            continue;
        }
        if (command == ":clear") {
            session.items.clear();
            Declarations::replGlobals.clear();
            refreshWords(session);
            std::cout << dim << "  session cleared" << reset << "\n";
            continue;
        }

        // Блок читаем до пустой строки, отступ следующей строки подставляется сам
        std::string snippet = line + "\n";
        bool cancelled = false;
        if (opensBlock(line)) {
            for (std::string previous = line;;) {
                std::string next;
                const auto result = lineedit::read(more, next, barsOf(previous) + (opensBlock(previous) ? "|   " : ""));
                if (result == lineedit::Result::Cancel) cancelled = true;
                if (result != lineedit::Result::Line || stripBars(next).empty()) break;
                lineedit::addHistory(next);
                snippet += next + "\n";
                previous = next;
            }
        }
        if (cancelled) continue;

        const int generation = ++session.generation;
        const int itemLines = countLines(session.items);
        const bool singleLine = countLines(snippet) == 1;
        std::string ir;

        if (command[0] == '[' && snippet.find("]main(") != std::string::npos) {
            // Своя main: выполняется как есть и не запоминается
            ok = compile(session.items + snippet, itemLines, generation, false, ir) && execute(session, ir, generation, nullptr);
        } else if (command[0] == '[' || startsWithWord(command, "use") ||
                   (singleLine && (std::regex_search(command, TYPED_DECL) || std::regex_search(command, INFERRED_DECL)))) {
            // Объявление живёт до конца сессии. Его выполнение инициализирует новые переменные
            ok = compile(session.items + snippet + wrapInMain(""), itemLines, generation, false, ir) && execute(session, ir, generation, nullptr);
            if (ok) {
                session.items += snippet;
                refreshWords(session);
                std::smatch match;
                if (std::regex_search(command, match, DECL_NAME)) popCheck(match[1]);
            }
        } else {
            // Голое выражение печатает своё значение. Не вышло (void, массив): выполняем строку как есть
            // Не собралось ни так ни так: ошибки показываем от выражения, у `x` без echo только "Unknown statement"
            const bool expression = singleLine && !std::regex_search(command, ASSIGNMENT) && !std::regex_search(command, STARTS_WITH_KEYWORD) &&
                                    !std::regex_search(command, VOID_CALL);
            const std::string printed = session.items + wrapInMain("echo(\"\" + (" + command + "))");
            if (expression && compile(printed, itemLines + 1, generation, true, ir))
                ok = execute(session, ir, generation, ui::ACCENT);
            else if (compile(session.items + wrapInMain(snippet), itemLines + 1, generation, expression, ir))
                ok = execute(session, ir, generation, nullptr);
            else {
                ok = false;
                if (expression) compile(printed, itemLines + 1, generation, false, ir);
            }
        }
    }
    lineedit::cooked();
    return 0;
}
