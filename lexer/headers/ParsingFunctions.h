#ifndef PARSINGFUNCTIONS_H
#define PARSINGFUNCTIONS_H
#include <string>
#include <vector>
#include <regex>
namespace ParsingFunctions
{
    std::vector<std::string> split(const std::string str, const std::string regex_str);
}


#endif // PARSINGFUNCTIONS_H