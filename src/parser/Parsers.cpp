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

    std::shared_ptr<TypeNode> returnType = getFullType(); // Получаем полный тип функции
    consume(TokenType::RightBracket, "Expected ']' before function declaration"); 
    std::string functionName = current().value; // Сохраняем имя функции
    consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора

    if (check(TokenType::LeftParen)) // Если следующий токен - это левая скобка, то это функция с параметрами
    {
        advance(); // Переходим к следующему токену
        std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> parameters; // Вектор параметров функции

        if (!check(TokenType::RightParen)) // Если следующий токен - это не правая скобка, то это функция с параметрами
        {
            do
            {
                std::shared_ptr<TypeNode> paramType = getFullType(); // Получаем полный тип параметра функции
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
                      // КОСТЫЛЬ АЛЕРТ 
        lineIndex--; // Без этого он скипает 2 линии а не одну так как в parse 
                    // После того как мы тута возвращаем и из-за этого он может проебать какие то значения
                   // Только уёбище ленивое перепиши это 
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
    //IC(current().value, peek().value, lineIndex, tokenIndex);
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
    //IC(expectedIndent, current().value, peek().value, lineIndex, tokenIndex);
    auto thenBlock = parseBlock(expectedIndent);
    std::shared_ptr<BlockNode> elseBlock = nullptr;
    tokenIndex = expectedIndent-2;
    //IC(current().value, peek().value, lineIndex, tokenIndex, expectedIndent);
    //if (tokenIndex!=(expectedIndent-1)) 

    if (check(TokenType::Pipe) && peek().value == "else")
    {
        nextLine(); // consume 'else'
        elseBlock = parseBlock(expectedIndent);
    }
                  // КОСТЫЛЬ АЛЕРТ
    lineIndex--; // Без этого он скипает 2 линии а не одну так как в parse 
                // После того как мы тута возвращаем и из-за этого он может проебать какие то значения
               // Только уёбище ленивое перепиши это 
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

    auto iterable = parseExpression(); // Парсим переменную для итерации

    int expectedIndent = getIndentLevel(lines[lineIndex]) + 1; // Уровень отступа для блока for
    nextLine(); // Переходим к следующему токену 
    //IC(expectedIndent, current().value, peek().value, lineIndex, tokenIndex);
    auto body = parseBlock(expectedIndent); // Парсим тело цикла for
                  // КОСТЫЛЬ АЛЕРТ 
    lineIndex--; // Без этого он скипает 2 линии а не одну так как в parse 
                // После того как мы тута возвращаем и из-за этого он может проебать какие то значения
               // Только уёбище ленивое перепиши это 
    return std::make_shared<ForNode>(iterationVariable, iterable, body); // Создаём узел цикла for
}

std::shared_ptr<ASTNode> Parser::parseWhile()
{
    /*
    While statement: 
    while (expression)
    |   {body}
    */
    consume(TokenType::Keyword, "Expected 'while' keyword"); // Проверяем наличие ключевого слова while
    auto condition = parseExpression();

    int expectedIndent = getIndentLevel(lines[lineIndex]) + 1;
    nextLine();

    auto body = parseBlock(expectedIndent);
                  // КОСТЫЛЬ АЛЕРТ 
    lineIndex--; // Без этого он скипает 2 линии а не одну так как в parse 
                // После того как мы тута возвращаем и из-за этого он может проебать какие то значения
               // Только уёбище ленивое перепиши это 

    return std::make_shared<WhileNode>(condition, body);
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
        //IC(current().value, peek().value, lineIndex, tokenIndex, actualIndent, expectedIndent);
        if (actualIndent < expectedIndent) break;
        if (actualIndent > expectedIndent)
            throw std::runtime_error("Unexpected indentation at line " + std::to_string(lineIndex) + " at " + current().value);

        //IC(actualIndent, expectedIndent, current().value, peek().value, lineIndex, tokenIndex);
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
    std::shared_ptr<TypeNode> type;                                           // Переменная для хранения типа
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

    if(current().type == TokenType::Type && (current().value != "array" && current().value != "map"))                       // Проверяем наличие типа
    {
        type = std::make_shared<SimpleTypeNode>(current().value);                                 // Сохраняем тип переменной
        consume(TokenType::Type, "Expected type");              // Проверяем наличие типа
        variableName = current().value;                         // Сохраняем имя переменной
        consume(TokenType::Identifier, "Expected identifier");  // Проверяем наличие идентификатора
        if(!check(TokenType::Operator) || current().value != "=") // Проверяем наличие оператора присваивания
        {   
            std::shared_ptr<NoneNode> none = std::make_shared<NoneNode>();
            return std::make_shared<VariableAssignNode>(variableName, isConst, type, none);
        }
    }
    else if(current().type == TokenType::Identifier && peek().value == "^=") // Проверяем наличие идентификатора
    {
        type = std::make_shared<SimpleTypeNode>("auto");
        variableName = current().value;                         // Сохраняем имя переменной
        consume(TokenType::Identifier, "Expected identifier");  // Проверяем наличие идентификатора
        if(!check(TokenType::Operator) || current().value != "^=") // Проверяем наличие оператора присваивания
        {
            throw std::runtime_error("Parser Error: Expected '^=' operator after static variable declaration at line " + std::to_string(current().line) +
                ", column " + std::to_string(current().column) +
                ": " + current().value);
        }
    }
    else if(current().type == TokenType::Identifier && peek().value == "=")
    {
        // ну тут тогда простой assignment
        variableName = current().value;                         // Сохраняем имя переменной
        consume(TokenType::Identifier, "Expected identifier");  // Проверяем наличие идентификатора
        consume(TokenType::Operator, "Expected = operator");
        auto expression = parseExpression();
        return std::make_shared<VariableReassignNode>(variableName, expression);
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
        type = getFullType(); // Получаем полный тип переменной               
        variableName = current().value;

        consume(TokenType::Identifier, "Expected identifier");  // Проверяем наличие идентификатора
        if(!check(TokenType::Operator) || current().value != "=") // Проверяем наличие оператора присваивания
        {
            std::shared_ptr<NoneNode> none = std::make_shared<NoneNode>();
            return std::make_shared<VariableAssignNode>(variableName, isConst, type, none);
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
    std::shared_ptr<CallNode> callNode = std::make_shared<CallNode>();
    callNode->callee = current().value;
    consume(TokenType::Identifier, "Expected function name"); // Проверяем наличие идентификатора функции
    consume(TokenType::LeftParen, "Expected '(' after function name");
    
    std::vector<std::shared_ptr<ASTNode>> arguments; // Вектор аргументов функции

    if (!check(TokenType::RightParen)) // Если после ( там нету сразу ) значит есть аргументы ёпта
    {
        do
        {
            arguments.push_back(parseExpression()); // Если аргумент разобран, добавляем его в вектор
        } while (match(TokenType::Comma));
    }

    consume(TokenType::RightParen, "Expected ')' after function arguments"); // Проверяем наличие правой скобки
    callNode->arguments = arguments;

    return callNode; // Создаём узел идентификатора
}

std::shared_ptr<ASTNode> Parser::parseStruct()
{
    /*
|   // Struct definition
|   [struct] User
|   | string name = "name"
|   | i32 age = 42
|   | bool isActive = true
    */
   
    consume(TokenType::LeftBracket, "Expected '[' before struct declaration"); // Проверяем наличие левой скобки
    consume(TokenType::Type, "Expected 'struct' keyword"); // Проверяем наличие ключевого слова struct
    consume(TokenType::RightBracket, "Expected ']' before struct declaration"); // Проверяем наличие левой скобки

    std::string name = current().value; // Сохраняем имя структуры
    consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора структуры
    
    int expectedIndent = getIndentLevel(lines[lineIndex]) + 1; // Уровень отступа для блока структуры
    nextLine(); // Переходим к следующему токену
    auto body = parseBlock(expectedIndent); // Парсим тело структуры
                  // КОСТЫЛЬ АЛЕРТ
    lineIndex--; // Без этого он скипает 2 линии а не одну так как в parse
                // После того как мы тута возвращаем и из-за этого он может проебать какие то значения
               // Только уёбище ленивое перепиши это    
    
    return std::make_shared<StructNode>(name, body); // Создаём узел структуры
}

std::shared_ptr<ASTNode> Parser::parseClass()
{
    consume(TokenType::LeftBracket, "Expected '[' before struct declaration"); // Проверяем наличие левой скобки
    consume(TokenType::Type, "Expected 'class' keyword"); // Проверяем наличие ключевого слова struct
    consume(TokenType::RightBracket, "Expected ']' before struct declaration"); // Проверяем наличие левой скобки

    std::string name = current().value; // Сохраняем имя структуры
    consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора структуры

    nextLine();


    std::shared_ptr<ASTNode> private_body; 
    std::shared_ptr<ASTNode> public_body;
    int expectedIndent = getIndentLevel(lines[lineIndex]) + 1; // Уровень отступа для блока структуры

    tokenIndex = expectedIndent - 1;
    Token currentAccess = current();

    // Первый AccessBlock(private/public)
    if(currentAccess.type == TokenType::Keyword && (currentAccess.value == "public" || currentAccess.value == "private")) // Проверяем наличие ключевого слова public
    {
        consume(TokenType::Keyword, "Expected access expression after class statement"); // Проверяем наличие ключевого слова public
        consume(TokenType::Colon, "Expected : after class access expression");
        nextLine();
        if(currentAccess.value == "public")
            public_body = parseBlock(expectedIndent);
        else
            private_body = parseBlock(expectedIndent);
    }
    else
    {
        throw std::runtime_error("Expected access expression after class statement");
    }

    // Второй AccessBlock(private/public)
    tokenIndex = expectedIndent - 1;
    currentAccess = current();

    if(currentAccess.type == TokenType::Keyword && (currentAccess.value == "public" || currentAccess.value == "private")) 
    {
        consume(TokenType::Keyword, "Expected keyword"); // Проверяем наличие ключевого слова public
        consume(TokenType::Colon, "Expected : after class access expression");
        nextLine();
        if(currentAccess.value == "public")
            public_body = parseBlock(expectedIndent);
        else if(currentAccess.value == "private")
            private_body = parseBlock(expectedIndent);
        else
            throw std::runtime_error("Expected access expression after class statement");
    }

    lineIndex--;
    return std::make_shared<ClassNode>(name, public_body, private_body); // Создаём узел структуры   
}