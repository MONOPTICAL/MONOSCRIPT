#include "headers/Parser.h"
/*
Тут все логические и вспомогательные функции
По типу getIndentLevel, getLastTokenInCurrentLine и т.д.
Так сделано потому что язык очень гибкий и многофункциональный и написать всё в одном месте не удобно
*/

/// @brief Возвращает текущий токен
/// @details Возвращает текущий токен, если индекс токена меньше размера токенов
/// @return Текущий токен, если индекс токена меньше размера токенов, иначе возвращает токен с типом None и пустым значением
Token Parser::current()
{
    if (!isEndOfFile() && !isEndOfLine()) {
        return lines[lineIndex][tokenIndex];
    }
    return Token{TokenType::None, "", -1, -1};
}

/// @brief Переходит к следующему токену
/// @details Если индекс токена меньше размера токенов, увеличивает индекс токена на 1 и возвращает текущий токен
/// @return Текущий токен, если индекс токена меньше размера токенов, иначе возвращает токен с типом None и пустым значением
Token Parser::advance()
{
    if (!isEndOfFile() && !isEndOfLine())
    {
        return lines[lineIndex][tokenIndex++];
    }
    return Token{TokenType::None, "", -1, -1};
}

/// @brief Переходит к следующему токену, если текущий токен совпадает с заданным типом
/// @details Если текущий токен совпадает с заданным типом, увеличивает индекс токена на 1
/// @return true, если текущий токен совпадает с заданным типом, иначе возвращает false
bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

/// @brief Проверяет, совпадает ли текущий токен с заданным типом
/// @details Если текущий токен совпадает с заданным типом, возвращает true, иначе возвращает false
/// @param type Тип токена, с которым нужно сравнить текущий токен
/// @return true, если текущий токен совпадает с заданным типом, иначе возвращает false
bool Parser::check(TokenType type)
{
    if (isEndOfFile()) return false;
    if (isEndOfLine()) return false;

    return lines[lineIndex][tokenIndex].type == type;
}

/// @brief Возвращает следующий токен без перехода к нему
/// @details Если индекс токена меньше размера токенов, возвращает следующий токен, иначе возвращает токен с типом None и пустым значением
/// @return Следующий токен, если индекс токена меньше размера токенов, иначе возвращает токен с типом None и пустым значением
Token Parser::peek()
{
    if (!isEndOfFile() && tokenIndex + 1 < lines[lineIndex].size())
    {
        return lines[lineIndex][tokenIndex+1];
    }
    return Token{TokenType::None, "", -1, -1};
}

/// @brief Переходит к следующей строке токенов
/// @details Если достигнут конец файла, возвращает false. Иначе увеличивает индекс строки на 1 и сбрасывает индекс токена на 0
/// @return true, если удалось перейти к следующей строке, иначе возвращает false
bool Parser::nextLine()
{
    if (isEndOfFile()) return false;
    lineIndex++;
    tokenIndex = 0;
    return true;
}

void Parser::throwError(const std::string &errMsg)
{
    std::string sourceFragment = "";
    if (lineIndex < lines.size()) 
    {
        std::string lineStr = "";
        for (const auto& token : lines[lineIndex]) 
        {
            lineStr += token.value + " ";
        }
        
        sourceFragment = "\n|>  " + lineStr + " <|\n";
        sourceFragment = std::string(sourceFragment.size()-2, '-') + sourceFragment + std::string(sourceFragment.size()-2, '-');
    }
    
    throw std::runtime_error(
        "Parser Error [line " + std::to_string(lineIndex+1) + 
        (tokenIndex < lines[lineIndex].size() ? ", column " + std::to_string(lines[lineIndex][tokenIndex].column) : "") +
        "]: " + errMsg + "\n\nSOURCE(FILENAME) - placeholders\n" + sourceFragment
    );
}

/// @brief Проверяет, совпадает ли текущий токен с заданным типом и переходит к следующему токену, если совпадает
/// @param Token Тип токена, с которым нужно сравнить текущий токен
/// @param errMsg Сообщение об ошибке, если токен не совпадает с заданным типом
/// @throws std::runtime_error Если токен не совпадает с заданным типом, выбрасывает исключение с сообщением об ошибке
/// @details Если текущий токен совпадает с заданным типом, переходит к следующему токену. Иначе выбрасывает исключение с сообщением об ошибке
void Parser::consume(TokenType Token, const std::string &errMsg)
{
    if (match(Token)) return;

    throwError(errMsg);
}

