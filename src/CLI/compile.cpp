#include "headers/runner.h"
#include "../runtime/headers/CodeGenContext.h"
#include "../runtime/headers/ASTVisitors.h"
#include "../loader/headers/loader.h"
#include "headers/ui.h"
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <cstdio>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/Program.h>

bool emitExecutable(llvm::Module* module, const std::string& outputPath) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // LLVM IR -> .ll
    ui::stage("Writing LLVM IR to a file");
    std::error_code EC;
    std::string irPath = outputPath + ".ll";
    llvm::raw_fd_ostream irFile(irPath, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        std::cerr << "Could not open the IR file: " << EC.message() << std::endl;
        return false;
    }
    
    module->print(irFile, nullptr);
    irFile.close();

    std::string objPath = outputPath + ".o";
    // llc может лежать не в PATH, а рядом с LLVM, с которым собран компилятор
    std::string llcTool = "llc";
#ifdef LLVM_TOOLS_PATH
    if (std::ifstream(std::string(LLVM_TOOLS_PATH) + "/llc").good())
        llcTool = std::string(LLVM_TOOLS_PATH) + "/llc";
#endif
    // pic нужен, потому что gcc по умолчанию собирает PIE, а без него не линкуются строки
    ui::stage("llc: generating machine code");
    auto llc = llvm::sys::findProgramByName(llcTool);
    if (!llc) {
        std::cerr << "Could not find llc: " << llc.getError().message() << std::endl;
        return false;
    }
    int llcRet = llvm::sys::ExecuteAndWait(*llc,
        {*llc, "-filetype=obj", "-relocation-model=pic", "-o", objPath, irPath});
    if (llcRet != 0) {
        ui::pipelineStop();
        std::cerr << "llc failed with code " << llcRet << std::endl;
        return false;
    }

    std::string stdlibPath = loader::findLibraryPath();
    if (stdlibPath.empty()) {
        std::cerr << "Could not find the standard library" << std::endl;
        return false;
    }

#if DEBUG
    std::cout << "Стандартная библиотека: " << stdlibPath << std::endl;
#endif

    std::string stdlibDir = "";
    size_t lastSlash = stdlibPath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        stdlibDir = stdlibPath.substr(0, lastSlash);
    }
    
    // Компилируем и линкуем
    auto gcc = llvm::sys::findProgramByName("gcc");
    if (!gcc) {
        std::cerr << "Could not find gcc: " << gcc.getError().message() << std::endl;
        return false;
    }
    const std::string rpath = "-Wl,-rpath," + stdlibDir;
    ui::stage("gcc: linking the executable");
    int gccRet = llvm::sys::ExecuteAndWait(*gcc,
        {*gcc, "-g", "-o", outputPath, objPath, stdlibPath, "-lgc", "-lm", "-ldl", rpath});
    if (gccRet != 0) {
        ui::pipelineStop();
        std::cerr << "gcc failed with code " << gccRet << std::endl;
        return false;
    }
    
    // Удаляем промежуточные файлы
    std::remove(irPath.c_str());
    std::remove(objPath.c_str());
    
    return true;
}

bool compileToExecutable(std::shared_ptr<ProgramNode> combinedAST, const std::string& outputFile, bool offOptimization) {
    CodeGenContext context(outputFile);
    ASTGen codeGen(context);
    ui::stage("Code generation: building LLVM IR");
    combinedAST->accept(codeGen);

    if (context.TheModule != nullptr) {
#if DEBUG
        std::cerr << "\n--- LLVM IR ---" << std::endl;
        context.TheModule->print(llvm::outs(), nullptr);
        std::cerr << "--- END LLVM IR ---\n" << std::endl;
#endif
        // Без оптимизаций тесты ловят то, что O2 случайно маскирует: так прятался баг с раскладкой структур
        if (offOptimization) ui::stage("Optimizer: skipped");
        else {
            ui::stage("Optimizer: running LLVM O2 passes");
            llvm::PassBuilder passBuilder(&CodeGenContext::targetMachine()); // без машины цели O2 не знает стоимость инструкций и почти не векторизует
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
            MPM.run(*context.TheModule, MAM);
        }
#if DEBUG
        std::cerr << "\n--- LLVM IR(O2) ---" << std::endl;
        context.TheModule->print(llvm::outs(), nullptr);
        std::cerr << "--- END LLVM IR(O2) ---\n" << std::endl;
#endif
        if (!context.TheModule->getFunction("main")) {
            ui::pipelineStop();
            std::cerr << "Error: entry function not found: " << CodeGenContext::findEntryName(*combinedAST) << ", add main or mark a function with @entry" << std::endl;
            return false;
        }
        if (!emitExecutable(context.TheModule.get(), outputFile)) {
            std::cerr << "Could not build the executable." << std::endl;
            return false;
        }
        return true;
    } else {
        std::cerr << "Error: could not get the LLVM module to compile." << std::endl;
        return false;
    }
}