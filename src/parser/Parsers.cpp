#include "headers/Parser.h"
#include "../visitors/headers/TomlType.h"
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
    int line = lineIndex; int token = tokenIndex; // Запоминаем строку и токен
    consume(TokenType::LeftBracket, "Expected '[' before function declaration"); // Проверяем наличие левой скобки

    std::string association = "";

    std::shared_ptr<TypeNode> returnType = getFullType(); // Получаем полный тип функции
    consume(TokenType::RightBracket, "Expected ']' before function declaration"); 
    std::string functionName = current().value; // Сохраняем имя функции
    consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора

    // Шаблон: [T]first<T>(array<T>: a)
    std::vector<std::string> typeParameters;
    if (check(TokenType::Operator) && current().value == "<")
    {
        advance();
        do
        {
            typeParameters.push_back(current().value);
            consume(TokenType::Identifier, "Expected type parameter name");
        } while (match(TokenType::Comma));

        if (!check(TokenType::Operator) || current().value != ">")
            throwError("Expected > to close type parameters");
        advance();
    }

    // Пока разбираем сам шаблон, T это имя типа: x->T и T y = x должны разбираться
    const bool isTemplate = !typeParameters.empty() && typeSubstitution.empty();
    if (isTemplate)
        for (const auto& name : typeParameters) typeSubstitution[name] = name;

    if (check(TokenType::LeftParen)) // Если следующий токен - это левая скобка, то это функция с параметрами
    {
        advance(); // Переходим к следующему токену
        std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> parameters; // Вектор параметров функции
        int lambdaCounter = 0; // Счётчик лямбд

        if (!check(TokenType::RightParen)) // Если следующий токен - это не правая скобка, то это функция с параметрами
        {
            do
            {
                std::shared_ptr<TypeNode> paramType = getFullType(); // Получаем полный тип параметра функции
                consume(TokenType::Colon, "Expected ':' after parameter type"); // Проверяем наличие двоеточия после типа параметра функции
                if (paramType->toString() == "func") // Если тип параметра не определён, выбрасываем исключение
                {
                    /*
                    func: (i8: a) -> i8
                    Params: 
                        func<i8, i8> : "@0" - последний параметр это return type а все остальные это параметры функции
                    */
                    std::string paramName;
                    if (check(TokenType::Identifier))
                    {
                        paramName = current().value; // Сохраняем имя параметра функции 
                        consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора параметра функции
                    }
                    else
                        paramName = "@" + std::to_string(lambdaCounter++);

                    consume(TokenType::LeftParen, "Expected '(' after function type"); // Проверяем наличие левой скобки после типа параметра функции
                    std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> params;
                    if (!check(TokenType::RightParen)) do // func: f() -> i32 без параметров
                    {
                        std::shared_ptr<TypeNode> paramType = getFullType(); // Получаем полный тип параметра функции
                        consume(TokenType::Colon, "Expected ':' after parameter type"); // Проверяем наличие двоеточия после типа параметра функции
                        std::string paramName = current().value; // Сохраняем имя параметра функции
                        consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора параметра функции

                        params.push_back({paramType, paramName}); // Добавляем параметр в вектор параметров функции
                    } while (match(TokenType::Comma)); // Пока следующий токен - это запятая, продолжаем добавлять параметры функции
                    consume(TokenType::RightParen, "Expected ')' after parameters in lambda/function literal");
                    consume(TokenType::Arrow, "Expected '->' after parameters in lambda/function literal"); // Проверяем наличие стрелки после параметров функции
                    auto returnType = getFullType(); // Получаем полный тип возвращаемого значения функции

                    std::shared_ptr<GenericTypeNode> genericType = std::make_shared<GenericTypeNode>("func"); // Создаём указатель на тип функции
                    genericType->line = lineIndex; genericType->column = tokenIndex; // Устанавливаем строку и колонку для узла
                    for (const auto& param : params) // Для каждого параметра функции
                    {
                        genericType->typeParameters.push_back(param.first); // Добавляем тип параметра в параметры функции
                    }
                    genericType->typeParameters.push_back(returnType); // Добавляем тип возвращаемого значения в параметры функции

                    parameters.push_back({genericType, paramName}); // Добавляем параметр в вектор параметров функции
                }
                else
                {
                    std::string paramName = current().value; // Сохраняем имя параметра функции
                    consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора параметра функции

                    parameters.push_back({paramType, paramName}); // Добавляем параметр в вектор параметров функции
                }
            } while (match(TokenType::Comma)); // Пока следующий токен - это запятая, продолжаем добавлять параметры функции
        }

        consume(TokenType::RightParen, "Expected ')' after function parameters"); // Проверяем наличие правой скобки после параметров функции
        
        std::vector<std::string> labels; // Вектор меток функции
        while (check(TokenType::Label))
        {
            std::string label = current().value;
            consume(TokenType::Label, "Expected label"); // Проверяем наличие метки функции

            labels.push_back(label);
        }
        
        int expectedIndent = getIndentLevel(lines[lineIndex]) + 1; // Уровень отступа для блока if
        

        std::shared_ptr<BlockNode> body = nullptr; 
        
        {
            const int declIndent = getIndentLevel(lines[lineIndex]);
            const int expectedIndent = declIndent + 1;

            nextLine();

            if (isEndOfFile())
                throwError("Expected indentation after function declaration");

            if (getIndentLevel(lines[lineIndex]) == expectedIndent)
            {
                body = parseBlock(expectedIndent);
                lineIndex--; 
            }
            else
            {
                throwError("Expected indentation after function declaration");
            }
        }

        auto func = std::make_shared<FunctionNode>(functionName, association, returnType, parameters, labels, body); // Создаём узел функции
        func->line = line; func->column = token; // Устанавливаем строку и колонку для узла

        // При разборе копии подстановка уже сделана, это обычная функция
        if (isTemplate)
        {
            typeSubstitution.clear();
            if (!sharedLines) sharedLines = std::make_shared<std::vector<std::vector<Token>>>(lines);
            func->typeParameters = typeParameters;

            auto source = sharedLines;
            const std::string module = moduleName;
            func->instantiate = [source, module, line, token](const std::map<std::string, std::string>& types) {
                Parser copy(*source, module);
                copy.typeSubstitution = types;
                copy.lineIndex = line;
                copy.tokenIndex = token;
                return copy.parseFunction();
            };
        }
        return func; // Возвращаем узел функции
    }
    else
    {
        throwError("Expected '(' after function name");
    }

    return nullptr; // Если ничего не найдено, возвращаем nullptr
}