/// @brief  Проверяет, достигнут ли конец файла
/// @details Если индекс строки больше или равен размеру токенов, возвращает true
/// @return  true, если достигнут конец файла, иначе false
bool Parser::isEndOfFile() const
{
    return lineIndex >= lines.size();
}

/// @brief Проверяет, достигнут ли конец строки токенов
/// @return true, если достигнут конец строки токенов, иначе false
bool Parser::isEndOfLine() const
{
    return tokenIndex >= lines[lineIndex].size();
}

/// @brief Получает уровень отступа для данной строки токенов
/// @details Уровень отступа определяется количеством токенов типа Pipe в начале строки
/// @param line Строка токенов, для которой нужно получить уровень отступа 
/// @return Уровень отступа для данной строки токенов
int Parser::getIndentLevel(const std::vector<Token>& line)
{
    int indentLevel = 0;
    for (const auto& token : line)
    {
        if (token.type == TokenType::Pipe) // если токен - отступ
        {
            indentLevel++;
        }
        else
        {
            break; // выходим из цикла, если встретили не отступ
        }
    }
    return indentLevel;
}

/// @brief Получает последний токен в текущей строке
/// @return Последний токен в текущей строке, если индекс строки меньше размера токенов, иначе возвращает токен с типом None и пустым значением
Token Parser::getLastTokenInCurrentLine() const
{
    if (lineIndex < lines.size())
    {
        const std::vector<Token>& currentLine = lines[lineIndex];
        if (!currentLine.empty())
        {
            return currentLine.back(); // возвращаем последний токен в текущей строке
        }
    }
    return Token{TokenType::None, "", -1, -1};
}

/// @brief Возвращает полный тип токена
/// @details Может быть случий когда пользователь использует array<i32> или array<string> и т.д. и такие случии обрабатываются в Lexer как {Type, "array"} {Operator, "<"} {Type, "i32"} {Operator, ">"}
/// @return Полный тип токена, если он существует, иначе возвращает пустую строку
std::shared_ptr<TypeNode> Parser::getFullType()
{
    Token currentToken = current();
    if (currentToken.type != TokenType::Type && currentToken.type != TokenType::Identifier) {
        throwError("Expected type identifier");
    }
    
    std::string baseName = current().value;
    advance();
    
    // Проверяем, является ли это параметризованным типом (generic)
    if (check(TokenType::Operator) && current().value == "<") {
        advance(); // Пропускаем <
        
        auto genericType = std::make_shared<GenericTypeNode>(baseName);
        genericType->line = lineIndex;
        genericType->column = tokenIndex;
        
        // Парсим параметры типа
        do {
            genericType->typeParameters.push_back(getFullType());
            
            if (check(TokenType::Comma)) {
                advance(); // Пропускаем ,
            } else {
                break;
            }
        } while (true);
        
        // Проверяем закрывающий >
        if (!check(TokenType::Operator) && !(current().value == "<")) {
            throwError("Expected > to close generic type");
        }
        advance(); // Пропускаем >
        
        return genericType;
    } else {
        // Простой тип
        auto node = std::make_shared<SimpleTypeNode>(baseName);
        node->line = lineIndex; node->column = tokenIndex;
        return node;
    }
}

/// @brief Получает приоритет токена
/// @param token Токен, для которого нужно получить приоритет
/// @return Все возможные приоритеты токенов: or - 1, and - 2, == - 3, + и - - 4, * и / - 5. Если токен не является оператором или ключевым словом, возвращает -1
int Parser::getPrecedence(const Token &token) const
{
    if (token.type != TokenType::Operator && token.type != TokenType::Keyword && token.type != TokenType::PipeArrow) return -1;
    if (token.value == "or") return 1; // keyword
    if (token.value == "and") return 2; // keyword
    if (token.value == "|>") return 3; // keyword
    if (token.value == "==" || token.value == "<" || token.value == ">" || token.value == "!=" || token.value == "<=" || token.value == ">=") return 4; // operator
    if (token.value == "+" || token.value == "-") return 5; // operator
    if (token.value == "*" || token.value == "/" || token.value == "%" || token.value == "**") return 6; // operator

    return -1; // Для компилятора, чтобы не ругался на -Wreturn-type
}

