// Раннер тестов фронтенда. Каждый .ms с директивой проверки запускается в отдельном процессе,
// чтобы segfault компилятора ронял один тест, а не весь прогон.
//
// Директивы пишутся комментариями в самом .ms:
//   // expect: ok                  код должен быть принят
//   // expect: no-crash            можно принять или отвергнуть, но нельзя упасть
//   // expect-error:               должна быть любая ошибка компиляции
//   // expect-error: текст         в выводе должен быть этот текст
//   // expect-error: 9: текст      ошибка на строке 9 (текст необязателен)
//   // expect-parse: (- (- 10 3) 2) в разборе должно быть это S-выражение, семантика не запускается
//   // expect-output: текст        программа при запуске печатает эту строку, директив может быть несколько,
//                                  строки проверяются по порядку; без LLVM такой тест пропускается
//   // expect-runtime-error: текст программа запускается и останавливается с этим сообщением и кодом 101,
//                                  без LLVM такой тест пропускается
//   // expect-start-error: текст   программу не удаётся запустить: компилятор выходит с этим сообщением и кодом 1,
//                                  без LLVM такой тест пропускается
//   // expect-quiet                на успехе компилятор ничего не печатает
//   // xfail: причина              известный баг: падение не ломает прогон
//
// Тесты с запуском выполняются дважды: через JIT и через --compile. Код выхода и вывод должны совпасть с ожидаемым.
//
// Файлы без директив считаются вспомогательными (например, импортируемые модули).
// Если xfail-тест вдруг прошёл, это ошибка прогона: баг починен, пометку надо убрать.
#include "headers/test_runner.h"
#include "headers/ast_tools.h"
#include "headers/cli.h"
#include "headers/ui.h"
#include "headers/symantic.h"
#include "../lexer/headers/Lexer.h"
#include "../parser/headers/Parser.h"
#include "../includes/termcolor.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

// --- Проверка одного файла ---

static std::string show(const std::shared_ptr<ASTNode>& node);

static std::string showRaw(const std::shared_ptr<ASTNode>& node)
{
    if (!node) return "nil";
    if (auto n = std::dynamic_pointer_cast<NumberNode>(node)) return std::to_string(n->value);
    if (auto n = std::dynamic_pointer_cast<FloatNumberNode>(node)) return std::to_string(n->value);
    if (auto n = std::dynamic_pointer_cast<StringNode>(node)) return "\"" + n->value + "\"";
    if (auto n = std::dynamic_pointer_cast<IdentifierNode>(node)) return n->name;
    if (std::dynamic_pointer_cast<NoneNode>(node)) return "none";
    if (std::dynamic_pointer_cast<NullNode>(node)) return "null";
    if (auto n = std::dynamic_pointer_cast<BinaryOpNode>(node))
        return "(" + n->op + " " + show(n->left) + " " + show(n->right) + ")";
    if (auto n = std::dynamic_pointer_cast<UnaryOpNode>(node))
        return "(" + n->op + " " + show(n->operand) + ")";
    if (auto n = std::dynamic_pointer_cast<CallNode>(node))
    {
        std::string result = "(call " + n->callee;
        for (const auto& arg : n->arguments) result += " " + show(arg);
        return result + ")";
    }
    if (auto n = std::dynamic_pointer_cast<AccessExpression>(node))
    {
        std::string result = "(access " + show(n->expression);
        for (auto next = std::dynamic_pointer_cast<AccessExpression>(n->nextAccess); next;
             next = std::dynamic_pointer_cast<AccessExpression>(next->nextAccess))
            result += " " + show(next->expression);
        return result + ")";
    }
    if (auto n = std::dynamic_pointer_cast<VariableAssignNode>(node))
        return "(var " + n->name + " " + show(n->expression) + ")";
    if (auto n = std::dynamic_pointer_cast<VariableReassignNode>(node))
        return "(set " + n->name + " " + show(n->expression) + ")";
    if (auto n = std::dynamic_pointer_cast<ReturnNode>(node))
        return "(return " + show(n->expression) + ")";
    if (auto n = std::dynamic_pointer_cast<BlockNode>(node))
    {
        std::string result = "(block";
        for (const auto& statement : n->statements) result += " " + show(statement);
        return result + ")";
    }
    return "(?)";
}