std::shared_ptr<ASTNode> Parser::parseCast(std::shared_ptr<ASTNode> expression)
{
    consume(TokenType::Arrow, "Expected '->' after expression"); // Проверяем наличие стрелки после выражения

    // x->T в копии шаблона
    if (check(TokenType::Identifier) && typeSubstitution.count(current().value))
    {
        expression->implicitCastTo = parseTomlType(typeSubstitution.at(current().value));
        advance();
        return expression;
    }
    if(check(TokenType::Type))
    {
        std::string typeName = current().value; // Сохраняем имя типа
        if(typeName != "array" && typeName != "func" && typeName != "string" && typeName != "struct" && typeName != "map" && typeName != "void")
        {
            advance(); // Переходим к следующему токену
        }
        else
        {
            throwError("Expected primitive type after '->'");
        }
        expression->implicitCastTo = std::make_shared<SimpleTypeNode>(typeName); // Создаём указатель на тип
        expression->implicitCastTo->line = lineIndex; expression->implicitCastTo->column = tokenIndex; // Устанавливаем строку и колонку для узла
        return expression; // Возвращаем выражение
    }
    else
    {
        throwError("Expected primitive type after '->'");
    }
    return nullptr; // Если ничего не найдено, возвращаем nullptr
}

std::shared_ptr<ASTNode> Parser::parseIf()
{
    int line = lineIndex; int token = tokenIndex; // Запоминаем строку и токен

    // Проверяем, что текущий токен - это ключевое слово if
    consume(TokenType::Keyword, "Expected 'if' keyword"); 
    // Мы не делаем проверку здесь на наличие круглой скобки, так как это может быть выражение без скобок
    // И даже если это выражение без скобок, то оно всё равно может казаться как if (expression)
    // Вот воможные варианты:
    // if(2+2==4), if((2+2)==4), if 2+2==4, if (2+2)==4
    // Поэтому мы просто сразу посылаем его в parseExpression который разберёт его

    auto condition = parseExpression();
    if (!isEndOfLine()) throwError("Character(" + current().value + ") not parsed at the end of");
    int expectedIndent = getIndentLevel(lines[lineIndex]) + 1; // Уровень отступа для блока if
    nextLine(); // Переходим к следующей строке токенов 

    auto thenBlock = parseBlock(expectedIndent);
    std::shared_ptr<ASTNode> elseBlock = nullptr;
    tokenIndex = expectedIndent-1;
    //if (tokenIndex!=(expectedIndent-1)) 

    if (current().value == "else")
    {
        if(peek().value == "if")
        {
            advance();
            elseBlock = parseIf();
            lineIndex++;
        }
        else
        {
            advance(); // consume 'else'
            if (!isEndOfLine()) throwError("Character(" + current().value + ") not parsed at the end of");
            nextLine();
            elseBlock = parseBlock(expectedIndent);
        }
    }

    lineIndex--; // Без этого он скипает 2 линии а не одну

    auto ifNode = std::make_shared<IfNode>(condition, thenBlock, elseBlock);
    ifNode->line = line; ifNode->column = token; // Устанавливаем строку и колонку для узла
    
    return ifNode; // Возвращаем узел if
}

