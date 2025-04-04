#include "headers/Lexer.h"
#include "iostream"
Lexer::Lexer(const std::string &sourceCode)
{
    this->sourceCode = ParsingFunctions::split(sourceCode, "\n");
    std::cout << "Source code:" << std::endl;
    for (const auto &line : this->sourceCode)
    {
        std::cout << line << std::endl;
    }
    this->currentLine = 1;
    this->currentIndex = 0;
    this->currentTokenType = TokenType::None;
    this->currentTokenValue = "";
    this->isStringnotFinished = false;

}

void Lexer::tokenize()
{
    for(auto x : this->sourceCode)
    {   
        for(auto y : x)
        {
            currentIndex++;
            if (isspace(y))
            {
                continue;
            }
            else
            {
                TokenType type = IdentifyTokenType(y); // LeftParen
                //std::cout << "\nValue: " << y << "\nToken type: " << TokenTypeToString(type) << "\nCurrent token type: " << TokenTypeToString(currentTokenType) << std::endl; 
                if(IsKeyword(currentTokenValue) != TokenType::Identifier)
                {
                    addToken(TokenType::Keyword, currentTokenValue);
                    currentTokenType = TokenType::None; // Reset the current token type for the next token
                    currentTokenValue = ""; // Reset the current token value for the next token
                }

                if (type != currentTokenType && currentTokenType != TokenType::None) // <--
                {
                    if (currentTokenType == TokenType::String && type != TokenType::String && isStringnotFinished)
                    {
                        currentTokenType = TokenType::String;
                        currentTokenValue += y;
                        //std::cout << "String not finished" << std::endl;
                        continue;
                    }

                    if (currentTokenType == TokenType::Identifier && type == TokenType::Number)
                    {
                        currentTokenValue += y;
                        continue; 
                    }

                    if (type != TokenType::String && type != TokenType::Identifier)
                    {
                        if(currentTokenValue.length() > 0) 
                        {
                            currentIndex--;
                            addToken(currentTokenType, currentTokenValue); // Add the previous token if it has more than 1 character
                            currentIndex++;
                            //std::cout << "1)Current token value:" << currentTokenValue << std::endl;
                            //std::cout << "1)Current token type:" << TokenTypeToString(currentTokenType) << std::endl;
                        }
                        currentTokenType = type;
                        currentTokenValue = y;
                        addToken(currentTokenType, currentTokenValue);
                        //std::cout << "2)Current token value:" << currentTokenValue << std::endl;
                        //std::cout << "2)Current token type:" << TokenTypeToString(currentTokenType) << std::endl;
                        currentTokenType = TokenType::None; // Reset the current token type for the next token
                        currentTokenValue = ""; // Reset the current token value for the next token
                        continue;
                    }
                    else
                    {
                        currentTokenType = type;
                        currentTokenValue = y;
                        continue;
                    }
                }

                if (type == TokenType::Identifier && currentTokenType == TokenType::None)
                {
                    currentTokenType = type;
                    currentTokenValue = y;
                    //std::cout << "5)Current token value:" << currentTokenValue << std::endl;
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
                    //std::cout << "3)Current token value:" << currentTokenValue << std::endl;
                    //std::cout << "3)Current token type:" << TokenTypeToString(currentTokenType) << std::endl;
                    currentTokenType = TokenType::None;
                    continue;
                }
                //std::cout << "Default case" << std::endl;
                currentTokenType = type;
                currentTokenValue = y;
                addToken(currentTokenType, currentTokenValue); // Add the token to the current line
                currentTokenType = TokenType::None; // Reset the current token type for the next token
                currentTokenValue = ""; // Reset the current token value for the next token
                //std::cout << "4)Current token value:" << currentTokenValue << std::endl;
                //std::cout << "4)Current token type:" << TokenTypeToString(currentTokenType) << std::endl;
            }
        }
        if(currentTokenType != TokenType::None)
            addToken(currentTokenType, currentTokenValue); // Add the last token of the line to the current line
            
        allTokens.push_back(currentTokens); // Add the current line tokens to the allTokens vector
        currentTokens.clear(); // Clear the current tokens for the next line
        currentIndex = 0; // Reset the current index for the next line
        currentTokenType = TokenType::None; // Reset the current token type for the next line
        currentTokenValue = ""; // Reset the current token value for the next line
        currentLine++; // Increment the line number
    }
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
    Token token = {type, value, currentLine, currentIndex};
    currentTokens.push_back(token);
}

TokenType Lexer::IdentifyTokenType(const char &value) const
{
    if (isalpha(value) || value == '_')
        return TokenType::Identifier;
    else if (isdigit(value))
        return TokenType::Number;
    else if (value == '"')
        return TokenType::String;
    else if (value == '+' || value == '-' || value == '*' || value == '/' || value == '^' || value == '=' || value == '!' || value == '<' || value == '>')
        return TokenType::Operator;
    else if (value == '(')
        return TokenType::LeftParen;
    else if (value == ')')
        return TokenType::RightParen;
    else if (value == '[')
        return TokenType::LeftBracket;
    else if (value == ']')
        return TokenType::RightBracket;
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
        "echo",
        "if", 
        "and",
        "or",
        "else", 
        "while", 
        "for", 
        "return",
        "break",
        "continue",
        "true",
        "false",
        "null",
        "import",
        "const"
    }; 
    if (std::find(keywords.begin(), keywords.end(), value) != keywords.end())
    {
        return TokenType::Keyword;
    }
    return TokenType::Identifier; // Default case, can be changed as needed
}