static std::string show(const std::shared_ptr<ASTNode>& node)
{
    if (node && node->implicitCastTo)
        return "(cast " + node->implicitCastTo->toString() + " " + showRaw(node) + ")";
    return showRaw(node);
}

// Лексер -> парсер, тела функций печатаются как S-выражения
static int parseOnly(const std::string& file)
{
    Lexer lexer(readSourceCode(file));
    lexer.tokenize();
    Parser parser(lexer.getTokens(), file);
    auto program = parser.parse();

    for (const auto& node : program->body)
    {
        auto function = std::dynamic_pointer_cast<FunctionNode>(node);
        if (!function) continue;
        auto body = std::dynamic_pointer_cast<BlockNode>(function->body);
        if (!body) continue;
        for (const auto& statement : body->statements)
            std::cout << show(statement) << "\n";
    }
    return 0;
}

int checkMain(int argc, char* argv[])
{
    try
    {
        if (argc == 3 && std::string(argv[1]) == "--parse-only")
            return parseOnly(argv[2]);

        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " [--parse-only] FILE" << std::endl;
            return 2;
        }

        std::string file = argv[1];
        auto tokens = tokenizeSource(readSourceCode(file), false);
        auto program = parseAndLinkModules(tokens, file, false);
        symanticParseModule(program, false);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}

// --- Раннер ---

#if defined(_WIN32) || defined(_WIN64)

int runFrontendTests(const std::vector<std::string>&, const std::vector<std::string>&, const std::string&, bool, const std::string&)
{
    std::cerr << "Error: tests are not supported on Windows yet." << std::endl;
    return 1;
}

#else

struct TestCase
{
    fs::path path;
    std::string expect;                   // ok, no-crash, error, parse или run
    int errorLine = 0;
    std::string errorText;
    std::vector<std::string> parseChecks;
    std::vector<std::string> outputChecks;
    bool quiet = false;
    std::string xfail;
};

struct RunResult
{
    int exitCode;
    std::string output;
};

static std::string trim(const std::string& value)
{
    const std::string whitespace = " \t\r\n";
    size_t first = value.find_first_not_of(whitespace);
    if (first == std::string::npos) return "";
    return value.substr(first, value.find_last_not_of(whitespace) - first + 1);
}

static std::string shellQuote(const std::string& value)
{
    std::string result = "'";
    for (char c : value)
    {
        if (c == '\'') result += "'\\''";
        else result += c;
    }
    return result + "'";
}

static bool startsWith(const std::string& value, const std::string& prefix)
{
    return value.rfind(prefix, 0) == 0;
}

// Длинное число считается текстом ошибки, а не номером строки
static bool isLineNumber(const std::string& value)
{
    return !value.empty() && value.size() <= 6 &&
           std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c); });
}

// false - в файле нет директив проверки, это не тест
static bool loadTest(const fs::path& path, TestCase& test)
{
    test.path = path;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line))
    {
        line = trim(line);
        if (startsWith(line, "// expect: "))
            test.expect = trim(line.substr(11));
        else if (startsWith(line, "// expect-error:"))
        {
            test.expect = "error";
            std::string rest = trim(line.substr(16));
            size_t colon = rest.find(':');
            if (colon != std::string::npos && isLineNumber(rest.substr(0, colon)))
            {
                test.errorLine = std::stoi(rest.substr(0, colon));
                rest = trim(rest.substr(colon + 1));
            }
            else if (isLineNumber(rest))
            {
                test.errorLine = std::stoi(rest);
                rest.clear();
            }
            test.errorText = rest;
        }
        else if (startsWith(line, "// expect-parse: "))
        {
            test.expect = "parse";
            test.parseChecks.push_back(trim(line.substr(17)));
        }
        else if (startsWith(line, "// expect-output: "))
        {
            test.expect = "run";
            test.outputChecks.push_back(trim(line.substr(18)));
        }
        else if (startsWith(line, "// expect-runtime-error: "))
        {
            test.expect = "runtime-error";
            test.errorText = trim(line.substr(25));
        }
        else if (startsWith(line, "// expect-start-error: "))
        {
            test.expect = "start-error";
            test.errorText = trim(line.substr(23));
        }
        else if (line == "// expect-quiet")
            test.quiet = true;
        else if (startsWith(line, "// xfail: "))
            test.xfail = trim(line.substr(10));
    }
    return !test.expect.empty();
}

