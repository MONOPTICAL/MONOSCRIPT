#include "./headers/Parser.h"
#include "Parser.h"

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
    if (!isEndOfFile() && tokenIndex < lines[lineIndex].size()) {
        return lines[lineIndex][tokenIndex];
    }
    return Token{TokenType::None, "", -1, -1};
}

/// @brief Переходит к следующему токену
/// @details Если индекс токена меньше размера токенов, увеличивает индекс токена на 1 и возвращает текущий токен
/// @return Текущий токен, если индекс токена меньше размера токенов, иначе возвращает токен с типом None и пустым значением
Token Parser::advance()
{
    if (!isEndOfFile() && tokenIndex < lines[lineIndex].size())
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
    if (tokenIndex >= lines[lineIndex].size()) return false;

    return lines[lineIndex][tokenIndex].type == type;
}

/// @brief Возвращает следующий токен без перехода к нему
/// @details Если индекс токена меньше размера токенов, возвращает следующий токен, иначе возвращает токен с типом None и пустым значением
/// @return Следующий токен, если индекс токена меньше размера токенов, иначе возвращает токен с типом None и пустым значением
Token Parser::peek()
{
    if (!isEndOfFile() && tokenIndex+1 < lines[lineIndex].size())
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

/// @brief Проверяет, совпадает ли текущий токен с заданным типом и переходит к следующему токену, если совпадает
/// @param Token Тип токена, с которым нужно сравнить текущий токен
/// @param errMsg Сообщение об ошибке, если токен не совпадает с заданным типом
/// @throws std::runtime_error Если токен не совпадает с заданным типом, выбрасывает исключение с сообщением об ошибке
/// @details Если текущий токен совпадает с заданным типом, переходит к следующему токену. Иначе выбрасывает исключение с сообщением об ошибке
void Parser::consume(TokenType Token, const std::string &errMsg)
{
    if (match(Token)) return;

    throw std::runtime_error("Error: " + errMsg +
        " at line " + std::to_string(lineIndex));
}

/// @brief  Проверяет, достигнут ли конец файла
/// @details Если индекс строки больше или равен размеру токенов, возвращает true
/// @return  true, если достигнут конец файла, иначе false
bool Parser::isEndOfFile() const
{
    return lineIndex >= lines.size();
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

/// @brief Получает приоритет токена
/// @param token Токен, для которого нужно получить приоритет
/// @return Все возможные приоритеты токенов: or - 1, and - 2, == - 3, + и - - 4, * и / - 5. Если токен не является оператором или ключевым словом, возвращает -1
int Parser::getPrecedence(const Token &token) const
{
    if (token.type != TokenType::Operator || token.type != TokenType::Keyword) return -1;
    if (token.value == "or") return 1; // keyword
    if (token.value == "and") return 2; // keyword
    if (token.value == "==") return 3; // operator
    if (token.value == "+" || token.value == "-") return 4; // operator
    if (token.value == "*" || token.value == "/") return 5; // operator
}

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
        if (statement) program->body.push_back(statement);
        if (!nextLine()) break; // если не удалось перейти к следующей строке, выходим из цикла
    }
    return program;
}

/// @brief
/// @details
/// @return 
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
        return parseAssignment();
    }
    // Static variable declaration
        // [Type] [variableName] = [expression] or [variableName] ^= [expression]
        // i32 i = 0 or i ^= 0
        // ↑ if we have type, and next token is identifier, we have static variable declaration
    else if (currentToken.type == TokenType::Type && peek().type == TokenType::Identifier)
    {
        return parseAssignment();
    }
    // Function call
        // [functionName]([expression], [expression], ...)
        // bubbleSort(arr, 0, 10)
        // ↑ if we have identifier, and next token is left parenthesis, we have function call
    else if (currentToken.type == TokenType::Identifier && peek().type == TokenType::LeftParen)
    {
        return parseCallOrVariable();
    }
    else
    {
        std::runtime_error("Parser Error: Unknown statement at line " + std::to_string(currentToken.line) +
            ", column " + std::to_string(currentToken.column) +
            ": " + currentToken.value);
    }
}

std::shared_ptr<ASTNode> Parser::parseExpression()
{
    return parseBinary();
}

std::shared_ptr<ASTNode> Parser::parseBinary(int precedence)
{
    auto left = parseUnary();

    while (true)
    {
        Token currentToken = current(); // + текущий токен, == текущий токен
        int currentPrecedence = getPrecedence(currentToken); // Получаем приоритет текущего токена

        if (currentPrecedence < precedence) break; // Если текущий приоритет меньше, чем заданный, выходим из цикла

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
        return std::make_shared<NumberNode>(currentToken.value); // Создаём узел цифры
    }
    else if (currentToken.type == TokenType::String) // Если токен - строка
    {
        advance(); // Переходим к следующему токену
        return std::make_shared<StringNode>(currentToken.value); // Создаём узел строки
    }
    else if (currentToken.type == TokenType::Identifier) // Если токен - идентификатор
    {
        advance(); // Переходим к следующему токену
        return std::make_shared<IdentifierNode>(currentToken.value); // Создаём узел идентификатора
    }
    else if (currentToken.type == TokenType::LeftParen) // Если токен - левая скобка
    {
        advance(); // Переходим к следующему токену
        auto expression = parseExpression(); // Разбираем выражение внутри скобок
        consume(TokenType::RightParen, "Expected ')' after expression"); // Проверяем наличие правой скобки
        return expression; // Возвращаем разобранное выражение
    }
    throw std::runtime_error("Parser Error: Unknown primary expression at line " + std::to_string(currentToken.line) +
        ", column " + std::to_string(currentToken.column) +
        ": " + currentToken.value);
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

    auto thenBlock = parseBlock();
    std::shared_ptr<BlockNode> elseBlock = nullptr;
    if (check(TokenType::Keyword) && peek().value == "else")
    {
        advance(); // consume 'else'
        elseBlock = parseBlock();
    }
    return std::make_shared<IfNode>(condition, thenBlock, elseBlock);
}