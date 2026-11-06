// Проверка одного файла без LLVM, см. src/CLI/test_runner.cpp
#include "../src/CLI/headers/test_runner.h"

int main(int argc, char* argv[])
{
    return checkMain(argc, argv);
}
