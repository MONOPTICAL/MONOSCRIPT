#ifndef TOKEN_H
#define TOKEN_H

#include <string>
enum class TokenType { 
    None,                   // none - это не токен, а просто значение по умолчанию
    Identifier,             // identifier - имя переменной или функции
    Type,                   // type - тип переменной 
    Number,                 // number - число
    String,                 // string - строка
    Operator,               // operator - оператор (например, +, -, *, /, %, ^, =, !=, <, >)
    Keyword,                // keyword - ключевое слово (например, if, else, while, for, return)
    LeftParen,              // left parenthesis - левая скобка ( для выражений и функций 
    RightParen,             // right parenthesis - правая скобка ) для выражений и функций
    LeftBracket,            // [ - левая квадратная скобка [ для массивов
    RightBracket,           // ] - правая квадратная скобка ] для массивов
    LeftBrace,              // { - левая фигурная скобка { для map
    RightBrace,             // } - правая фигурная скобка } для map
    Comma,                  // , - запятая , для разделения аргументов в функциях и массивов
    Semicolon,              // ; - точка с запятой ; для разделения выражений
    Colon,                  // : - двоеточие : для разделения ключа и значения в map
    Pipe,                   // | - вертикальная черта | для обозначения блока
    Dot,                    // . - точка . для доступа к полям объекта
    PipeArrow,              // |> - оператор для передачи результата функции в другую функцию и импорта модулей
    Arrow,                  // -> - стрелка для указания на указатель
};

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
    case TokenType::Dot: return "Dot";
    case TokenType::LeftBrace: return "LeftBrace";
    case TokenType::RightBrace: return "RightBrace";
    case TokenType::PipeArrow: return "PipeArrow";
    case TokenType::Arrow: return "Arrow";
    default: return "Unknown";
    }
}

#endif // TOKEN_H