std::shared_ptr<ASTNode> Parser::parseFor()
{
    /*
    For statement:
    for [iteration variable] in [variable]
    for i in arr
    */
    int line = lineIndex; int token = tokenIndex; // Запоминаем строку и токен

    consume(TokenType::Keyword, "Expected 'for' keyword"); // Проверяем наличие ключевого слова for

    std::string iterationVariable = current().value; // Сохраняем имя переменной итерации
    consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора

    if(current().type != TokenType::Keyword && current().value != "in")
    {
        throwError("Expected 'in' keyword at line");
    }
    consume(TokenType::Keyword, "Expected 'in' keyword"); // Проверяем наличие ключевого слова in
    
    int currentLine = lineIndex; // Сохраняем текущую линию
    auto iterable = parseExpression(); // Парсим переменную для итерации
    if (!isEndOfLine()) throwError("Character(" + current().value + ") not parsed at the end of");
    if (currentLine < lineIndex) // Если мы не перешли на следующую линию, то это не for
        lineIndex--;

    int expectedIndent = getIndentLevel(lines[lineIndex]) + 1; // Уровень отступа для блока for
    nextLine(); // Переходим к следующему токену 

    auto body = parseBlock(expectedIndent); // Парсим тело цикла for

    lineIndex--; // Без этого он скипает 2 линии а не одну

    auto forNode = std::make_shared<ForNode>(iterationVariable, iterable, body); // Создаём узел цикла for
    forNode->line = line; forNode->column = token; // Устанавливаем строку и колонку для узла
    
    return forNode; // Возвращаем узел цикла for
}

std::shared_ptr<ASTNode> Parser::parseWhile()
{
    /*
    While statement: 
    while (expression)
    |   {body}
    */
    int line = lineIndex; int token = tokenIndex; // Запоминаем строку и токен

    consume(TokenType::Keyword, "Expected 'while' keyword"); // Проверяем наличие ключевого слова while
    auto condition = parseExpression();
    if (!isEndOfLine()) throwError("Character(" + current().value + ") not parsed at the end of");

    int expectedIndent = getIndentLevel(lines[lineIndex]) + 1;
    nextLine();

    auto body = parseBlock(expectedIndent);

    lineIndex--; // Без этого он скипает 2 линии а не одну


    auto whileNode = std::make_shared<WhileNode>(condition, body);
    whileNode->line = line; whileNode->column = token; // Устанавливаем строку и колонку для узла
    return whileNode; // Возвращаем узел цикла while
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
    block->line = lineIndex; block->column = tokenIndex; // Устанавливаем строку и колонку для узла
    while (!isEndOfFile()) {
        int actualIndent = getIndentLevel(lines[lineIndex]);

        if (actualIndent < expectedIndent) break;
        
        if (actualIndent > expectedIndent)
            throwError("Unexpected indentation");

        // пропустить Pipe токены
        tokenIndex = actualIndent;

        int currentLine = lineIndex;
        auto stmt = parseStatement();  // не должен заниматься отступами
        if (stmt) block->statements.push_back(stmt);
        if ((currentLine == lineIndex) && !isEndOfLine()) throwError("Character(" + current().value + ") not parsed at the end of");
        nextLine();
    }

   return block;
}