static RunResult runShell(const std::string& command)
{
    RunResult result{-1, ""};
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return result;

    char buffer[4096];
    size_t count;
    while ((count = fread(buffer, 1, sizeof(buffer), pipe)) > 0)
        result.output.append(buffer, count);

    int status = pclose(pipe);
    if (WIFEXITED(status)) result.exitCode = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) result.exitCode = 128 + WTERMSIG(status);
    return result;
}

// Запуск из папки теста: линкер ищет импортируемые модули рядом
static std::string inTestDir(const TestCase& test)
{
    return "cd " + shellQuote(test.path.parent_path().string()) + " && timeout 10";
}

static RunResult runCheck(const std::vector<std::string>& command_, const TestCase& test)
{
    std::string command = inTestDir(test);
    for (const auto& part : command_) command += " " + shellQuote(part);
    if (test.expect == "parse") command += " --parse-only";
    command += " " + shellQuote(test.path.filename().string()) + " 2>&1";
    return runShell(command);
}

// Собирает тест через --compile и запускает готовый файл. Если собрать не вышло, в результате ошибка компилятора
static RunResult runCompiled(const std::string& compiler, const TestCase& test)
{
    const fs::path binary = fs::temp_directory_path() / ("ms_test_" + std::to_string(getpid()) + "_" + test.path.stem().string());

    RunResult result = runCheck({compiler, "--compile", binary.string()}, test);
    if (result.exitCode == 0)
        result = runShell(inTestDir(test) + " " + shellQuote(binary.string()) + " 2>&1");

    // Если сборка упала, компилятор оставляет промежуточные файлы
    std::error_code ignored;
    for (const char* suffix : {"", ".ll", ".o"})
        fs::remove(binary.string() + suffix, ignored);
    return result;
}

static bool hasErrorOnLine(const std::string& output, int line)
{
    const std::string prefix = "[line " + std::to_string(line);
    return output.find(prefix + ",") != std::string::npos || output.find(prefix + "]") != std::string::npos;
}

// Такой код выхода у программы, остановленной ошибкой выполнения (ms_panic в mono/rt.d)
constexpr int RUNTIME_ERROR_CODE = 101;

