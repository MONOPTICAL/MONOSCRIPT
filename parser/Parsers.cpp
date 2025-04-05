#include "headers/Parser.h"
/*
Ну а тут уже парсеры для всех действий в языке
*/

std::shared_ptr<FunctionNode> Parser::parseFunction()
{
    /*
    Function declaration:
    [array<i32>]bubbleSort(array<i32>: arr)
    [Type]functionName([Type]: variableName ...)
    */
    consume(TokenType::LeftBracket, "Expected '[' before function declaration"); // Проверяем наличие левой скобки

    std::string returnType = getFullType(); // Получаем полный тип функции
    consume(TokenType::RightBracket, "Expected ']' before function declaration"); 
    std::string functionName = current().value; // Сохраняем имя функции
    consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора

    if (check(TokenType::LeftParen)) // Если следующий токен - это левая скобка, то это функция с параметрами
    {
        advance(); // Переходим к следующему токену
        std::vector<std::pair<std::string, std::string>> parameters; // Вектор параметров функции

        if (!check(TokenType::RightParen)) // Если следующий токен - это не правая скобка, то это функция с параметрами
        {
            do
            {
                std::string paramType = getFullType(); // Получаем полный тип параметра функции
                consume(TokenType::Colon, "Expected ':' after parameter type"); // Проверяем наличие двоеточия после типа параметра функции
                std::string paramName = current().value; // Сохраняем имя параметра функции
                consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора параметра функции

                parameters.push_back({paramType, paramName}); // Добавляем параметр в вектор параметров функции
            } while (match(TokenType::Comma)); // Пока следующий токен - это запятая, продолжаем добавлять параметры функции
        }

        consume(TokenType::RightParen, "Expected ')' after function parameters"); // Проверяем наличие правой скобки после параметров функции
        int expectedIndent = getIndentLevel(lines[lineIndex]) + 1; // Уровень отступа для блока if
        nextLine(); // Переходим к следующему токену
        auto body = parseBlock(expectedIndent); // Парсим тело функции
        return std::make_shared<FunctionNode>(functionName, returnType, parameters, body); // Создаём узел функции
    }
    else
    {
        throw std::runtime_error("Parser Error: Expected '(' after function name at line " + std::to_string(current().line) +
            ", column " + std::to_string(current().column) +
            ": " + current().value);
    }

    return nullptr; // Если ничего не найдено, возвращаем nullptr
}

std::shared_ptr<ASTNode> Parser::parseIf()
{
    // Проверяем, что текущий токен - это ключевое слово if
    consume(TokenType::Keyword, "Expected 'if' keyword"); 
    // Мы не делаем проверку здесь на наличие круглой скобки, так как это может быть выражение без скобок
    // И даже если это выражение без скобок, то оно всё равно может казаться как if (expression)
    // Вот воможные варианты:
    // if(2+2==4), if((2+2)==4), if 2+2==4, if (2+2)==4
    // Поэтому мы просто сразу посылаем его в parseExpression который разберёт его

    auto condition = parseExpression();
    int expectedIndent = getIndentLevel(lines[lineIndex]) + 1; // Уровень отступа для блока if
    nextLine(); // Переходим к следующей строке токенов 
    IC(expectedIndent, current().value, peek().value, lineIndex, tokenIndex);
    auto thenBlock = parseBlock(expectedIndent);
    std::shared_ptr<BlockNode> elseBlock = nullptr;
    if (check(TokenType::Keyword) && peek().value == "else")
    {
        advance(); // consume 'else'
        elseBlock = parseBlock(expectedIndent);
    }
    return std::make_shared<IfNode>(condition, thenBlock, elseBlock);
}

