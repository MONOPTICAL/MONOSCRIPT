#include "../src/CLI/headers/test_runner.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>

namespace fs = std::filesystem;

int main()
{
    struct Case {
        const char* name;
        const char* output;
        int exitCode;
        bool shouldPass;
        const char* nativeOutput = nullptr;
    };
    const Case cases[] = {
        {"matching", "printf '%s\\n' first second\n", 0, true},
        {"missing", "", 0, false},
        {"wrong", "printf '%s\\n' first wrong\n", 0, false},
        {"reordered", "printf '%s\\n' second first\n", 0, false},
        {"failed", "printf '%s\\n' first second\n", 1, false},
        {"program_exit", "printf '%s\\n' first second '[main exited with code 7]'\n", 7, true},
        {"native_missing", "printf '%s\\n' first second\n", 0, false, ""},
        {"native_wrong", "printf '%s\\n' first second\n", 0, false, "printf '%s\\n' first wrong\n"},
        {"native_reordered", "printf '%s\\n' first second\n", 0, false, "printf '%s\\n' second first\n"},
    };

    // The fake compiler also acts as its own native output, exercising both runner paths without LLVM.
    const fs::path root = fs::temp_directory_path() / ("ms_runner_checks_" + std::to_string(getpid()));
    if (!fs::create_directory(root)) return 1;
    struct Cleanup {
        fs::path path;
        ~Cleanup() { std::error_code ignored; fs::remove_all(path, ignored); }
    } cleanup{root};

    int failed = 0;
    for (const auto& test : cases) {
        const fs::path directory = root / test.name;
        fs::create_directory(directory);
        std::ofstream(directory / "case.ms") << "// expect-output: first\n// expect-output: second\n";
        const fs::path compiler = directory / "compiler";
        {
            std::ofstream script(compiler);
            script << "#!/bin/sh\n"
                      "if [ \"$1\" = \"--offOptimization\" ]; then shift; fi\n"
                      "if [ \"$1\" = \"--compile\" ]; then\n"
                      "  cp \"$0\" \"$2\" && chmod +x \"$2\"\n"
                      "  exit $?\n"
                      "fi\n"
                      "if [ \"$#\" -eq 0 ]; then\n"
                   << (test.nativeOutput ? test.nativeOutput : test.output)
                   << "exit " << test.exitCode << "\nfi\n"
                   << test.output << "exit " << test.exitCode << "\n";
        }
        fs::permissions(compiler, fs::perms::owner_all);
        const bool passed = runFrontendTests({compiler.string()}, {compiler.string(), "run"}, directory.string(), false, "") == 0;
        if (passed != test.shouldPass) {
            std::cerr << "Unexpected test-runner result: " << test.name << "\n";
            ++failed;
        }
    }
    return failed == 0 ? 0 : 1;
}
