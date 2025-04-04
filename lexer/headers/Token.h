#ifndef TOKEN_H
#define TOKEN_H

#include <string>
enum class TokenType { 
    None, 
    Identifier, 
    Type,
    Number, 
    String, 
    Operator, 
    Keyword, 
    LeftParen, 
    RightParen, 
    LeftBracket,
    RightBracket,
    Comma, 
    Semicolon,
    Colon,
    Pipe
};
/*
Identifier: Represents variable names, function names, etc.
Number: Represents numeric literals (integers, floats, etc.)
String: Represents string literals (enclosed in quotes).
Operator: Represents operators (e.g., +, -, *, /).
Keyword: Represents reserved keywords (e.g., if, else, while).
LeftParen: Represents the left parenthesis '('.
RightParen: Represents the right parenthesis ')'.
Comma: Represents the comma ','.

*/

/*
Identifiers:
echo
sum

*/
/*
Operators:
Сейчас + - * / ^= = 
Потом != == < > <= >= && ++ --
*/

/*
a ^= 1;
Tokenized as:
Identifier: a
Operator: ^=
Number: 1

*/
struct Token {
    TokenType type;
    std::string value;
    int line, column;
};

inline std::string TokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TokenType::None: return "None";
    case TokenType::Identifier: return "Identifier";
    case TokenType::Number: return "Number";
    case TokenType::String: return "String";
    case TokenType::Operator: return "Operator";
    case TokenType::LeftParen: return "LeftParen";
    case TokenType::RightParen: return "RightParen";
    case TokenType::LeftBracket: return "LeftBracket";
    case TokenType::RightBracket: return "RightBracket";
    case TokenType::Comma: return "Comma";
    case TokenType::Semicolon: return "Semicolon";
    case TokenType::Colon: return "Colon";
    case TokenType::Keyword: return "Keyword";
    case TokenType::Pipe: return "Pipe";
    case TokenType::Type: return "Type";
    default: return "Unknown";
    }
}

#endif // TOKEN_H