#include "headers/cli.h"
#include <iostream>
#include <fstream>

void printHelp(const char* programName) {
    std::cout << "🤔 Usage: " << programName << " [OPTIONS] [FILE]\n\n"
              << "⚙️ Options:\n"
              << "  --help           ❓ Show this help message and exit\n"
              << "  --tokens         🏷️  Show tokens\n"
              << "  --ast            🌳 Show AST\n"
              << "  --symantic       🔍 Show symantic analysis\n"
              << "  --run, run       ▶️  Execute the program via LLVM\n"
              << "\n⌨️ If FILE is not specified, input is read from standard input.\n"
              << std::endl;
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
        } else if (arg == "--symantic") {
            options.showSymantic = true;
        } else if (arg == "--run" || arg == "run") {
            options.run = true;
        } else if (arg[0] != '-') {
            options.inputFile = arg;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            exit(1);
        }
    }
    
    if (!options.showTokens && !options.showAST && !options.showSymantic) {
        options.showTokens = options.showAST = options.showSymantic = true;
    }
    
    return options;
}

std::string readSourceCode(const std::string& inputFile) {
    std::string sourceCode;
    
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
        std::cout << "Enter source code (Ctrl+D to finish):\n";
        std::string line;
        while (std::getline(std::cin, line)) {
            sourceCode += line + "\n";
        }
    }
    
    return sourceCode;
}