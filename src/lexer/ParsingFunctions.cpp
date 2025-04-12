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

    std::string trim(const std::string& str)
    {
        const std::string whitespace = " \t\n\r\v\f"; 

        size_t first = str.find_first_not_of(whitespace);
        if (first == std::string::npos) return ""; // No content

        size_t last = str.find_last_not_of(whitespace);
        return str.substr(first, (last - first + 1));
    }
}