std::shared_ptr<ASTNode> Parser::parseBinary(int precedence) 
{
    auto left = parseUnary(); // 2
    int initialLine = lineIndex;  // Запоминаем начальную строку
    int movedLine = 0; // Флаг для проверки, был ли сделан переход на следующую строку

    while (true)
    {
        Token currentToken = current(); // + текущий токен, == текущий токен
        int currentPrecedence = getPrecedence(currentToken); // Получаем приоритет текущего токена
        // Если дошли до конца строки — пробуем перейти на следующую строку с pipe
        if (currentToken.type == TokenType::None)
        {
            // Проверяем, что следующая строка существует
            if (!isEndOfFile() && lineIndex + 1 < lines.size())
            {
                // Получаем уровень отступа следующей строки
                int nextIndent = getIndentLevel(lines[lineIndex + 1]);

                if (nextIndent < lines[lineIndex + 1].size() &&
                    lines[lineIndex + 1][nextIndent].type == TokenType::PipeArrow)
                {
                    nextLine();
                    movedLine++;

                    tokenIndex = getIndentLevel(lines[lineIndex]); // Перепрыгиваем через пайпы/отступы

                    isPipe = true;
                    currentToken = current();
                    currentPrecedence = getPrecedence(currentToken);
                }
                else
                {
                    break; // Нет pipe — выходим из цикла, не переходим на строку
                }
            }
            else
            {
                break; // Конец файла — выходим из цикла
            }
        }
        if (currentPrecedence < precedence) break; // Если текущий приоритет меньше, чем заданный, выходим из цикла

        advance(); // Переходим к следующему токену
        auto right = parseBinary(currentPrecedence);

        left = std::make_shared<BinaryOpNode>(left, currentToken.value, right); // Создаём новый узел бинарной операции
        left->line = lineIndex; left->column = tokenIndex; // Устанавливаем строку и колонку для узла
    }

    return left; // Возвращаем разобранное выражение
}

std::shared_ptr<ASTNode> Parser::parseUnary()
{
    Token currentToken = current();
    if (currentToken.type == TokenType::Operator && (currentToken.value == "!" || currentToken.value == "-" || currentToken.value == "?"))
    {
        advance(); // Переходим к следующему токену
        auto right = parseUnary(); // Рекурсивно разбираем правую часть выражения
        auto node = std::make_shared<UnaryOpNode>(currentToken.value, right); // Создаём новый узел унарной операции
        node->line = lineIndex; node->column = tokenIndex; // Устанавливаем строку и колонку для узла
        return node; // Возвращаем узел унарной операции
    }
    return parsePrimary(); // Если нет унарной операции, разбираем первичное выражение
}