// Пустая строка - тест прошёл, иначе причина провала
static std::string evaluate(const TestCase& test, const RunResult& run)
{
    if (run.exitCode == 124) return "hung (10 s timeout)";

    // Программа дошла до конца: код выхода теперь её собственный, это не ошибка компилятора.
    // Строки вывода ищем по порядку: между ними может быть отладочный вывод
    const bool programFinished = run.output.find(" exited with code ") != std::string::npos;
    if (test.expect == "run" && programFinished) {
        std::istringstream programOutput(run.output);
        std::string programLine;
        size_t nextOutputCheck = 0;
        while (nextOutputCheck < test.outputChecks.size() && std::getline(programOutput, programLine))
            if (trim(programLine) == test.outputChecks[nextOutputCheck]) ++nextOutputCheck;
        if (nextOutputCheck < test.outputChecks.size())
            return "output is missing the line: " + test.outputChecks[nextOutputCheck];
        return "";
    }

    if (test.expect == "runtime-error")
    {
        if (run.exitCode != RUNTIME_ERROR_CODE)
            return "expected a runtime error with code " + std::to_string(RUNTIME_ERROR_CODE) + ", exit code " + std::to_string(run.exitCode);
        if (run.output.find(test.errorText) == std::string::npos)
            return "output is missing the text: " + test.errorText;
        return "";
    }

    if (run.exitCode == RUNTIME_ERROR_CODE && test.expect == "run")
        return "unexpected runtime error";

    if (run.exitCode != 0 && run.exitCode != 1)
        return "compiler crashed, exit code " + std::to_string(run.exitCode);

    if (test.expect == "no-crash") return "";

    if (test.expect == "start-error")
    {
        if (run.exitCode == 0) return "expected the program not to start";
        if (run.output.find(test.errorText) == std::string::npos)
            return "output is missing the text: " + test.errorText;
        return "";
    }

    if (test.expect == "error")
    {
        if (run.exitCode == 0) return "expected a compile error, but the code was accepted";
        if (test.errorLine != 0 && !hasErrorOnLine(run.output, test.errorLine))
            return "error is not on line " + std::to_string(test.errorLine);
        if (!test.errorText.empty() && run.output.find(test.errorText) == std::string::npos)
            return "output is missing the text: " + test.errorText;
        return "";
    }

    if (run.exitCode != 0) return "expected success, but the code was rejected";

    for (const auto& check : test.parseChecks)
        if (run.output.find(check) == std::string::npos)
            return "parse output is missing: " + check;

    // В Debug-сборке отладочный вывод разрешён
    if (!DEBUG && test.quiet && !trim(run.output).empty()) return "the compiler printed something on success";
    if (test.expect != "ok" && test.expect != "parse" && test.expect != "run") return "unknown expect directive: " + test.expect;
    return "";
}

static void printOutput(const std::string& output)
{
    std::istringstream stream(output);
    std::string line;
    int shown = 0;
    while (std::getline(stream, line) && shown++ < 15)
        std::cout << "      " << termcolor::grey << "| " << line << termcolor::reset << "\n";
}

// --- Отчёт ---

enum class TestStatus { Passed, KnownBug, Fixed, Failed, Skipped };

struct ReportEntry
{
    std::string name;
    TestStatus status;
    std::string reason;
    std::string location;
    std::string failure;
    std::string output;
};

// "описание, Lexer.cpp:144" -> описание и место в коде
static void splitReason(const std::string& xfail, std::string& reason, std::string& location)
{
    size_t comma = xfail.rfind(", ");
    if (comma != std::string::npos && xfail.find(".cpp", comma) != std::string::npos)
    {
        reason = xfail.substr(0, comma);
        location = xfail.substr(comma + 2);
    }
    else
        reason = xfail;
}

// Ширина в символах, а не байтах: в именах может быть кириллица
static size_t displayWidth(const std::string& value)
{
    return std::count_if(value.begin(), value.end(), [](unsigned char c) { return (c & 0xC0) != 0x80; });
}

static std::string padRight(const std::string& value, size_t width)
{
    size_t current = displayWidth(value);
    return current >= width ? value : value + std::string(width - current, ' ');
}

static std::string groupTitle(const std::string& group)
{
    static const std::map<std::string, std::string> titles = {
        {"crash", "Compiler crashes"},
        {"parser", "Parser"},
        {"lexer", "Lexer"},
        {"types", "Type checks"},
        {"rejected_valid", "Valid code accepted"},
        {"unfinished", "Unfinished"},
        {"modules", "Modules"},
        {"diagnostics", "Diagnostics"},
        {"ok", "Control: already works"},
        {"run", "Running programs"},
    };
    auto it = titles.find(group);
    if (it != titles.end()) return it->second;
    return group.empty() ? "Ungrouped" : group;
}

