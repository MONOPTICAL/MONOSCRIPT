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
//   // expect-runtime-error: текст программа запускается и останавливается с этим сообщением и кодом 1,
//                                  без LLVM такой тест пропускается
//   // expect-quiet                на успехе компилятор ничего не печатает
//   // xfail: причина              известный баг: падение не ломает прогон
//
// Файлы без директив считаются вспомогательными (например, импортируемые модули).
// Если xfail-тест вдруг прошёл, это ошибка прогона: баг починен, пометку надо убрать.
#include "headers/test_runner.h"
#include "headers/ast_tools.h"
#include "headers/cli.h"
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
        else if (line == "// expect-quiet")
            test.quiet = true;
        else if (startsWith(line, "// xfail: "))
            test.xfail = trim(line.substr(10));
    }
    return !test.expect.empty();
}

static RunResult runCheck(const std::vector<std::string>& command_, const TestCase& test)
{
    // Запуск из папки теста: линкер ищет импортируемые модули рядом
    std::string command = "cd " + shellQuote(test.path.parent_path().string()) + " && timeout 10";
    for (const auto& part : command_) command += " " + shellQuote(part);
    if (test.expect == "parse") command += " --parse-only";
    command += " " + shellQuote(test.path.filename().string()) + " 2>&1";

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

static bool hasErrorOnLine(const std::string& output, int line)
{
    const std::string prefix = "[line " + std::to_string(line);
    return output.find(prefix + ",") != std::string::npos || output.find(prefix + "]") != std::string::npos;
}

// Пустая строка - тест прошёл, иначе причина провала
static std::string evaluate(const TestCase& test, const RunResult& run)
{
    if (run.exitCode == 124) return "завис (таймаут 10 с)";
    if (run.exitCode != 0 && run.exitCode != 1)
        return "компилятор упал, код выхода " + std::to_string(run.exitCode);

    if (test.expect == "no-crash") return "";

    if (test.expect == "runtime-error")
    {
        if (run.exitCode == 0) return "ожидалась ошибка при запуске, но программа завершилась успешно";
        if (run.output.find(test.errorText) == std::string::npos)
            return "в выводе нет текста: " + test.errorText;
        return "";
    }

    if (test.expect == "error")
    {
        if (run.exitCode == 0) return "ожидалась ошибка компиляции, но код принят";
        if (test.errorLine != 0 && !hasErrorOnLine(run.output, test.errorLine))
            return "ошибка не на строке " + std::to_string(test.errorLine);
        if (!test.errorText.empty() && run.output.find(test.errorText) == std::string::npos)
            return "в выводе нет текста: " + test.errorText;
        return "";
    }

    if (run.exitCode != 0) return "ожидался успех, но код отвергнут";

    for (const auto& check : test.parseChecks)
        if (run.output.find(check) == std::string::npos)
            return "в разборе нет: " + check;

    // Строки вывода ищем по порядку: между ними может быть отладочный вывод
    std::istringstream outputStream(run.output);
    std::string outputLine;
    size_t nextCheck = 0;
    while (nextCheck < test.outputChecks.size() && std::getline(outputStream, outputLine))
        if (trim(outputLine) == test.outputChecks[nextCheck]) ++nextCheck;
    if (nextCheck < test.outputChecks.size())
        return "в выводе нет строки: " + test.outputChecks[nextCheck];

    // В Debug-сборке отладочный вывод разрешён
    if (!DEBUG && test.quiet && !trim(run.output).empty()) return "на успехе компилятор что-то напечатал";
    if (test.expect != "ok" && test.expect != "parse" && test.expect != "run") return "неизвестная директива expect: " + test.expect;
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
        {"crash", "Падения компилятора"},
        {"parser", "Парсер"},
        {"lexer", "Лексер"},
        {"types", "Типы пропускают лишнее"},
        {"rejected_valid", "Отвергается верный код"},
        {"unfinished", "Не доделано"},
        {"modules", "Модули"},
        {"diagnostics", "Диагностика"},
        {"ok", "Контроль: уже работает"},
        {"run", "Запуск программ"},
    };
    auto it = titles.find(group);
    if (it != titles.end()) return it->second;
    return group.empty() ? "Без раздела" : group;
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
                  << " пропущено: нужна сборка с LLVM, запусти ./ms --run_tests" << termcolor::reset << "\n";
        return;
    }

    const bool allPassed = passed + skipped == static_cast<int>(entries.size());
    std::cout << "\n";
    if (allPassed) std::cout << termcolor::green << "✓ ";
    else std::cout << termcolor::yellow << "▲ ";
    std::cout << termcolor::bold << groupTitle(group) << termcolor::reset << termcolor::grey << "  "
              << passed << " из " << entries.size() << " проходит" << termcolor::reset << "\n";

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
                          << "  починено, убери пометку xfail" << termcolor::reset << "\n";
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
    std::cout << termcolor::reset << "  " << passed << " из " << total << "\n\n";

    std::cout << "  " << termcolor::green << "Прошло:          " << passed << termcolor::reset << "\n";
    std::cout << "  " << termcolor::yellow << "Известных багов: " << knownBugs << termcolor::reset << "\n";
    if (skipped > 0) std::cout << "  " << termcolor::grey << "Пропущено:       " << skipped << " (нужна сборка с LLVM)" << termcolor::reset << "\n";
    if (fixed > 0) std::cout << "  " << termcolor::green << "Починено:        " << fixed << " (убери xfail)" << termcolor::reset << "\n";
    if (failed > 0) std::cout << "  " << termcolor::red << "Провалено:       " << failed << termcolor::reset << "\n";

    std::cout << "\n";
    if (total == 0) std::cout << termcolor::red << termcolor::bold << "Ни одного теста не найдено";
    else if (failed > 0 || fixed > 0) std::cout << termcolor::red << termcolor::bold << "Есть неожиданные результаты";
    else std::cout << termcolor::green << termcolor::bold << "Неожиданных результатов нет";
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
        const std::string group = relative.has_parent_path() ? relative.begin()->string() : "";

        ReportEntry entry;
        entry.name = relative.stem().string();

        const bool needsRun = test.expect == "run" || test.expect == "runtime-error";
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
        splitReason(test.xfail, entry.reason, entry.location);

        if (entry.failure.empty() && test.xfail.empty()) { entry.status = TestStatus::Passed; ++passed; }
        else if (entry.failure.empty()) { entry.status = TestStatus::Fixed; ++fixed; }
        else if (!test.xfail.empty()) { entry.status = TestStatus::KnownBug; ++knownBugs; }
        else { entry.status = TestStatus::Failed; ++failed; }

        groups[group].push_back(entry);
    }

    std::cout << "\n" << termcolor::bold << "MONOSCRIPT: тесты фронтенда" << termcolor::reset << "\n";

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
