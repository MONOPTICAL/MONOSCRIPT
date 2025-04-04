#include "headers/Lexer.h"
#include "headers/Token.h"
#include "iostream"

int main()
{
    std::string sourceCode;
    std::string line;

    std::cout << "Enter source code (end with Ctrl+Z on Windows or Ctrl+D on Linux/Mac):" << std::endl;

    // Считываем строки до конца ввода
    while (std::getline(std::cin, line))
    {
        sourceCode += line + "\n";
    }

    std::cout << "Source code:" << std::endl << sourceCode << std::endl;

    Lexer lexer(sourceCode);
    lexer.tokenize();
    lexer.getTokens();
    

    return 0;
}