std::shared_ptr<ASTNode> Parser::parseFor()
{
    /*
    For statement:
    for [iteration variable] in [variable]
    for i in arr
    */
    consume(TokenType::Keyword, "Expected 'for' keyword"); // Проверяем наличие ключевого слова for

    std::string iterationVariable = current().value; // Сохраняем имя переменной итерации
    consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора

    if(current().type != TokenType::Keyword && current().value != "in")
    {
        throw std::runtime_error("Parser Error: Expected 'in' keyword at line " + std::to_string(current().line) +
            ", column " + std::to_string(current().column) +
            ": " + current().value);
    }
    consume(TokenType::Keyword, "Expected 'in' keyword"); // Проверяем наличие ключевого слова in

    auto variable = parseExpression(); // Парсим переменную для итерации

    int expectedIndent = getIndentLevel(lines[lineIndex]) + 1; // Уровень отступа для блока for
    nextLine(); // Переходим к следующему токену 
    IC(expectedIndent, current().value, peek().value, lineIndex, tokenIndex);
    auto body = parseBlock(expectedIndent); // Парсим тело цикла for

    return std::make_shared<ForNode>(iterationVariable, variable, body); // Создаём узел цикла for
}

std::shared_ptr<BlockNode> Parser::parseBlock(int expectedIndent)
{
    /*
    Как выглядят блоки в коде:
    |
    |   *code*
    |
    */
   auto block = std::make_shared<BlockNode>();

   while (!isEndOfFile()) {
        int actualIndent = getIndentLevel(lines[lineIndex]);
        if (actualIndent < expectedIndent) break;
        if (actualIndent > expectedIndent)
            throw std::runtime_error("Unexpected indentation at line " + std::to_string(lineIndex));

        IC(actualIndent, expectedIndent, current().value, peek().value, lineIndex, tokenIndex);
        // пропустить Pipe токены
        tokenIndex = actualIndent;

        auto stmt = parseStatement();  // не должен заниматься отступами
        if (stmt) block->statements.push_back(stmt);
        nextLine();
   }

   return block;
}

std::shared_ptr<ASTNode> Parser::parseReturn()
{
    /*
    Return statement:
    return [expression]
    return 0
    */
    consume(TokenType::Keyword, "Expected 'return' keyword"); // Проверяем наличие ключевого слова return
    advance(); // Переходим к следующему токену
    auto expression = parseExpression(); // Парсим выражение после return
    return std::make_shared<ReturnNode>(expression); // Создаём узел возврата
}

std::shared_ptr<ASTNode> Parser::parseExpression()
{
    return parseBinary();
}