std::shared_ptr<ASTNode> Parser::parsePrimary()
{
    Token currentToken = current();
    if (currentToken.type == TokenType::Number) // Если токен - число
    {
        advance(); // Переходим к следующему токену
        std::optional<int64_t> intValue; // Переменная для хранения значения числа
        std::optional<float> floatValue; // Переменная для хранения значения числа с плавающей точкой
        try
        {
            if(std::find(currentToken.value.begin(), currentToken.value.end(), '.') != currentToken.value.end()) // Если число с плавающей точкой
            {
                floatValue = std::stof(currentToken.value); // Преобразуем строку в число с плавающей точкой
            }
            else
            {
                intValue = std::stoll(currentToken.value); // Преобразуем строку в число
            }
        }
        catch (const std::invalid_argument& e) // Если преобразование не удалось, выбрасываем исключение
        {
            throw std::runtime_error("Parser Error: Invalid number format at line " + std::to_string(currentToken.line) +
                ", column " + std::to_string(currentToken.column) +
                ": " + currentToken.value);
        }

        if (intValue.has_value()) // Если число целое
        {
            std::shared_ptr<SimpleTypeNode> type;
            if (intValue.value() == 0 || intValue.value() == 1) {
                type = std::make_shared<SimpleTypeNode>("i1");
            }
            else if (intValue.value() >= -128 && intValue.value() <= 127) {
                type = std::make_shared<SimpleTypeNode>("i8");
            }
            else if (intValue.value() >= -32768 && intValue.value() <= 32767) {
                type = std::make_shared<SimpleTypeNode>("i16");
            }
            else if (intValue.value() >= -2147483648 && intValue.value() <= 2147483647) {
                type = std::make_shared<SimpleTypeNode>("i32");
            }
            else {
                type = std::make_shared<SimpleTypeNode>("i64");
            }
            type->line = lineIndex; type->column = tokenIndex;

            auto ASTnode = std::make_shared<NumberNode>(intValue.value(), type); // Создаём узел числа

            ASTnode->line = lineIndex; ASTnode->column = tokenIndex; // Устанавливаем строку и колонку для узла
            if(check(TokenType::Arrow)) return parseCast(ASTnode); // Если есть каст, то кастим
            return ASTnode; // Возвращаем узел числа
        }
        else
        {
            auto ASTnode = std::make_shared<FloatNumberNode>(floatValue.value()); // Создаём узел числа с плавающей точкой
            
            ASTnode->line = lineIndex; ASTnode->column = tokenIndex; // Устанавливаем строку и колонку для узла

            if(check(TokenType::Arrow)) return parseCast(ASTnode); // Если есть каст, то кастим
            return ASTnode; // Возвращаем узел числа с плавающей точкой
        }
    }
    else if (currentToken.type == TokenType::String) // Если токен - строка
    {
        advance(); // Переходим к следующему токену
        
        std::string strValue = currentToken.value.substr(1, currentToken.value.length()-2); // Получаем значение строки обрезая кавычки

        // Обрабатываем экранированные последовательности
        std::string processedValue;
        processedValue.reserve(strValue.length()); // Резервируем память для оптимизации
        
        for (size_t i = 0; i < strValue.length(); ++i) {
            if (strValue[i] == '\\' && i + 1 < strValue.length()) {
                // Обработка экранированных последовательностей
                switch (strValue[i + 1]) {
                    case 'n': processedValue.push_back('\n'); break;
                    case 't': processedValue.push_back('\t'); break;
                    case 'r': processedValue.push_back('\r'); break;
                    case 'f': processedValue.push_back('\f'); break;
                    case 'b': processedValue.push_back('\b'); break;
                    case '0': processedValue.push_back('\0'); break;
                    case '\'': processedValue.push_back('\''); break;
                    case '"': processedValue.push_back('"'); break;
                    case '\\': processedValue.push_back('\\'); break;
                    default: 
                        // Неизвестная экранированная последовательность - сохраняем как есть
                        processedValue.push_back('\\');
                        processedValue.push_back(strValue[i + 1]);
                        break;
                }
                ++i;
            } else {
                processedValue.push_back(strValue[i]);
            }
        }
        
        auto stringNode = std::make_shared<StringNode>(processedValue); // Используем обработанное значение // Создаём узел строки обризаяя кавычки
        stringNode->line = lineIndex; stringNode->column = tokenIndex; // Устанавливаем строку и колонку для узла
        
        return stringNode; // Возвращаем узел строки
    }
    else if (currentToken.type == TokenType::Identifier && peek().type == TokenType::LeftParen) // Если токен - идентификатор
    {
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
            
            auto ASTnode = std::make_shared<CallNode>(currentToken.value, arguments); // Создаём узел вызова функции
            ASTnode->line = lineIndex; ASTnode->column = tokenIndex; // Устанавливаем строку и колонку для узла
            
            if(check(TokenType::Arrow)) return parseCast(ASTnode); // Если есть каст, то кастим
            
            return ASTnode; // Возвращаем узел вызова функции
        }
    }
    else if (currentToken.type == TokenType::Identifier && peek().type == TokenType::LeftBracket)
    {
        return parseMemberExpression();
    }
    else if (currentToken.type == TokenType::Identifier && peek().type == TokenType::Dot)
    {
        return parseMemberExpression();
    }
    else if (currentToken.type == TokenType::LeftBracket && peek().type == TokenType::Type)
    {
        // [i8]func(i8: a) -> a*a
        advance(); // [

        std::shared_ptr<TypeNode> returnType = getFullType();

        consume(TokenType::RightBracket, "Expected ']' after type in lambda/function literal");
        consume(TokenType::LeftParen, "Expected '(' after function name"); // Проверяем наличие правой скобки

        // Парсим параметры
        std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> params;
        do
        {
            std::shared_ptr<TypeNode> paramType = getFullType(); // Получаем полный тип параметра функции
            consume(TokenType::Colon, "Expected ':' after parameter type"); // Проверяем наличие двоеточия после типа параметра функции

            std::string paramName = current().value; // Сохраняем имя параметра функции
            consume(TokenType::Identifier, "Expected identifier"); // Проверяем наличие идентификатора параметра функции

            params.push_back({paramType, paramName}); // Добавляем параметр в вектор параметров функции
        } while (match(TokenType::Comma)); // Пока следующий токен - это запятая, продолжаем добавлять параметры функции
    
        consume(TokenType::RightParen, "Expected ')' after parameters in lambda/function literal");

        // Парсим тело
        if(check(TokenType::Arrow))
        {
            advance(); // Переходим к следующему токену
            auto body = parseExpression();
            auto lambda = std::make_shared<LambdaNode>(returnType, params, body);
            lambda->line = lineIndex; lambda->column = tokenIndex; // Устанавливаем строку и колонку для узла
            return lambda; // Возвращаем узел лямбда-функции
        }
        else if(check(TokenType::Colon))
        {
            int expectedIndent = getIndentLevel(lines[lineIndex]) + 1; // Уровень отступа для блока if
            nextLine(); // Переходим к следующему токену
            
            auto body = parseBlock(expectedIndent);
            
            lineIndex--; // Без этого он скипает 2 линии а не одну
            tokenIndex = getIndentLevel(lines[lineIndex]); // Перепрыгиваем через пайпы/отступы
            auto lamda = std::make_shared<LambdaNode>(returnType, params, body);
            lamda->line = lineIndex; lamda->column = tokenIndex; // Устанавливаем строку и колонку для узла
            return lamda; // Возвращаем узел лямбда-функции
        }
        else
        {
            throwError("Expected '->' or ':' after parameters in lambda/function literal");
        }

        // Вернуть FunctionLiteralNode или аналогичный узел

        /*
        FunctionNode(const std::string& name, const std::string& associated, std::shared_ptr<TypeNode> returnType, const std::vector<std::pair<std::shared_ptr<TypeNode>, std::string>> parameters, std::shared_ptr<ASTNode> body) 
            : name(name), associated(associated), returnType(returnType), parameters(parameters), body(body) {}
        */
    }
    else if (currentToken.type == TokenType::LeftBracket)
    {
        std::shared_ptr<BlockNode> body = std::make_shared<BlockNode>();
        body->line = lineIndex; body->column = tokenIndex; // Устанавливаем строку и колонку для узла
        if(peek().type == TokenType::RightBracket) {
            auto none = std::make_shared<NoneNode>(); 
            none->line = lineIndex; none->column = tokenIndex; // Устанавливаем строку и колонку для узла
            return none; // Возвращаем узел none
        }// если нихуя нету в скобках скипай

        do 
        {
            advance();

            auto value = parseExpression(); 

            body->statements.push_back(value);
        } while(current().type == TokenType::Comma);

        consume(TokenType::RightBracket, "Expected ']' after array arguments");
        return body;
    }
    else if (currentToken.type == TokenType::LeftBrace)
    {
        if(peek().type == TokenType::RightBrace)  return std::make_shared<NoneNode>(); 

        std::shared_ptr<BlockNode> body = std::make_shared<BlockNode>();
        body->line = lineIndex; body->column = tokenIndex; // Устанавливаем строку и колонку для узла
        
        do 
        {
            std::shared_ptr<KeyValueNode> key_value = std::make_shared<KeyValueNode>();
            key_value->line = lineIndex; key_value->column = tokenIndex; // Устанавливаем строку и колонку для узла

            advance();

            key_value->keyName = current().value; // {[here] : value}

            auto key = parseExpression(); // {[here] : value}
            key_value->key = key; 

            consume(TokenType::Colon, "Expected : after key"); // Ну сжираем нахуй : между ними

            auto value = parseExpression(); // {key : [here]}
            key_value->value = value;

            body->statements.push_back(key_value);
        } while(current().type == TokenType::Comma);

        consume(TokenType::RightBrace, "Expected '}' after map arguments");
        return body;
    }
    else if (currentToken.type == TokenType::LeftParen) // Если токен - левая скобка
    {
        advance(); // Переходим к следующему токену
        auto expression = parseExpression(); // Разбираем выражение внутри скобок

        consume(TokenType::RightParen, "Expected ')' after expression"); // Проверяем наличие правой скобки

        /*
        TODO: Каст выражений с помощью CastNode.
        Пример: (1+2)->i32
        if (check(TokenType::Arrow)) // Если есть каст, то кастим
        {
            auto cast = std::make_shared<CastNode>(expression);
            cast->line = lineIndex; cast->column = tokenIndex; // Устанавливаем строку и колонку для узла
            return parseCast(cast);
        }
        */
        return expression; // Возвращаем разобранное выражение
    }
    else if (currentToken.type == TokenType::Keyword)
    {
        if (currentToken.value == "true" || currentToken.value == "false") // Если токен - булевый литерал
        {
            advance(); // Переходим к следующему токену
            auto type = std::make_shared<SimpleTypeNode>("i1"); // Создаём тип bool
            
            type->line = lineIndex; type->column = tokenIndex; // Устанавливаем строку и колонку для узла
            
            auto numberNode = std::make_shared<NumberNode>(currentToken.value == "true", type); // Создаём узел булевого значения
            
            numberNode->line = lineIndex; numberNode->column = tokenIndex; // Устанавливаем строку и колонку для узла
            return numberNode; // Создаём узел булевого значения
        }
        else if (currentToken.value == "null") // Если токен - null
        {
            advance(); // Переходим к следующему токену
            auto nullNode = std::make_shared<NullNode>(); // Создаём узел null
            nullNode->line = lineIndex; nullNode->column = tokenIndex; // Устанавливаем строку и колонку для узла
            return nullNode; // Возвращаем узел null
        }
        else if (currentToken.value == "none")
        {
            advance(); // Переходим к следующему токену
            auto noneNode = std::make_shared<NoneNode>(); // Создаём узел none
            noneNode->line = lineIndex; noneNode->column = tokenIndex; // Устанавливаем строку и колонку для узла
            return noneNode; // Возвращаем узел none
        }
        else if (currentToken.value == "defined")
        {
            advance();
            auto call = std::make_shared<CallNode>();
            call->line = lineIndex; call->column = tokenIndex; // Устанавливаем строку и колонку для узла
            call->callee = "defined";
            call->arguments.push_back(parseExpression());
            return call;
        }
    }
    else if (currentToken.type == TokenType::Identifier && (peek().value != "." || peek().value != "["))
    {
        advance();
        auto ASTnode = std::make_shared<IdentifierNode>(currentToken.value);
        ASTnode->line = lineIndex; ASTnode->column = tokenIndex; // Устанавливаем строку и колонку для узла
        if(check(TokenType::Arrow)) return parseCast(ASTnode); // Если есть каст, то кастим
        else return ASTnode; // Возвращаем узел идентификатора
    }
    throw std::runtime_error("Parser Error: Unknown primary expression at line " + std::to_string(currentToken.line) +
        ", column " + std::to_string(currentToken.column) +
        ": " + currentToken.value);
}

