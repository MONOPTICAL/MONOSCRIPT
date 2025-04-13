#include "lexer/headers/Lexer.h"
#include "lexer/headers/Token.h"
#include "parser/headers/Parser.h"
#include "includes/ASTDebugger.hpp"

// g++ main.cpp lexer/Lexer.cpp lexer/ParsingFunctions.cpp parser/Logic.cpp parser/Parser.cpp parser/Parsers.cpp -o main -std=c++20
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

    try {
        Lexer lexer(sourceCode);
        lexer.tokenize();
        const std::vector<std::vector<Token>> X = lexer.getTokens();
    
        Parser parser(X);
        std::shared_ptr<ProgramNode> program = parser.parse();
        if (program) {
            ASTDebugger::debug(program);
        } else {
            std::cerr << "Failed to parse the source code." << std::endl;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    

    
    return 0;
}