/*
if totalScore > 10 and !(totalScore == 15)
|   echo("Above threshold")
else if totalScore == 15
|   echo("Exact match")
else
|   echo("Below threshold")
*/

std::shared_ptr<ASTNode> Parser::parseReturn()
{
    /*
    Return statement:
    return [expression]
    return 0
    */
    int line = lineIndex; int token = tokenIndex; // Запоминаем строку и токен

    consume(TokenType::Keyword, "Expected 'return' keyword"); // Проверяем наличие ключевого слова return
    std::shared_ptr<ASTNode> expression;
    if(current().column != -1)
        expression = parseExpression(); 
    else
        expression = std::make_shared<NullNode>();

    auto returnNode = std::make_shared<ReturnNode>(expression); // Создаём узел возврата
    returnNode->line = line; returnNode->column = token; // Устанавливаем строку и колонку для узла
    return returnNode; // Возвращаем узел возврата
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
            throwError("Expected 'const' or 'final'");
        }
        keyWord != "const" ? finalFlag = true : finalFlag = false; // Если токен - это const, то finalFlag = false, иначе finalFlag = true
        advance();                                                 // Переходим к следующему токену
    }

    if(current().type == TokenType::Type && (current().value != "array" && current().value != "map"))                       // Проверяем наличие типа
    {
        type = std::make_shared<SimpleTypeNode>(current().value); // Сохраняем тип переменной   
        type->line = lineIndex; type->column = tokenIndex; // Устанавливаем строку и колонку для узла
        
        consume(TokenType::Type, "Expected type");              // Проверяем наличие типа
        variableName = current().value;                         // Сохраняем имя переменной
        consume(TokenType::Identifier, "Expected identifier");  // Проверяем наличие идентификатора
        if(!check(TokenType::Operator) || current().value != "=") // Проверяем наличие оператора присваивания
        {   
            std::shared_ptr<NoneNode> none = std::make_shared<NoneNode>();
            none->line = lineIndex; none->column = tokenIndex; // Устанавливаем строку и колонку для узла

            auto varAssign = std::make_shared<VariableAssignNode>(variableName, isConst, type, none);
            varAssign->line = lineIndex; varAssign->column = tokenIndex; // Устанавливаем строку и колонку для узла
            varAssign->isDeferred = finalFlag;
            return varAssign;
        }
    }
    else if(current().type == TokenType::Identifier && peek().value == "^=") // Проверяем наличие идентификатора
    {
        type = std::make_shared<SimpleTypeNode>("auto"); // Сохраняем тип переменной
        type->line = lineIndex; type->column = tokenIndex; // Устанавливаем строку и колонку для узла

        variableName = current().value;                         // Сохраняем имя переменной
        consume(TokenType::Identifier, "Expected identifier");  // Проверяем наличие идентификатора
        if(!check(TokenType::Operator) || current().value != "^=") // Проверяем наличие оператора присваивания
        {
            throwError("Expected '^=' operator after static variable declaration");
        }
    }
    else if(current().type == TokenType::Identifier && peek().value == "=")
    {
        // ну тут тогда простой assignment
        variableName = current().value;                         // Сохраняем имя переменной
        consume(TokenType::Identifier, "Expected identifier");  // Проверяем наличие идентификатора
        consume(TokenType::Operator, "Expected = operator");
        auto expression = parseExpression();
        auto varReassign = std::make_shared<VariableReassignNode>(variableName, expression);
        varReassign->line = lineIndex; varReassign->column = tokenIndex; // Устанавливаем строку и колонку для узла
        return varReassign; // Возвращаем узел присваивания переменной
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
            none->line = lineIndex; none->column = tokenIndex; // Устанавливаем строку и колонку для узла

            auto varAssign = std::make_shared<VariableAssignNode>(variableName, isConst, type, none);
            varAssign->line = lineIndex; varAssign->column = tokenIndex; // Устанавливаем строку и колонку для узла
            varAssign->isDeferred = finalFlag;
            return varAssign; // Возвращаем узел присваивания переменной
        }
    }
    // Здесь мы уже у оператора присваивания, который идёт после имени переменной
    advance(); // Переходим к следующему токену
    //std::cout << "[parseAssignment] Current token: " << current().value << std::endl; // Выводим текущий токен в консоль
    std::shared_ptr<ASTNode> expression = parseExpression(); // Разбираем выражение справа от оператора присваивания
    
    if (!expression) // Если выражение не разобрано, выбрасываем исключение
    {
        throwError("Expected expression after assignment operator");
    }

    auto varAssign = std::make_shared<VariableAssignNode>(variableName, isConst, type ,expression); // Создаём узел присваивания переменной
    varAssign->line = lineIndex; varAssign->column = tokenIndex; // Устанавливаем строку и колонку для узла
    return varAssign; // Возвращаем узел присваивания переменной
}

