#include "headers/Parser.h"
/*
Тут все логические и вспомогательные функции
По типу getIndentLevel, getLastTokenInCurrentLine и т.д.
Так сделано потому что язык очень гибкий и многофункциональный и написать всё в одном месте не удобно
*/



/// @brief Конструктор класса Parser
/// @details Инициализирует вектор токенов, индекс строки и индекс токена
/// @param tokens Вектор токенов, который будет парситься 
Parser::Parser(const std::vector<std::vector<Token>>& tokens)
{
    this->lines = tokens;
    this->lineIndex = 0;
    this->tokenIndex = 0;
    this->currentNode = nullptr;
}

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

/// @brief Возвращает полный тип токена при этом не переходя к следующему токену
/// @return Полный тип токена, если он существует, иначе возвращает пустую строку
std::string Parser::peekType()
{
    int backupIndex = tokenIndex;
    int backupLineIndex = lineIndex;
    std::string type = getFullType();
    tokenIndex = backupIndex;
    lineIndex = backupLineIndex;
    return type;
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

/// @brief Проверяет, совпадает ли текущий токен с заданным типом и переходит к следующему токену, если совпадает
/// @param Token Тип токена, с которым нужно сравнить текущий токен
/// @param errMsg Сообщение об ошибке, если токен не совпадает с заданным типом
/// @throws std::runtime_error Если токен не совпадает с заданным типом, выбрасывает исключение с сообщением об ошибке
/// @details Если текущий токен совпадает с заданным типом, переходит к следующему токену. Иначе выбрасывает исключение с сообщением об ошибке
void Parser::consume(TokenType Token, const std::string &errMsg)
{
    if (match(Token)) return;

    throw std::runtime_error("Parser Error: " + errMsg +
        " at line " + std::to_string(lineIndex));
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
std::string Parser::getFullType()
{
    std::string fullType;

    if(check(TokenType::Type) || check(TokenType::Identifier))
    {
        fullType += current().value; // добавляем текущий токен к полному типу
        advance(); // переходим к следующему токену
    }
    else 
    {
        throw std::runtime_error("Parser Error: Expected type at line " + std::to_string(lineIndex));
    }

    if (check(TokenType::Operator) && current().value == "<")
    {
        fullType += "<";
        advance(); // пропускаем <

        while (true)
        {
            fullType += getFullType(); // рекурсивно получаем вложенный тип

            if (check(TokenType::Comma)) {
                fullType += ", ";
                advance();
            }
            else if (check(TokenType::Operator) && current().value == ">") {
                fullType += ">";
                advance();
                break;
            }
            else {
                throw std::runtime_error("Parser Error: Expected ',' or '>' inside generic type at line " +
                    std::to_string(current().line));
            }
        }
    }

    return fullType; // возвращаем полный тип
}

/// @brief Получает приоритет токена
/// @param token Токен, для которого нужно получить приоритет
/// @return Все возможные приоритеты токенов: or - 1, and - 2, == - 3, + и - - 4, * и / - 5. Если токен не является оператором или ключевым словом, возвращает -1
int Parser::getPrecedence(const Token &token) const
{
    if (token.type != TokenType::Operator && token.type != TokenType::Keyword) return -1;
    if (token.value == "or") return 1; // keyword
    if (token.value == "and") return 2; // keyword
    if (token.value == "==") return 3; // operator
    if (token.value == "+" || token.value == "-") return 4; // operator
    if (token.value == "*" || token.value == "/") return 5; // operator

    return -1; // Для компилятора, чтобы не ругался на -Wreturn-type
}

std::shared_ptr<ASTNode> Parser::parseBinary(int precedence)
{
    auto left = parseUnary();

    while (true)
    {
        Token currentToken = current(); // + текущий токен, == текущий токен
        int currentPrecedence = getPrecedence(currentToken); // Получаем приоритет текущего токена
        if (currentPrecedence < precedence) break; // Если текущий приоритет меньше, чем заданный, выходим из цикла
        //std::cout << "not here, breaked" << std::endl; // Отладочный вывод
        advance(); // Переходим к следующему токену
        auto right = parseBinary(currentPrecedence); // Рекурсивно разбираем правую часть выражения

        left = std::make_shared<BinaryOpNode>(left, currentToken.value, right); // Создаём новый узел бинарной операции
    }
    
    return left; // Возвращаем разобранное выражение
}

std::shared_ptr<ASTNode> Parser::parseUnary()
{
    Token currentToken = current();
    if (currentToken.type == TokenType::Operator && (currentToken.value == "-" || currentToken.value == "!"))
    {
        advance(); // Переходим к следующему токену
        auto right = parseUnary(); // Рекурсивно разбираем правую часть выражения
        return std::make_shared<UnaryOpNode>(currentToken.value, right); // Создаём новый узел унарной операции
    }
    return parsePrimary(); // Если нет унарной операции, разбираем первичное выражение
}

std::shared_ptr<ASTNode> Parser::parsePrimary()
{
    Token currentToken = current();
    if (currentToken.type == TokenType::Number) // Если токен - число
    {
        advance(); // Переходим к следующему токену
        std::optional<int> intValue; // Переменная для хранения значения числа
        std::optional<float> floatValue; // Переменная для хранения значения числа с плавающей точкой
        try
        {
            if(std::find(currentToken.value.begin(), currentToken.value.end(), '.') != currentToken.value.end()) // Если число с плавающей точкой
            {
                floatValue = std::stof(currentToken.value); // Преобразуем строку в число с плавающей точкой
            }
            else
            {
                intValue = std::stoi(currentToken.value); // Преобразуем строку в число
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
            return std::make_shared<NumberNode>(intValue.value()); // Создаём узел числа
        }
        else
        {
            return std::make_shared<FloatNumberNode>(floatValue.value()); // Создаём узел числа с плавающей точкой
        }
    }
    else if (currentToken.type == TokenType::String) // Если токен - строка
    {
        advance(); // Переходим к следующему токену
        return std::make_shared<StringNode>(currentToken.value); // Создаём узел строки
    }
    else if (currentToken.type == TokenType::Identifier) // Если токен - идентификатор
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
            return std::make_shared<CallNode>(currentToken.value, arguments); // Создаём узел вызова функции
        }

        return std::make_shared<IdentifierNode>(currentToken.value); // Создаём узел идентификатора
    }
    else if (currentToken.type == TokenType::LeftParen) // Если токен - левая скобка
    {
        advance(); // Переходим к следующему токену
        auto expression = parseExpression(); // Разбираем выражение внутри скобок

        consume(TokenType::RightParen, "Expected ')' after expression"); // Проверяем наличие правой скобки
        return expression; // Возвращаем разобранное выражение
    }
    else if (currentToken.type == TokenType::Keyword)
    {
        if (currentToken.value == "true" || currentToken.value == "false") // Если токен - булевый литерал
        {
            advance(); // Переходим к следующему токену
            return std::make_shared<BooleanNode>(currentToken.value == "true"); // Создаём узел булевого значения
        }
        else if (currentToken.value == "null") // Если токен - null
        {
            advance(); // Переходим к следующему токену
            return std::make_shared<NullNode>(); // Создаём узел null
        }
    }
    
    throw std::runtime_error("Parser Error: Unknown primary expression at line " + std::to_string(currentToken.line) +
        ", column " + std::to_string(currentToken.column) +
        ": " + currentToken.value);
}