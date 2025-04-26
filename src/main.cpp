#include "lexer/headers/Lexer.h"
#include "lexer/headers/Token.h"
#include "parser/headers/Parser.h"
#include "linker/headers/Linker.h"
#include "visitors/headers/TypeSymbolVisitor.h"
#include "includes/ASTDebugger.hpp"
#include "runtime/headers/CodeGenContext.h" 
#include "runtime/headers/ASTVisitors.h"
#include <llvm/IR/Module.h>         // Для llvm::Module
#include <llvm/Support/raw_ostream.h> // Для llvm::outs()
#include <llvm/IRReader/IRReader.h> // Для parseIR
#include <llvm/Support/MemoryBuffer.h> // Для MemoryBuffer
#include <llvm/Support/SourceMgr.h> // Для SMDiagnostic
// For Execution
// ----------------------------------------------
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/TargetSelect.h>
// ----------------------------------------------
#include <llvm/Support/Error.h> // Для ExitOnError
#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <chrono>
#include <dlfcn.h> // Для dlopen

void printHelp(const char* programName) {
    std::cout << "Использование: " << programName << " [ОПЦИИ] [ФАЙЛ]\n\n"
              << "Опции:\n"
              << "  --help           Показать эту справку и выйти\n"
              << "  --tokens         Показать токены\n"
              << "  --ast            Показать AST\n"
              << "\nЕсли ФАЙЛ не указан, ввод читается со стандартного ввода.\n"
              << std::endl;
}

int executeModule(llvm::Module* module) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    llvm::ExitOnError ExitOnErr;
    ExitOnErr.setBanner("Error JIT: ");

    void* handle = dlopen(STDLIB_SO_PATH, RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        std::cerr << "Ошибка загрузки libm_std.so: " << dlerror() << std::endl;
        return 1;
    }

    // JIT
    auto JIT = ExitOnErr(llvm::orc::LLJITBuilder().create());

    auto& jitDylib = JIT->getMainJITDylib();
    jitDylib.addGenerator(
        cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            JIT->getDataLayout().getGlobalPrefix()))
    );

    // --- Клонирование модуля ---
    auto Ctx = std::make_unique<llvm::LLVMContext>();
    std::unique_ptr<llvm::Module> ClonedModule;

    { // Ограничиваем область видимости буфера и потока
        llvm::SmallString<0> IRBuffer;
        llvm::raw_svector_ostream OS(IRBuffer);
        module->print(OS, nullptr); // Сериализуем оригинальный модуль в буфер

        llvm::SMDiagnostic Err;
        ClonedModule = llvm::parseIR(
            *llvm::MemoryBuffer::getMemBuffer(IRBuffer.str()), // Создаем MemoryBuffer из строки
            Err,
            *Ctx // Используем новый контекст для клонированного модуля
        );

        if (!ClonedModule) {
            std::cerr << "Ошибка при клонировании модуля для JIT: ";
            Err.print("executeModule", llvm::errs());
            return 1;
        }
    }
    // --- Конец клонирования ---


    // ThreadSafeModule и добавляем в JIT
    llvm::orc::ThreadSafeModule TSM(std::move(ClonedModule), std::move(Ctx));
    ExitOnErr(JIT->addIRModule(std::move(TSM)));

    // main
    auto MainSymbol = ExitOnErr(JIT->lookup("main"));

    // int(*)()
    auto MainFn = MainSymbol.toPtr<int(*)()>();

    // Вызываем функцию
    std::cout << "Выполнение функции main()...\n";
    int Result = MainFn();
    std::cout << "Функция main() завершена.\n";

    return Result;
}

