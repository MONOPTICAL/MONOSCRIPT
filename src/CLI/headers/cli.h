#pragma once
#include <string>

struct CLIOptions {
    bool showTokens = false;
    bool showAST = false;
    bool showSymantic = false;
    bool run = false; // LLVM
    std::string inputFile;
};

void printHelp(const char* programName);
CLIOptions parseArgs(int argc, char* argv[]);
std::string readSourceCode(const std::string& inputFile);