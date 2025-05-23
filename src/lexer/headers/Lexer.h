#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <optional>
#include "Token.h"
#include "ParsingFunctions.h"
#include "../../includes/icecream.hpp"

class Lexer
{
public:
    Lexer(const std::string& sourceCode);
    void tokenize();
    void printTokens();
    const std::vector<std::vector<Token>>& getTokens() const; // Returns a vector of vectors of tokens
    /* Token Vector:
    "i ^= 4"
    → 
    [
        {Identifier, "i"},
        {Operator, "^="},
        {Number, "4"}
    ]
    "echo(sum(2+i))"
    → 
    [
        {Keyword, "echo"},
        {LeftParen, "("},
        {Identifier, "sum"},
        {LeftParen, "("},
        {Number, "2"},
        {Operator, "+"},
        {Identifier, "i"},
        {RightParen, ")"},
        {RightParen, ")"}
    ]

    And then the lexer will return a vector of these vectors, where each vector represents a line of code.
    For example, if the input code has 3 lines, the lexer will return a vector of 3 vectors of tokens.
    Then all this will be passed to the parser, which will parse each line separately.
    */
    
private:
    std::vector<std::string> sourceCode;
    std::vector<Token> currentTokens;
    std::vector<std::vector<Token>> allTokens; // Vector of vectors to store tokens for each line
    std::vector<char> firstLettersInBuiltTypes;
    int currentLine;
    int currentIndex;
    TokenType currentTokenType;
    std::string currentTokenValue;
    bool isStringnotFinished = false;
    bool isInMultilineComment = false;
    bool isInMultilineExpression = false;

    void addToken(TokenType type, const std::string& value);
    TokenType IdentifyTokenType(const char& value) const;
    TokenType IsKeyword(const std::string& value) const;
    std::optional<char> peek(int offset) const;
    void resetValues();
    void removeEmpty();
};

#endif // LEXER_H