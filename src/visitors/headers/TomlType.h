#pragma once
#include "../../parser/headers/AST.h"
#include <memory>
#include <string>

// Тип из mono.toml: "i32", "array<string>", "map<string, array<i32>>"
inline std::shared_ptr<TypeNode> parseTomlType(const std::string& text) {
    const size_t open = text.find('<');
    if (open == std::string::npos || text.back() != '>') {
        return std::make_shared<SimpleTypeNode>(text);
    }

    auto generic = std::make_shared<GenericTypeNode>(text.substr(0, open));
    int depth = 0;
    size_t start = open + 1;
    for (size_t i = open + 1; i < text.size(); i++) {
        const char c = text[i];
        if (c == '<') depth++;
        if (c == '>' && depth > 0) { depth--; continue; }
        if ((c == ',' && depth == 0) || i == text.size() - 1) {
            size_t first = start;
            while (first < i && text[first] == ' ') first++;
            generic->typeParameters.push_back(parseTomlType(text.substr(first, i - first)));
            start = i + 1;
        }
    }
    return generic;
}
