#include "headers/Lexer.h"
#include "iostream"

Lexer::Lexer(const std::string &sourceCode)
{
    this->sourceCode = ParsingFunctions::split(sourceCode, "\n");
    this->currentLine = 1;
    this->currentIndex = -1;
    this->currentTokenType = TokenType::None;
    this->currentTokenValue = "";
    this->isStringnotFinished = false;
    this->isInMultilineComment = false;
}

void Lexer::tokenize()
{
    for(auto x : this->sourceCode)
    {   
        for(auto y : x)
        {
            currentIndex++;
            if (this->isInMultilineComment)
                if(y!='*' && peek(0)!='/')
                    continue;
                else
                    this->isInMultilineComment=false;

            TokenType type = IdentifyTokenType(y);
            TokenType keywordCheck;

            if ((y == ' ' && y != '.') && (currentTokenType != TokenType::String)) // По какой то причине C++ перенаправляет . сюда 
            {
                if(currentTokenValue!="")
                {
                    keywordCheck = IsKeyword(currentTokenValue); // Check if the current token is a keyword

                    keywordCheck==TokenType::Identifier ? addToken(currentTokenType, currentTokenValue) : addToken(keywordCheck, currentTokenValue);
                    
                    resetValues();
                }
                continue;
            }

            keywordCheck = IsKeyword(currentTokenValue); // Check if the current token is a keyword
            if(keywordCheck != TokenType::Identifier)
            {
                std::optional<char> peekChar = peek(0);
                if(peekChar.has_value() && IdentifyTokenType(peekChar.value())!=TokenType::Identifier)
                {
                    addToken(keywordCheck, currentTokenValue);
                    resetValues();
                }
            }


            if (type != currentTokenType && (currentTokenType != TokenType::None && type != TokenType::Dot)) 
            {
                if (currentTokenType == TokenType::String && type != currentTokenType && isStringnotFinished)
                {
                    currentTokenType = TokenType::String;
                    currentTokenValue += y;
                    continue;
                }

                if (currentTokenType == TokenType::Identifier && type == TokenType::Number)
                {
                    currentTokenValue += y;
                    continue; 
                }

                if(currentTokenValue.length() > 0) 
                {
                    currentIndex--;
                    addToken(currentTokenType, currentTokenValue); // Add the previous token if it has more than 1 character
                    currentIndex++;
                }

                if (currentTokenType != TokenType::String)
                {
                    currentTokenType = type;
                    currentTokenValue = y;
                    if(type == TokenType::String)
                        isStringnotFinished = true;
                    continue;
                }
            }

            if (type == TokenType::Identifier && currentTokenType == TokenType::None)
            {
                currentTokenType = type;
                currentTokenValue = y;
                continue;
            }

            else if (type == TokenType::Identifier && currentTokenType == TokenType::Identifier)
            {
                currentTokenValue += y;
                continue;
            }

            if (type == TokenType::Number && currentTokenType == TokenType::None)
            {
                currentTokenType = type;
                currentTokenValue = y;
                continue;
            }
            else if (type == TokenType::Number && currentTokenType == TokenType::Number)
            {
                currentTokenValue += y;
                continue;
            }

            if (type == TokenType::Dot && currentTokenType == TokenType::Number)
            {
                currentTokenValue += y;
                continue;
            }

            if (type == TokenType::Operator && currentTokenType == TokenType::None)
            {
                currentTokenType = type;
                currentTokenValue = y;
                continue;
            }
            else if (type == TokenType::Operator && currentTokenType == TokenType::Operator)
            {
                currentTokenValue += y;

                // Просто скипает */
                if (currentTokenValue == "*/"){resetValues();continue;}
                
                if (currentTokenValue != "//" && currentTokenValue != "/*" && currentTokenValue != ">>")
                {
                    addToken(currentTokenType, currentTokenValue); // Add the token to the current line
                    resetValues();
                    continue;
                }
                else if (currentTokenValue == ">>")
                {
                    addToken(currentTokenType, ">"); // Add the token to the current line
                    addToken(currentTokenType, ">"); // Add the token to the current line
                    resetValues();
                    continue;
                }
                else if (currentTokenValue == "/*")
                {
                    resetValues();
                    this->isInMultilineComment = true;
                    continue;
                }
                else
                {
                    resetValues();
                    break;
                }
            }

            if(type == TokenType::String && currentTokenType != TokenType::String)
            {
                currentTokenType = type;
                currentTokenValue = y;
                isStringnotFinished = true;
                continue;
            }
            else if (type == TokenType::String && currentTokenType == TokenType::String)
            {
                currentTokenValue += y;
                isStringnotFinished = false;
                
                addToken(currentTokenType, currentTokenValue); // Add the token to the current line
                resetValues();
                continue;
            }
            //IC(currentTokenValue, TokenTypeToString(currentTokenType), y);
            if(currentTokenValue.length() > 0) 
            {            
                currentIndex--;
                addToken(currentTokenType, currentTokenValue); // Add the previous token if it has more than 1 character
                currentIndex++;
            }
            currentTokenType = type;
            currentTokenValue = y;
            addToken(currentTokenType, currentTokenValue); // Add the token to the current line
            resetValues();
        }
        if(currentTokenType != TokenType::None)
            if (currentTokenType == TokenType::Identifier)
                addToken(IsKeyword(currentTokenValue), currentTokenValue); // Add the token to the current line
            else 
                addToken(currentTokenType, currentTokenValue); // Add the last token of the line to the current line
        if (isStringnotFinished)
            throw std::runtime_error("Tokenizer Error: String not finished properly at line " + std::to_string(currentLine));
        allTokens.push_back(currentTokens); // Add the current line tokens to the allTokens vector
        currentTokens.clear(); // Clear the current tokens for the next line
        currentIndex = -1; // Reset the current index for the next line
        currentTokenType = TokenType::None; // Reset the current token type for the next line
        currentTokenValue = ""; // Reset the current token value for the next line
        currentLine++; // Increment the line number
    }
    removeEmpty();
}

