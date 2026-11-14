#include "headers/cli.h"
#include "headers/ui.h"
#include <filesystem>
#include <iostream>
#include <fstream>

void printHelp(const char* programName) {
    const std::string name = std::filesystem::path(programName).filename().string();
    const std::string bold = ui::style(stdout, ui::WHITE) + ui::style(stdout, ui::BOLD), accent = ui::style(stdout, ui::ACCENT), dim = ui::style(stdout, ui::DIM), reset = ui::style(stdout, ui::RESET);

    auto option = [&](const std::string& flag, const std::string& text) {
        std::cout << "  " << accent << flag << reset << std::string(flag.size() < 24 ? 24 - flag.size() : 1, ' ') << text << "\n";
    };

    ui::printBanner(stdout);
    std::cout << "\n" << bold << "USAGE" << reset << "\n"
              << "  " << name << " [OPTIONS] [FILE]\n\n"
              << bold << "RUN" << reset << "\n";
    option("run FILE", "Run a program through the JIT");
    option("--compile OUT FILE", "Build a native executable");
    option("--offOptimization", "Run or build without LLVM optimization");
    option("--timings", "Show how long each compiler stage took");
    std::cout << "\n" << bold << "INSPECT" << reset << "\n";
    option("--tokens", "Show tokens");
    option("--ast", "Show AST");
    option("--semantic", "Show semantic analysis");
    std::cout << "\n" << bold << "TEST" << reset << "\n";
    option("--run_tests [FILTER]", "Run the test suite, FILTER selects tests by path");
    option("--verbose", "Show every test");
    std::cout << "\n" << dim << "  Without FILE: an interactive session in a terminal, otherwise the source is read from standard input." << reset << "\n" << std::endl;
}

CLIOptions parseArgs(int argc, char* argv[]) {
    CLIOptions options;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printHelp(argv[0]);
            exit(0);
        } else if (arg == "--tokens") {
            options.showTokens = true;
        } else if (arg == "--ast") {
            options.showAST = true;
        } else if (arg == "--semantic" || arg == "--symantic") {
            options.showSymantic = true;
        } else if (arg == "--run" || arg == "run") {
            options.runJIT = true;
        } else if (arg == "--compile") {
            options.compileExecutable = true;
            if (i + 1 < argc) {
                options.ExecutableFile = argv[++i];
            } else {
                std::cerr << "Error: --compile requires an output file name." << std::endl;
                exit(1);
            }
        } else if (arg == "--offOptimization") {
            options.offOptimization = true;
        } else if (arg == "--timings") {
            options.showTimings = true;
        } else if (arg == "--run_tests") {
            options.runTests = true;
        } else if (arg == "--verbose") {
            options.verbose = true;
        } else if (arg[0] != '-') {
            options.inputFile = arg;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            exit(1);
        }
    }
    
    if (!options.showTokens && !options.showAST && !options.showSymantic) {
        options.showTokens = options.showAST = options.showSymantic = false;
    }
    
    return options;
}

std::string readSourceCode(const std::string& inputFile) {
    std::string sourceCode;
    ui::stage("Reading the source file");
    
    if (!inputFile.empty()) {
        std::ifstream file(inputFile);
        if (!file) {
            std::cerr << "Error: could not open file " << inputFile << std::endl;
            exit(1);
        }
        std::string line;
        while (std::getline(file, line)) {
            sourceCode += line + "\n";
        }
    } else {
        std::string line;
        while (std::getline(std::cin, line)) {
            sourceCode += line + "\n";
        }
    }
    
    return sourceCode;
}