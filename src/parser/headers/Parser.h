#include "../../lexer/headers/Token.h"
#include "../../includes/icecream.hpp"
#include "AST.h"
#include <memory>
#include <stdexcept>
#include <string_view>
#include <iostream>
#include <optional>
#include <algorithm>

class Parser {
public:
    Parser(const std::vector<std::vector<Token>>& tokens, std::string module) : 
    moduleName(module), lines(tokens), lineIndex(0), tokenIndex(0)
    {
        this->currentNode = nullptr;
    };

    std::shared_ptr<ProgramNode> parse();

private:
    std::vector<std::vector<Token>> lines; // вектор токенов
    std::string moduleName; // имя модуля
    int lineIndex; // индекс текущей строки токенов
    int tokenIndex; // индекс текущего токена в строке
    int currentIndent; // текущий уровень отступа
    bool isPipe;
    std::shared_ptr<ASTNode> currentNode; // указатель на текущий узел AST

    // указатель на текущий токен
    Token current(); // возвращает текущий токен
    Token advance(); // переходит к следующему токену
    Token peek(); // возвращает следующий токен без перехода к нему
    bool nextLine(); // переходит к следующей строке токенов
    bool match(TokenType type); // проверяет, совпадает ли текущий токен с заданным типом
                                // и если да, переходит к следующему токену
    bool check(TokenType type); // проверяет, является ли текущий токен заданного типа
                                // и если да, возвращает true, без перехода к следующему токену
    // парсеры
    std::shared_ptr<ASTNode> parseStatement();
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<FunctionNode> parseFunction();
    std::shared_ptr<BlockNode> parseBlock(int);
    std::shared_ptr<ASTNode> parseAssignment(bool);
    std::shared_ptr<ASTNode> parseIf();
    std::shared_ptr<ASTNode> parseFor();
    std::shared_ptr<ASTNode> parseWhile();
    std::shared_ptr<ASTNode> parseReturn();
    std::shared_ptr<ASTNode> parseCall();
    std::shared_ptr<ASTNode> parseStruct();

    std::shared_ptr<ASTNode> parseUse();

    std::shared_ptr<ASTNode> parseCast(std::shared_ptr<ASTNode> expression);

/*
все возможные случаи это
_______________________________________
Объявление функции: (есть)
[array<i32>]bubbleSort(array<i32>: arr)
[Type]functionName([Type]: variableName ...)

Объявление переменной: (есть)
i32 i = 0 или i ^= 0
Type variableName = expression или variableName ^= expression

Изменение переменной: (есть)
i = 0
variableName = expression

Условия: (есть)
if (expression) или if expression
| Body              | Body
else if             else if
| Body              | Body
else                else
| Body              | Body

Циклы: (есть)
for [iteration var name] in [variable]
| Body

Вызов функции: (есть)
functionName([expression], [expression], ...)

Возврат значения: (есть)
return [expression]

*/
    // выражения
    std::shared_ptr<ASTNode> parseBinary(int precedence = 0);
    std::shared_ptr<ASTNode> parseUnary();
    std::shared_ptr<ASTNode> parsePrimary();
    std::shared_ptr<ASTNode> parseMemberExpression();
    
    void parseDotNotation(std::shared_ptr<AccessExpression>);
    void parseArrayNotation(std::shared_ptr<AccessExpression>);

    // вспомогательное
    void consume(TokenType, const std::string& errMsg);
    bool isEndOfFile() const; 
    bool isEndOfLine() const; // проверяет, достигнут ли конец строки
    int getIndentLevel(const std::vector<Token>& line);
    Token getLastTokenInCurrentLine() const;
    int getPrecedence(const Token& token) const;
    void throwError(const std::string& errMsg);

    std::shared_ptr<TypeNode> getFullType();
};
    