std::shared_ptr<ASTNode> Parser::parseAssignment(bool isConst)
{
    /*
    Variable declaration or assignment:
    i32 i = 0 -- статическая переменная
    i ^= 0 -- динамическая переменная

    const i32 i = 0 -- константа с статической инициализацией
    final i ^= 0  -- константа с динамической инициализацией
    */                            
    std::string type;                                           // Переменная для хранения типа
    std::string variableName;                                   // Переменная для хранения имени переменной
    bool finalFlag = false;                                     // Флаг для проверки наличия ключевого слова final
    if(isConst)
    {                                     
        std::string keyWord = current().value;                  // Сохраняем текущее значение токена
        if (keyWord != "const" && keyWord != "final")           // Проверяем наличие ключевого слова const или final
        {
            throw std::runtime_error("Parser Error: Expected 'const' or 'final' at line " + std::to_string(current().line) +
                ", column " + std::to_string(current().column) +
                ": " + current().value);
        }
        keyWord != "const" ? finalFlag = true : finalFlag = false; // Если токен - это const, то finalFlag = false, иначе finalFlag = true
        advance();                                                 // Переходим к следующему токену
    }

    if(current().type == TokenType::Type && (current().value != "array" || current().value != "map"))                       // Проверяем наличие типа
    {
        type = current().value;                                 // Сохраняем тип переменной
        consume(TokenType::Type, "Expected type");              // Проверяем наличие типа
        variableName = current().value;                         // Сохраняем имя переменной
        consume(TokenType::Identifier, "Expected identifier");  // Проверяем наличие идентификатора
        if(!check(TokenType::Operator) || current().value != "=") // Проверяем наличие оператора присваивания
        {
            throw std::runtime_error("Parser Error: Expected '=' operator after static variable declaration at line " + std::to_string(current().line) +
                ", column " + std::to_string(current().column) +
                ": " + current().value);
        }
    }
    else if(current().type == TokenType::Identifier && peek().value == "^=") // Проверяем наличие идентификатора
    {
        type = "auto";
        variableName = current().value;                         // Сохраняем имя переменной
        consume(TokenType::Identifier, "Expected identifier");  // Проверяем наличие идентификатора
        if(!check(TokenType::Operator) || current().value != "^=") // Проверяем наличие оператора присваивания
        {
            throw std::runtime_error("Parser Error: Expected '^=' operator after static variable declaration at line " + std::to_string(current().line) +
                ", column " + std::to_string(current().column) +
                ": " + current().value);
        }
    }
    else
    {   // Тут обрабатываются случаи с динамической инициализацией переменной и с кастомным типом(например, array<i32>)
        // Все случаи:
        /*
        array<i32> i = 0
        map<string,i32> i = 0
        customType i = 0
        customeType<i32> i = 0
        Обрабатываются они вместе потому что в Lexer они обрабатываются как 
        {Type, "array"} {Operator, "<"} {Type, "i32"} {Operator, ">"} {Identifier, "i"}
        {Identifier, "customType"} {Operator, "<"} {Type, "i32"} {Operator, ">"} {Identifier, "i"}
        {Identifier, "customType"} {Identifier, "i"}
        И если посмотреть так образно то у них одинаковое окончание с Identifier и одинаковый оператор присваивания
        */
        type = getFullType(); // Получаем полный тип переменной                                       // Переменная имеет тип auto и когда во время runtime
        variableName = current().value;                         // Сохраняем имя переменной
        consume(TokenType::Identifier, "Expected identifier");  // Проверяем наличие идентификатора
        if(!check(TokenType::Operator) || current().value != "^=") // Проверяем наличие оператора присваивания
        {
            throw std::runtime_error("Parser Error: Expected '^=' operator after dynamic variable declaration at line " + std::to_string(current().line) +
                ", column " + std::to_string(current().column) +
                ": " + current().value);
        }
    }

    // Здесь мы уже у оператора присваивания, который идёт после имени переменной
    advance(); // Переходим к следующему токену
    //std::cout << "[parseAssignment] Current token: " << current().value << std::endl; // Выводим текущий токен в консоль
    std::shared_ptr<ASTNode> expression = parseExpression(); // Разбираем выражение справа от оператора присваивания
    
    if (!expression) // Если выражение не разобрано, выбрасываем исключение
    {
        throw std::runtime_error("Parser Error: Expected expression after assignment operator at line " + std::to_string(current().line) +
            ", column " + std::to_string(current().column) +
            ": " + current().value);
    }
    return std::make_shared<VariableAssignNode>(variableName, isConst, type ,expression); // Создаём узел присваивания переменной
}

std::shared_ptr<ASTNode> Parser::parseCall()
{
    /*
    Function call:
    [functionName]([expression], [expression], ...)
    bubbleSort(arr, 0, 10)
    */
    consume(TokenType::Identifier, "Expected function name"); // Проверяем наличие идентификатора функции
    advance(); // Переходим к следующему токену
    if (check(TokenType::LeftParen))
    {
        advance(); // Переходим к следующему токену
        
        std::vector<std::shared_ptr<ASTNode>> arguments; // Вектор аргументов функции
        if (!check(TokenType::RightParen))
        {
            do
            {
                arguments.push_back(parseExpression()); // Если аргумент разобран, добавляем его в вектор
            } while (match(TokenType::Comma));
        }

        consume(TokenType::RightParen, "Expected ')' after function arguments"); // Проверяем наличие правой скобки
        return std::make_shared<CallNode>(current().value, arguments); // Создаём узел вызова функции
    }

    return std::make_shared<IdentifierNode>(current().value); // Создаём узел идентификатора
}
