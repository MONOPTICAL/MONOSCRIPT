#include "headers/Parser.h"

/*
Ну а это уже самый главный парсер, который парсит распредиляет линии по разным handler'ам

*/

/// @brief Парсит программу
/// @details Создает узел программы и добавляет в него все узлы, которые были распознаны в процессе парсинга
/// @return Указатель на узел программы, содержащий все узлы, которые были распознаны в процессе парсинга
std::shared_ptr<ProgramNode> Parser::parse()
{
    auto program = std::make_shared<ProgramNode>();
    while (!isEndOfFile())
    {
        if (check(TokenType::None)) break; // конец файла
        auto statement = parseStatement();
        if (!statement)
        {
            std::runtime_error("Parser Error: Expected statement at line " + std::to_string(current().line) +
                ", column " + std::to_string(current().column) +
                ": " + current().value);
        }
        if (statement) program->body.push_back(statement);
        if (!nextLine()) break; // если не удалось перейти к следующей строке, выходим из цикла
    }
    return program;
}

/// @brief Парсит оператор
/// @details Разбирает оператор в зависимости от его типа. Если оператор не распознан, выбрасывает исключение.
/// @return Указатель на узел AST, представляющий разобранный оператор.
std::shared_ptr<ASTNode> Parser::parseStatement()
{
    Token currentToken = current();
    // Function declaration
        // [Type]functionName([Type]: variableName ...)
        // [array<i32>]bubbleSort(array<i32>: arr)
        // ↑ if we have left bracket, we have function declaration
    if (currentToken.type == TokenType::LeftBracket)
    {
        return parseFunction();
    }
    // If statement
        // if [expression] or if [expression]
        // if 2 > 0
        // ↑ if we have keyword if, we have if statement
    else if (currentToken.type == TokenType::Keyword && currentToken.value == "if")
    {
        return parseIf();
    }
    // For statement
        // for [iteration variable] in [variable]
        // for i in arr
        // ↑ if we have keyword for, we have for statement
    else if (currentToken.type == TokenType::Keyword && currentToken.value == "for")
    {
        return parseFor();
    }
    // Return statement
        // return [expression]
        // return 0
        // ↑ if we have keyword return, we have return statement    
    else if (currentToken.type == TokenType::Keyword && currentToken.value == "return")
    {
        return parseReturn();
    }
    // Dynamic variable declaration or assignment
        // [variableName] = [expression] or [variableName] ^= [expression]
        // i = 0 or i ^= 0
        // ↑ if we have identifier, and next token is operator, we have dynamic variable declaration or assignment
    else if (currentToken.type == TokenType::Identifier && peek().type == TokenType::Operator)
    {
        return parseAssignment(false);
    }
    // Static variable declaration
        // [Type] [variableName] = [expression] or [variableName] ^= [expression]
        // i32 i = 0 or i ^= 0
        // ↑ if we have type, and next token is identifier, we have static variable declaration
    else if (currentToken.type == TokenType::Type) //&& peek().type == TokenType::Identifier)
    {
        return parseAssignment(false);
    }
    // Const variable declaration
        // const [Type] [variableName] = [expression] or [variableName] ^= [expression]
        // const i32 i = 0 or i ^= 0
        // ↑ if we have keyword const, and next token is type, we have const variable declaration
    else if (currentToken.type == TokenType::Keyword && currentToken.value == "const" && peek().type == TokenType::Type)
    {
        return parseAssignment(true);
    }
    // Final variable declaration
        // final [Type] [variableName] = [expression] or [variableName] ^= [expression]
        // final i32 i = 0 or i ^= 0
        // ↑ if we have keyword final, and next token is type, we have final variable declaration
    else if(currentToken.type == TokenType::Keyword && currentToken.value == "final" && peek().type == TokenType::Identifier)
    {
        return parseAssignment(true);
    }
    //else if((currentToken.type == TokenType::Type || currentToken.type == TokenType::Operator) && )
    // Function call
        // [functionName]([expression], [expression], ...)
        // bubbleSort(arr, 0, 10)
        // ↑ if we have identifier, and next token is left parenthesis, we have function call
    else if (currentToken.type == TokenType::Identifier && peek().type == TokenType::LeftParen)
    {
        return parseCall();
    }
    else if (currentToken.type == TokenType::LeftParen)
    {
        return parseExpression();
    }
    else
    {
        std::runtime_error("Parser Error: Unknown statement at line " + std::to_string(currentToken.line) +
            ", column " + std::to_string(currentToken.column) +
            ": " + currentToken.value);
    }
    return nullptr; // Если ничего не найдено, возвращаем nullptr
}
