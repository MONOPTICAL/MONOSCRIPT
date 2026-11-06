#include "headers/runner.h"
#include "../runtime/headers/CodeGenContext.h"
#include "../runtime/headers/ASTVisitors.h"
#include "../loader/headers/loader.h"
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <cstdio>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Target/TargetMachine.h>

bool emitExecutable(llvm::Module* module, const std::string& outputPath) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // LLVM IR -> .ll
    std::error_code EC;
    std::string irPath = outputPath + ".ll";
    llvm::raw_fd_ostream irFile(irPath, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        std::cerr << "Не удалось открыть файл для IR кода: " << EC.message() << std::endl;
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
    std::string llcCmd = llcTool + " -filetype=obj -relocation-model=pic -o " + objPath + " " + irPath;
    
    std::cout << "Запуск команды: " << llcCmd << std::endl;
    int llcRet = std::system(llcCmd.c_str());
    if (llcRet != 0) {
        std::cerr << "Ошибка при выполнении llc: команда вернула код " << llcRet << std::endl;
        return false;
    }

    std::string stdlibPath = loader::findLibraryPath();
    if (stdlibPath.empty()) {
        std::cerr << "Не удалось найти стандартную библиотеку" << std::endl;
        return false;
    }

    std::cout << "Стандартная библиотека: " << stdlibPath << std::endl;

    std::string stdlibDir = "";
    size_t lastSlash = stdlibPath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        stdlibDir = stdlibPath.substr(0, lastSlash);
    }
    
    // Компилируем и линкуем
    std::string gccCmd = "gcc -g -o " + outputPath + " " + objPath;
    
    // mono
    gccCmd += " " + stdlibPath + " -lgc -lm -ldl -Wl,-rpath," + stdlibDir;
    
    std::cout << "Запуск команды: " << gccCmd << std::endl;
    int gccRet = std::system(gccCmd.c_str());
    if (gccRet != 0) {
        std::cerr << "Ошибка при линковке с помощью gcc: команда вернула код " << gccRet << std::endl;
        return false;
    }
    
    // Удаляем промежуточные файлы
    std::remove(irPath.c_str());
    std::remove(objPath.c_str());
    
    return true;
}

bool compileToExecutable(std::shared_ptr<ProgramNode> combinedAST, const std::string& outputFile) {
    CodeGenContext context(outputFile);
    ASTGen codeGen(context);
    combinedAST->accept(codeGen);

    if (context.TheModule != nullptr) {
#if DEBUG
        std::cerr << "\n--- LLVM IR ---" << std::endl;
        context.TheModule->print(llvm::outs(), nullptr);
        std::cerr << "--- END LLVM IR ---\n" << std::endl;
#endif
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
            MPM.run(*context.TheModule, MAM);
        }
#if DEBUG
        std::cerr << "\n--- LLVM IR(O2) ---" << std::endl;
        context.TheModule->print(llvm::outs(), nullptr);
        std::cerr << "--- END LLVM IR(O2) ---\n" << std::endl;
#endif
        if (!context.TheModule->getFunction("main")) {
            std::cerr << "Error: entry function not found: " << CodeGenContext::findEntryName(*combinedAST) << ", add main or mark a function with @entry" << std::endl;
            return false;
        }
        if (!emitExecutable(context.TheModule.get(), outputFile)) {
            std::cerr << "Ошибка при компиляции в исполняемый файл." << std::endl;
            return false;
        }
        return true;
    } else {
        std::cerr << "Ошибка: Не удалось получить модуль LLVM для компиляции." << std::endl;
        return false;
    }
}