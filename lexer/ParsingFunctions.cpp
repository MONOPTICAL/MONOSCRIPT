#include "headers/ParsingFunctions.h"

namespace ParsingFunctions
{
    std::vector<std::string> split(const std::string str, const std::string regex_str)
    {
        try
        {
                std::regex regexz(regex_str);
                return {std::sregex_token_iterator(str.begin(), str.end(), regexz, -1), std::sregex_token_iterator()};
        }
        catch(const std::exception& e)
        {
                return {};
        }
    }

}