static void printGroup(const std::string& group, const std::vector<ReportEntry>& entries, bool verbose)
{
    size_t nameWidth = 0, locationWidth = 0;
    int passed = 0, skipped = 0;
    for (const auto& entry : entries)
    {
        if (entry.status == TestStatus::Skipped) { ++skipped; continue; }
        if (entry.status == TestStatus::Passed) { ++passed; if (!verbose) continue; }
        nameWidth = std::max(nameWidth, displayWidth(entry.name));
        locationWidth = std::max(locationWidth, displayWidth(entry.location));
    }

    if (skipped == static_cast<int>(entries.size()))
    {
        std::cout << "\n" << termcolor::grey << "- " << groupTitle(group) << "  " << skipped
                  << " skipped: needs a build with LLVM, run ./ms --run_tests" << termcolor::reset << "\n";
        return;
    }

    const bool allPassed = passed + skipped == static_cast<int>(entries.size());
    std::cout << "\n";
    if (allPassed) std::cout << termcolor::green << "✓ ";
    else std::cout << termcolor::yellow << "▲ ";
    std::cout << termcolor::bold << groupTitle(group) << termcolor::reset << termcolor::grey << "  "
              << passed << " of " << entries.size() << " passed" << termcolor::reset << "\n";

    for (const auto& entry : entries)
    {
        switch (entry.status)
        {
            case TestStatus::Skipped:
                continue;
            case TestStatus::Passed:
                if (!verbose) continue;
                std::cout << "  " << termcolor::green << "✓ " << termcolor::reset << entry.name << "\n";
                break;
            case TestStatus::KnownBug:
                std::cout << "  " << termcolor::yellow << "• " << termcolor::reset << padRight(entry.name, nameWidth)
                          << "  " << termcolor::cyan << padRight(entry.location, locationWidth) << termcolor::reset
                          << "  " << entry.reason << "\n";
                break;
            case TestStatus::Fixed:
                std::cout << "  " << termcolor::green << "★ " << padRight(entry.name, nameWidth)
                          << "  fixed, remove the xfail mark" << termcolor::reset << "\n";
                break;
            case TestStatus::Failed:
                std::cout << "  " << termcolor::red << "✗ " << padRight(entry.name, nameWidth)
                          << "  " << entry.failure << termcolor::reset << "\n";
                printOutput(entry.output);
                break;
        }

        if (verbose && entry.status == TestStatus::KnownBug)
        {
            std::cout << "      " << termcolor::grey << entry.failure << termcolor::reset << "\n";
            printOutput(entry.output);
        }
    }
}

static void printSummary(int total, int passed, int knownBugs, int failed, int fixed, int skipped)
{
    const int barWidth = 40;
    const int filled = total > 0 ? passed * barWidth / total : 0;

    std::cout << "\n" << termcolor::grey << std::string(barWidth, '-') << termcolor::reset << "\n";
    std::cout << termcolor::green;
    for (int i = 0; i < filled; ++i) std::cout << "█";
    std::cout << termcolor::yellow;
    for (int i = filled; i < barWidth; ++i) std::cout << "░";
    std::cout << termcolor::reset << "  " << passed << " of " << total << "\n\n";

    std::cout << "  " << termcolor::green << "Passed:      " << passed << termcolor::reset << "\n";
    std::cout << "  " << termcolor::yellow << "Known bugs:  " << knownBugs << termcolor::reset << "\n";
    if (skipped > 0) std::cout << "  " << termcolor::grey << "Skipped:     " << skipped << " (needs a build with LLVM)" << termcolor::reset << "\n";
    if (fixed > 0) std::cout << "  " << termcolor::green << "Fixed:       " << fixed << " (remove xfail)" << termcolor::reset << "\n";
    if (failed > 0) std::cout << "  " << termcolor::red << "Failed:      " << failed << termcolor::reset << "\n";

    std::cout << "\n";
    if (total == 0) std::cout << termcolor::red << termcolor::bold << "No tests found";
    else if (failed > 0 || fixed > 0) std::cout << termcolor::red << termcolor::bold << "Unexpected results";
    else std::cout << termcolor::green << termcolor::bold << "No unexpected results";
    std::cout << termcolor::reset << std::endl;
}

