// Раннер тестов без LLVM, см. src/CLI/test_runner.cpp
#include "../src/CLI/headers/test_runner.h"
#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: run_tests CHECK_TOOL TESTS_DIR [--verbose] [FILTER]" << std::endl;
        return 2;
    }

    bool verbose = false;
    std::string filter;
    for (int i = 3; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--verbose") verbose = true;
        else filter = argv[i];
    }

    return runFrontendTests({std::filesystem::absolute(argv[1]).string()}, argv[2], verbose, filter);
}