const std::vector<std::vector<Token>> &Lexer::getTokens() const
{
    std::cout << "\nTokens for each line:" << std::endl;
    int currentLineFor = 1;
    for (const auto &lineTokens : allTokens)
    {
        std::cout << "Line " << currentLineFor << ": ";
        for (const auto &token : lineTokens)
        {
            std::cout << "{" << TokenTypeToString(token.type) << ", \"" << token.value << "\"} ";
        }
        std::cout << std::endl;
        currentLineFor++;
    }
    return allTokens;
}

void Lexer::addToken(TokenType type, const std::string &value)
{
    std::string trimmed = ParsingFunctions::trim(value); // for safety reasons
    char first = trimmed[0]; if(first-0 < 33) return; // ёбанный компилятор сука
    //IC(trimmed);
    Token token = {type, trimmed, currentLine, currentIndex};
    currentTokens.push_back(token);
}

TokenType Lexer::IdentifyTokenType(const char &value) const
{
    if (isalpha(value) || value == '_')
        return TokenType::Identifier;
    else if (value == '.')
    {
        return TokenType::Dot;
    }
    else if (isdigit(value))
        return TokenType::Number;
    else if (value == '"')
        return TokenType::String;
    else if (value == '+' || value == '-' || value == '*' || value == '%' || value == '/' || value == '^' || value == '=' || value == '!' || value == '<' || value == '>')
        return TokenType::Operator;
    else if (value == '(')
        return TokenType::LeftParen;
    else if (value == ')')
        return TokenType::RightParen;
    else if (value == '[')
        return TokenType::LeftBracket;
    else if (value == ']')
        return TokenType::RightBracket;
    else if (value == '{')
        return TokenType::LeftBrace;
    else if (value == '}')
        return TokenType::RightBrace;
    else if (value == ',')
        return TokenType::Comma;
    else if (value == ';')
        return TokenType::Semicolon;
    else if (value == ':')
        return TokenType::Colon;
    else if (value == '|')
        return TokenType::Pipe;

    // Add more token types as needed
    return TokenType::Identifier; // Default case, can be changed as needed
}

TokenType Lexer::IsKeyword(const std::string &value) const
{
    std::vector <std::string> keywords = {
        "if",  // If statement
        "and", // Logical AND
        "or", // Logical OR
        "else",  // Else statement
        "while",  // While loop
        "for",  // For loop
        "return", // Return statement
        "break", // Break statement
        "continue", // Continue statement
        "true", // Boolean true
        "false", // Boolean false
        "null", // Null value
        "import", // Import module
        "const", // Constant declaration
        "in", // In operator for iteration
        "is", // Type check operator
        "final", // Final keyword for initialization
        "public", // Public access modifier
        "private", // Private access modifier
        "this", // Class access
        "none", // None value
        "defiend"
    }; 

    std::vector<std::string> builtinTypes = {
        "i32", "i64", "bool", "string", "void", "array", "map", "float", "struct", "class" 
    };

    std::string trimmedValue = ParsingFunctions::trim(value);

    if (std::find(keywords.begin(), keywords.end(), trimmedValue) != keywords.end())
    {
        return TokenType::Keyword;
    }

    if (std::find(builtinTypes.begin(), builtinTypes.end(), trimmedValue) != builtinTypes.end())
    {
        return TokenType::Type; // Treat builtin types as keywords
    }

    return TokenType::Identifier; // Default case, can be changed as needed
}

std::optional<char> Lexer::peek(int offset) const
{
    //IC(currentIndex, currentLine,offset);
    if (currentIndex + offset < sourceCode[currentLine-1].length())
    {
        return sourceCode[currentLine-1][currentIndex+offset];
    }
    return NULL;
}

void Lexer::resetValues()
{
    currentTokenType = TokenType::None; // Reset the current token type for the next token
    currentTokenValue = ""; // Reset the current token value for the next token
}

void Lexer::removeEmpty()
{
    this->allTokens.erase(
        std::remove_if(
            this->allTokens.begin(), this->allTokens.end(),
            [](const std::vector<Token>& token) {return token.empty();}
        ),
        this->allTokens.end()
    );
    // Сделано так а не .erase(remove(.begin(), .end(), std::vector<Token>()))
    // Для будущего а то вдруг, хуй знает
}