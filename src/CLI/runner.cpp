#include "headers/runner.h"
#include "../runtime/headers/CodeGenContext.h"
#include "../runtime/headers/ASTVisitors.h"
#include "../includes/ASTDebugger.hpp"
#include "../loader/headers/loader.h"
#include "headers/ui.h"
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Error.h>
#include <iostream>
#include <chrono>
#include <dlfcn.h>

#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

int executeModule(std::unique_ptr<llvm::Module> module, std::unique_ptr<llvm::LLVMContext> Ctx, std::string mainFunction, bool offOptimization) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    llvm::ExitOnError ExitOnErr;
    ExitOnErr.setBanner("Error JIT: ");

    void* handle = nullptr;
    std::string lib_path_primary_attempt = loader::findLibraryPath();
    
    if (!lib_path_primary_attempt.empty() && lib_path_primary_attempt != "libm_std.so") 
        handle = dlopen(lib_path_primary_attempt.c_str(), RTLD_NOW | RTLD_GLOBAL);
    
    if (!handle) handle = dlopen("libm_std.so", RTLD_NOW | RTLD_GLOBAL);
    
    
    if (!handle) {
        ui::pipelineStop();
        std::cerr << "Error loading standard library (libm_std.so): " << dlerror() << std::endl;
        return RUN_FAILED;
    }

    // JIT
    auto JIT = ExitOnErr(llvm::orc::LLJITBuilder().create());

    auto& jitDylib = JIT->getMainJITDylib();
    jitDylib.addGenerator(
        llvm::cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            JIT->getDataLayout().getGlobalPrefix()))
    );

#if DEBUG
    std::cout << "\n--- LLVM IR ---" << std::endl;
    module->print(llvm::outs(), nullptr);
    std::cout << "--- END LLVM IR ---\n" << std::endl;
#endif

    ui::stage(offOptimization ? "Optimizer: skipped" : "Optimizer: running LLVM O2 passes");
    if(!offOptimization)
    {
        llvm::PassBuilder passBuilder;
        llvm::LoopAnalysisManager LAM;
        llvm::FunctionAnalysisManager FAM;
        llvm::CGSCCAnalysisManager CGAM;
        llvm::ModuleAnalysisManager MAM;
    
        // АНАЛизаторы
        passBuilder.registerModuleAnalyses(MAM); // АНАЛизирует на уровне всего модуля(llvm::Module)
        passBuilder.registerCGSCCAnalyses(CGAM); // АНАЛизирует на вызовы функций(llvm::CallInst)
        passBuilder.registerFunctionAnalyses(FAM); // АНАЛизирует на уровне функций(llvm::Function)
        passBuilder.registerLoopAnalyses(LAM); // АНАЛизирует на уровне циклов
        passBuilder.crossRegisterProxies(LAM, FAM, CGAM, MAM); // Связываем анализаторы друг с другом
    
        llvm::ModulePassManager MPM = passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);
    
        // Прогоняем *оптимизейшен*
        MPM.run(*module, MAM);
    }

#if DEBUG
    std::cout << "\n--- LLVM IR(O2) ---" << std::endl;
    module->print(llvm::outs(), nullptr);
    std::cout << "--- END LLVM IR(O2) ---\n" << std::endl;
#endif

    ui::stage("JIT: compiling to machine code");

    // ThreadSafeModule
    llvm::orc::ThreadSafeModule TSM(std::move(module), std::move(Ctx));
    ExitOnErr(JIT->addIRModule(std::move(TSM)));

    auto MainSymbol = ExitOnErr(JIT->lookup(mainFunction));
    auto MainFn = MainSymbol.toPtr<int(*)()>();

    // Анимация заканчивается до старта: дальше экран принадлежит программе
    ui::pipelineStop();

    // Вызываем функцию
    #if DEBUG
        std::cout << "Executing " << mainFunction << "()...\n";
    #endif
    int Result = MainFn();
    #if DEBUG
        std::cout << "Function " << mainFunction << "() executed.\n";
    #endif

    return Result;
}

int runProgram(std::shared_ptr<ProgramNode> combinedAST, const std::string& currentFilePath, bool showAST, bool offOptimization) {
#if DEBUG
    auto t_start = std::chrono::high_resolution_clock::now();
#endif
    // Code generation
    ui::stage("Code generation: building LLVM IR");
    auto llvmContext = std::make_unique<llvm::LLVMContext>();
    std::unique_ptr<llvm::Module> module;
    {
        CodeGenContext context(currentFilePath, llvmContext.get());
        ASTGen codeGen(context);
        combinedAST->accept(codeGen);
        module = std::move(context.TheModule);
    }

#if DEBUG
    auto t_codegen = std::chrono::high_resolution_clock::now();

    std::cout << "\n--- Запуск программы ---" << std::endl;
#endif

    try {
        
        if (module != nullptr) {
            const std::string entryFunctionName = CodeGenContext::findEntryName(*combinedAST);
            
#if DEBUG
            auto t_jit_start = std::chrono::high_resolution_clock::now();
#endif
            if (!module->getFunction("main")) {
                ui::pipelineStop();
                std::cerr << "Error: entry function not found: " << entryFunctionName << ", add main or mark a function with @entry" << std::endl;
                return RUN_FAILED;
            }

            int result = executeModule(std::move(module), std::move(llvmContext), "main", offOptimization);
#if DEBUG
            auto t_jit_end = std::chrono::high_resolution_clock::now();
#endif
            if (result == RUN_FAILED) return RUN_FAILED;

            std::cout << ui::style(stdout, result == 0 ? ui::DIM : ui::ACCENT) << "[" << entryFunctionName << " exited with code " << result << "]"
                      << ui::style(stdout, ui::RESET) << std::endl;

#if DEBUG
            // Статистика по времени
            auto ms_codegen = std::chrono::duration_cast<std::chrono::milliseconds>(t_codegen - t_start).count();
            auto ms_jit = std::chrono::duration_cast<std::chrono::milliseconds>(t_jit_end - t_jit_start).count();
            auto ms_total = std::chrono::duration_cast<std::chrono::milliseconds>(t_jit_end - t_start).count();

            std::cout << "\n--- Тайминги ---" << std::endl;
            std::cout << "Генерация IR:      " << ms_codegen << " мс" << std::endl;
            std::cout << "JIT + исполнение:  " << ms_jit << " мс" << std::endl;
            std::cout << "Всего:             " << ms_total << " мс" << std::endl;
            std::cout << "--- Конец таймингов ---\n" << std::endl;
#endif
            
            return result;
        } else {
            std::cerr << "Error: could not get the LLVM module to run." << std::endl;
            return RUN_FAILED;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error while running: " << e.what() << std::endl;
        return RUN_FAILED;
    }
}