std::shared_ptr<ASTNode> Parser::parseCall()
{
    /*
    Function call:
    [functionName]([expression], [expression], ...)
    bubbleSort(arr, 0, 10)
    */
    std::shared_ptr<CallNode> callNode = std::make_shared<CallNode>();
    callNode->line = lineIndex; callNode->column = tokenIndex; // Устанавливаем строку и колонку для узла
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
    int line = lineIndex; int token = tokenIndex; // Запоминаем строку и токен
    consume(TokenType::LeftBracket, "Expected '[' before struct declaration"); // Проверяем наличие левой скобки
    consume(TokenType::Type, "Expected 'struct' keyword"); // Проверяем наличие ключевого слова struct
    consume(TokenType::RightBracket, "Expected ']' before struct declaration"); // Проверяем наличие левой скобки

    std::string name = current().value; // Сохраняем имя структуры
    consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора структуры
    if (!isEndOfLine()) throwError("Character(" + current().value + ") not parsed at the end of");

    int expectedIndent = getIndentLevel(lines[lineIndex]) + 1; // Уровень отступа для блока структуры
    nextLine(); // Переходим к следующему токену
    auto body = parseBlock(expectedIndent); // Парсим тело структуры
    
    lineIndex--; // Без этого он скипает 2 линии а не одну    
    
    // Метод это обычная функция, у которой первым параметром идёт сам объект
    if (auto block = std::dynamic_pointer_cast<BlockNode>(body)) {
        for (const auto& statement : block->statements) {
            if (auto method = std::dynamic_pointer_cast<FunctionNode>(statement)) {
                if (!method->typeParameters.empty())
                    throwError("Generic methods are not supported yet: " + method->name);
                method->associated = name;
                method->parameters.insert(method->parameters.begin(), {std::make_shared<SimpleTypeNode>(name), "self"});
            }
        }
    }

    auto structNode = std::make_shared<StructNode>(name, body); // Создаём узел структуры
    structNode->line = line; structNode->column = token; // Устанавливаем строку и колонку для узла
    return structNode; // Возвращаем узел структуры
}

std::shared_ptr<ASTNode> Parser::parseUse()
{
    std::map<std::vector<std::string>, std::string> paths;
    std::string alias;
    int line = lineIndex; int token = tokenIndex; // Запоминаем строку и токен
    std::vector<std::string> path;

    consume(TokenType::Keyword, "Expected 'use' keyword"); // Проверяем наличие ключевого слова use
    nextLine();

    while (!isEndOfFile() && check(TokenType::PipeArrow))
    {
        consume(TokenType::PipeArrow, "Expected '|>' at start of import entry");
    
        do {
            path.push_back(current().value);
            consume(TokenType::Identifier, "Expected identifier in import path");
            if (!current().value.empty() && !(check(TokenType::Arrow) || check(TokenType::Colon)))
                throwError("Import path must be a single word without spaces");
        } while (match(TokenType::Arrow));
    
        if (match(TokenType::Colon)) {
            alias = current().value;
            consume(TokenType::Identifier, "Expected alias name after ':'");
            if (!current().value.empty())
                throwError("Alias must be a single word without spaces");
        }
    
        paths.insert({path, alias}); // Добавляем путь в мапу
        path.clear(); // Очищаем путь для следующего импорта
        alias.clear(); // Очищаем алиас для следующего импорта

        // Только если не конец файла — переходи к следующей строке
        if (!isEndOfFile())
            nextLine();
    }

    lineIndex--; // Без этого он скипает 2 линии а не одну
    auto importNode = std::make_shared<ImportNode>(paths);
    importNode->line = line; importNode->column = token; // Устанавливаем строку и колонку для узла
    return importNode; // Возвращаем узел импорта
}