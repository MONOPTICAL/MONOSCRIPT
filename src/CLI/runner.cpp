#include "headers/runner.h"
#include "../runtime/headers/CodeGenContext.h"
#include "../runtime/headers/ASTVisitors.h"
#include "../includes/ASTDebugger.hpp"
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Error.h>
#include <iostream>
#include <chrono>
#include <dlfcn.h>

int executeModule(llvm::Module* module, std::string mainFunction) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    llvm::ExitOnError ExitOnErr;
    ExitOnErr.setBanner("Error JIT: ");

    void* handle = dlopen(STDLIB_SO_PATH, RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        std::cerr << "Error loading libm_std.so: " << dlerror() << std::endl;
        return 1;
    }

    // JIT
    auto JIT = ExitOnErr(llvm::orc::LLJITBuilder().create());

    auto& jitDylib = JIT->getMainJITDylib();
    jitDylib.addGenerator(
        llvm::cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            JIT->getDataLayout().getGlobalPrefix()))
    );

    // --- Клонирование модуля ---
    auto Ctx = std::make_unique<llvm::LLVMContext>();
    std::unique_ptr<llvm::Module> ClonedModule;

    { // Ограничиваем область видимости буфера и потока
        llvm::SmallString<0> IRBuffer;
        llvm::raw_svector_ostream OS(IRBuffer);
        module->print(OS, nullptr);

        llvm::SMDiagnostic Err;
        ClonedModule = llvm::parseIR(
            *llvm::MemoryBuffer::getMemBuffer(IRBuffer.str()),
            Err,
            *Ctx
        );

        if (!ClonedModule) {
            std::cerr << "Error cloning module for JIT: ";
            Err.print("executeModule", llvm::errs());
            return 1;
        }
    }

    // ThreadSafeModule
    llvm::orc::ThreadSafeModule TSM(std::move(ClonedModule), std::move(Ctx));
    ExitOnErr(JIT->addIRModule(std::move(TSM)));

    auto MainSymbol = ExitOnErr(JIT->lookup(mainFunction));
    auto MainFn = MainSymbol.toPtr<int(*)()>();

    // Вызываем функцию
    std::cout << "Executing " << mainFunction << "()...\n";
    int Result = MainFn();
    std::cout << "Function " << mainFunction << "() executed.\n";

    return Result;
}

int runProgram(std::shared_ptr<ProgramNode> combinedAST, const std::string& currentFilePath, bool showAST) {
    auto t_start = std::chrono::high_resolution_clock::now();
    
    // Code generation
    CodeGenContext context(currentFilePath);
    ASTGen codeGen(context);
    combinedAST->accept(codeGen);
    
    auto t_codegen = std::chrono::high_resolution_clock::now();

    std::cout << "\n--- LLVM IR ---" << std::endl;
    context.TheModule->print(llvm::outs(), nullptr);
    std::cout << "--- END LLVM IR ---\n" << std::endl;
    
    std::cout << "\n--- Запуск программы ---" << std::endl;
    
    try {
        auto t_jit_start = std::chrono::high_resolution_clock::now();
        
        if (context.TheModule != nullptr) { 
            std::string entryFunctionName = "main";
            
            // Поиск функции с меткой @entry
            for (const auto& node : combinedAST->body) {
                if (auto func = std::dynamic_pointer_cast<FunctionNode>(node)) {
                    for (const auto& label : func->labels) {
                        if (label == "@entry") {
                            entryFunctionName = func->name;
                            break;
                        }
                    }
                }
            }
            
            int result = executeModule(context.TheModule.get(), entryFunctionName);
            auto t_jit_end = std::chrono::high_resolution_clock::now();
            std::cout << "Программа выполнена. Результат: " << result << std::endl;

            // Статистика по времени
            auto ms_codegen = std::chrono::duration_cast<std::chrono::milliseconds>(t_codegen - t_start).count();
            auto ms_jit = std::chrono::duration_cast<std::chrono::milliseconds>(t_jit_end - t_jit_start).count();
            auto ms_total = std::chrono::duration_cast<std::chrono::milliseconds>(t_jit_end - t_start).count();

            std::cout << "\n--- Тайминги ---" << std::endl;
            std::cout << "Генерация IR:      " << ms_codegen << " мс" << std::endl;
            std::cout << "JIT + исполнение:  " << ms_jit << " мс" << std::endl;
            std::cout << "Всего:             " << ms_total << " мс" << std::endl;
            std::cout << "--- Конец таймингов ---\n" << std::endl;
            
            return result;
        } else {
            std::cerr << "Ошибка: Не удалось получить модуль LLVM для выполнения." << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка при выполнении: " << e.what() << std::endl;
        return 1;
    }
}