void Parser::parseDotNotation(std::shared_ptr<AccessExpression> next)
{
    // Точечная нотация
    advance(); // пропускаем .

    if (!(check(TokenType::Identifier) || check(TokenType::Type)))
    {
        throwError("Expected Identifier or Type after dot notation");
    }
    
    next->memberName = current().value;

    if(peek().type == TokenType::LeftParen)
    {
        next->expression = parseCall();
    }
    else 
        advance();
    
    next->notation = ".";
}

void Parser::parseArrayNotation(std::shared_ptr<AccessExpression> next)
{
    // Индексная нотация
    advance(); // пропускаем [
    
    if (check(TokenType::RightBracket))
    {
        throwError("Expected primary-expression before ']'");
    }
    
    next->expression = parseExpression();
    next->notation = "[]";
    
    consume(TokenType::RightBracket, "Expected ']' after array notation");
}

std::shared_ptr<ASTNode> Parser::parseMemberExpression()
{
    std::shared_ptr<AccessExpression> root = std::make_shared<AccessExpression>();
    root->line = lineIndex; root->column = tokenIndex; // Устанавливаем строку и колонку для узла
    auto currentNode = root;
    
    if (!(check(TokenType::Identifier) || check(TokenType::Type)))
    {
        throwError("Expected type or identifier in member expression");
    }
    
    root->memberName = current().value;
    advance();
    
    while (check(TokenType::Dot) || check(TokenType::LeftBracket) || 
           (check(TokenType::LeftParen) && currentNode->memberName.size() > 0))
    {
        auto next = std::make_shared<AccessExpression>();
        next->line = lineIndex; next->column = tokenIndex; // Устанавливаем строку и колонку для узла
        next->memberName = currentNode->memberName;
        
        if (check(TokenType::Dot))
        {
            parseDotNotation(next);
        }
        else if (check(TokenType::LeftBracket))
        {
            parseArrayNotation(next);
        }
        else if (check(TokenType::LeftParen))
        {
            next->expression = parseCall();
            next->notation = ".";
        }
        
        currentNode->nextAccess = next;
        currentNode = next;
    }
    
    if (root->nextAccess == nullptr && !root->expression)
    {
        auto IdentifierASTNode = std::make_shared<IdentifierNode>(root->memberName);
        IdentifierASTNode->line = lineIndex; IdentifierASTNode->column = tokenIndex; // Устанавливаем строку и колонку для узла
        return IdentifierASTNode; // Возвращаем узел идентификатора
    }
    
    return root;
}