int runFrontendTests(const std::vector<std::string>& checkCommand, const std::vector<std::string>& runCommand, const std::string& testsDirPath, bool verbose, const std::string& filter)
{
    const fs::path testsDir = fs::absolute(testsDirPath);
    if (!fs::is_directory(testsDir))
    {
        std::cerr << "Error: tests directory not found: " << testsDir.string() << std::endl;
        return 1;
    }

    std::vector<TestCase> tests;
    for (const auto& entry : fs::recursive_directory_iterator(testsDir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".ms") continue;
        TestCase test;
        if (!loadTest(entry.path(), test)) continue;
        if (!filter.empty() && fs::relative(entry.path(), testsDir).string().find(filter) == std::string::npos) continue;
        tests.push_back(test);
    }
    std::sort(tests.begin(), tests.end(), [](const TestCase& a, const TestCase& b) { return a.path < b.path; });

    std::map<std::string, std::vector<ReportEntry>> groups;
    int passed = 0, knownBugs = 0, failed = 0, fixed = 0, skipped = 0;

    for (const auto& test : tests)
    {
        const fs::path relative = fs::relative(test.path, testsDir);
        ui::showProgress(passed + knownBugs + failed + fixed + skipped, static_cast<int>(tests.size()), failed + fixed, relative.string());
        const std::string group = relative.has_parent_path() ? relative.begin()->string() : "";

        ReportEntry entry;
        entry.name = relative.stem().string();

        const bool needsRun = test.expect == "run" || test.expect == "runtime-error" || test.expect == "start-error";
        if (needsRun && runCommand.empty())
        {
            entry.status = TestStatus::Skipped;
            ++skipped;
            groups[group].push_back(entry);
            continue;
        }

        const RunResult run = runCheck(needsRun ? runCommand : checkCommand, test);
        entry.failure = evaluate(test, run);
        entry.output = run.output;

        // Собранный файл должен вести себя так же, как JIT
        if (entry.failure.empty() && needsRun)
        {
            RunResult compiled = runCompiled(runCommand[0], test);
            entry.output = compiled.output;

            // Собранный файл не печатает строку о завершении, её печатает только ms run
            if (run.output.find(" exited with code ") != std::string::npos)
                compiled.output += "\n[ exited with code " + std::to_string(compiled.exitCode) + "]\n";

            if (compiled.exitCode != run.exitCode)
                entry.failure = "--compile: exit code " + std::to_string(compiled.exitCode) + ", through JIT " + std::to_string(run.exitCode);
            else if (const std::string failure = evaluate(test, compiled); !failure.empty())
                entry.failure = "--compile: " + failure;
        }

        splitReason(test.xfail, entry.reason, entry.location);

        if (entry.failure.empty() && test.xfail.empty()) { entry.status = TestStatus::Passed; ++passed; }
        else if (entry.failure.empty()) { entry.status = TestStatus::Fixed; ++fixed; }
        else if (!test.xfail.empty()) { entry.status = TestStatus::KnownBug; ++knownBugs; }
        else { entry.status = TestStatus::Failed; ++failed; }

        groups[group].push_back(entry);
    }

    ui::clearProgress();

    std::cout << "\n" << termcolor::bold << "MONOSCRIPT: tests" << termcolor::reset << "\n";

    std::vector<std::string> order = {"crash", "parser", "lexer", "types", "rejected_valid", "unfinished", "modules", "diagnostics", "ok"};
    for (const auto& [group, entries] : groups)
        if (std::find(order.begin(), order.end(), group) == order.end())
            order.push_back(group);

    for (const auto& group : order)
    {
        auto it = groups.find(group);
        if (it != groups.end()) printGroup(group, it->second, verbose);
    }

    printSummary(static_cast<int>(tests.size()), passed, knownBugs, failed, fixed, skipped);

    return (failed == 0 && fixed == 0 && !tests.empty()) ? 0 : 1;
}

#endif
