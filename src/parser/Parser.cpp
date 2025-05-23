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
    program->line = lineIndex; program->column = tokenIndex;
    program->moduleName = moduleName;
    
    while (!isEndOfFile())
    {
        if (check(TokenType::None)) break; // конец файла
        int currentLine = lineIndex;
        auto statement = parseStatement();
        if (!statement)
        {
            std::runtime_error("Parser Error: Expected statement at line " + std::to_string(current().line) +
                ", column " + std::to_string(current().column) +
                ": " + current().value);
        }
        if (statement) program->body.push_back(statement);
        if (!isEndOfLine() && (currentLine == lineIndex)) throwError("Character(" + current().value + ") not parsed at the end of");
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
    if (currentToken.type == TokenType::LeftBracket && (getLastTokenInCurrentLine().type == TokenType::RightParen || getLastTokenInCurrentLine().type == TokenType::Label))
    {
        return parseFunction();
        
    }
    // Use statement
        // use 
        // |> std -> io : alias
        // ↑ if we have keyword use, we have use statement
    else if (currentToken.type == TokenType::Keyword && currentToken.value == "use")
    {
        return parseUse();
    }
    // Struct declaration
        // [struct] User
        // |> string name = "name"
        // |> i32 age = 42
        // |> bool isActive = true
        // ↑ if we have left bracket and struct keyword, we have struct declaration
    else if (currentToken.type == TokenType::LeftBracket && peek().value == "struct")
    {
        return parseStruct();
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
    else if (currentToken.type == TokenType::Keyword && currentToken.value == "while")
    {
        return parseWhile();
    }
    // Return statement
        // return [expression]
        // return 0
        // ↑ if we have keyword return, we have return statement    
    else if (currentToken.type == TokenType::Keyword && currentToken.value == "return")
    {
        return parseReturn();
    }
    // Break statement
        // break
        // ↑ if we have keyword break, we have break statement
    else if (currentToken.type == TokenType::Keyword && currentToken.value == "break")
    {
        auto breakNode = std::make_shared<BreakNode>(); // А тут и в continue нехуй заморачиваться, правильно? правильно
        breakNode->line = lineIndex; breakNode->column = tokenIndex;
        return breakNode;
    }
    // Continue statement
        // continue
        // ↑ if we have keyword continue, we have continue statement
    else if (currentToken.type == TokenType::Keyword && currentToken.value == "continue")
    {
        auto continueNode = std::make_shared<ContinueNode>();
        continueNode->line = lineIndex; continueNode->column = tokenIndex;
        return continueNode;
    }
    else if (currentToken.type == TokenType::Identifier && (peek().type == TokenType::Dot || peek().type == TokenType::LeftBracket))
    {
        auto memberExpression = parseMemberExpression();

        if(!isEndOfLine() && check(TokenType::Operator))
        {
            if(current().value == "=")
            {
                advance();
                auto reassignNode = std::make_shared<ReassignMemberNode>();
                reassignNode->line = lineIndex; reassignNode->column = tokenIndex;
                reassignNode->accessExpression = memberExpression;
                reassignNode->expression = parseExpression();

                return reassignNode;
            }
            else
            {
                throwError("Expected = symbol when assigning value to member expression");
            }
        }

        return memberExpression;
    }
    // Dynamic variable declaration or assignment
        // [variableName] = [expression] or [variableName] ^= [expression]
        // i = 0 or i ^= 0
        // ↑ if we have identifier, and next token is operator and is not dot(Dot annotation), we have dynamic variable declaration or assignment
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
    // убрать это нахуй, какой бля нормальный человек начнёт писать строку с (
    else if (currentToken.type == TokenType::LeftParen)
    {
        return parseExpression();
    }
    else
    {
        if(currentToken.line != -1)
            throwError("Unknown statement");
    }
    return nullptr;
}