int main(int argc, char* argv[]) {
    bool showTokens = false;
    bool showAST = false;
    std::string inputFile;

    // Парсинг аргументов
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "--tokens") {
            showTokens = true;
        } else if (arg == "--ast") {
            showAST = true;
        } else if (arg[0] != '-') {
            inputFile = arg;
        } else {
            std::cerr << "Неизвестная опция: " << arg << std::endl;
            return 1;
        }
    }
    if (!showTokens && !showAST) showTokens = showAST = true;

    // Чтение исходника
    std::string sourceCode;
    if (!inputFile.empty()) {
        std::ifstream file(inputFile);
        if (!file) {
            std::cerr << "Ошибка: не удалось открыть файл " << inputFile << std::endl;
            return 1;
        }
        std::string line;
        while (std::getline(file, line)) sourceCode += line + "\n";
    } else {
        std::cout << "Введите исходный код (Ctrl+D для завершения):\n";
        std::string line;
        while (std::getline(std::cin, line)) sourceCode += line + "\n";
    }

    try {
        auto t_start = std::chrono::high_resolution_clock::now();
        Lexer lexer(sourceCode);
        lexer.tokenize();
        const auto tokens = lexer.getTokens();

        if (showTokens) {
            std::cout << "\n--- Токены ---\n";
            lexer.printTokens();
            std::cout << "--- Конец токенов ---\n\n";
        }
        
        Parser parser(tokens);
        auto program = parser.parse();

        // Получаем путь текущего файла
        std::string currentFilePath = std::filesystem::current_path().string();
        Linker linker(currentFilePath);
        
        // Добавляем основной модуль
        if (!linker.addModule(std::filesystem::path(inputFile).stem().string(), inputFile, program)) {
            return 1;
        }
        
        if (!linker.linkModules()) {
            return 1;
        }
        
        auto linkedModules = linker.getLinkedASTs();
        
        // Создаем объединенный AST из всех модулей
        std::shared_ptr<ProgramNode> combinedAST = std::make_shared<ProgramNode>();

        std::cout << "--- Модули ---" << std::endl;

        // Выводим информацию о связанных модулях
        for (const auto& [name, module] : linker.getModules()) {
            std::cout << "Модуль: " << name << " (" << module.path << ")" << std::endl;
            std::cout << "  Функции: " << module.functions.size() << std::endl;
            std::cout << "  Глобальные переменные: " << module.globals.size() << std::endl;
            std::cout << "  Импортировано: " << module.imports.size() << " символов" << std::endl;
            
            // Копируем все узлы из текущего модуля в объединенный AST
            if (module.ast) {
                for (const auto& node : module.ast->body) {
                    // Пропускаем директивы импорта в объединенном AST, они уже обработаны
                    if (!std::dynamic_pointer_cast<ImportNode>(node)) {
                        combinedAST->body.push_back(node);
                    }
                }
            }
        }

        std::cout << "--- Конец модулей ---\n";

        if (showAST && combinedAST) {
            std::cout << "\n--- Объединенный AST ---\n";
            ASTDebugger::debug(combinedAST);
            std::cout << "--- Конец объединенного AST ---\n";
        }
        else if (!combinedAST) {
            std::cerr << "Ошибка: не удалось разобрать исходный код.\n";
            return 1;
        }

        auto t_analysis = std::chrono::high_resolution_clock::now();

        TypeSymbolVisitor typeSymbolVisitor;

        combinedAST->accept(typeSymbolVisitor);

        if (showAST && combinedAST) {
            std::cout << "\n--- AST(2) ---\n";
            ASTDebugger::debug(combinedAST);
            std::cout << "--- Конец AST(2) ---\n";
        } else if (!combinedAST) {
            std::cerr << "Ошибка: не удалось разобрать исходный код.\n";
            return 1;
        }

        CodeGenContext context(currentFilePath);
        ASTGen codeGen(context);
        combinedAST->accept(codeGen);
        
        auto t_codegen = std::chrono::high_resolution_clock::now();

        std::cout << "\n--- LLVM IR ---" << std::endl;
        context.TheModule->print(llvm::outs(), nullptr);
        std::cout << "--- Конец LLVM IR ---\n" << std::endl;

        
        std::cout << "\n--- Запуск программы ---" << std::endl;
        try {
            auto t_jit_start = std::chrono::high_resolution_clock::now();
            if (context.TheModule != nullptr) { 
                    int result = executeModule(context.TheModule.get());
                    auto t_jit_end = std::chrono::high_resolution_clock::now();
                    std::cout << "Программа выполнена. Результат: " << result << std::endl;

                    auto ms_analysis = std::chrono::duration_cast<std::chrono::milliseconds>(t_analysis - t_start).count();
                    auto ms_codegen  = std::chrono::duration_cast<std::chrono::milliseconds>(t_codegen - t_analysis).count();
                    auto ms_jit      = std::chrono::duration_cast<std::chrono::milliseconds>(t_jit_end - t_jit_start).count();
                    auto ms_total    = std::chrono::duration_cast<std::chrono::milliseconds>(t_jit_end - t_start).count();

                    std::cout << "\n--- Тайминги ---" << std::endl;
                    std::cout << "Анализ и парсинг: " << ms_analysis << " мс" << std::endl;
                    std::cout << "Генерация IR:      " << ms_codegen  << " мс" << std::endl;
                    std::cout << "JIT + исполнение:  " << ms_jit      << " мс" << std::endl;
                    std::cout << "Всего:             " << ms_total    << " мс" << std::endl;
                    std::cout << "--- Конец таймингов ---\n" << std::endl;
            } else {
                    std::cerr << "Ошибка: Не удалось получить модуль LLVM для выполнения." << std::endl;
            }
    
        } catch (const std::exception& e) {
            std::cerr << "Ошибка при выполнении: " << e.what() << std::endl;
        }
        std::cout << "--- Конец запуска ---\n" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}