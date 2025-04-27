#include "headers/Register.h"
#include "../parser/headers/AST.h"

void Registry::addBuiltinType(const std::string& name, std::shared_ptr<TypeNode> type) {
    builtinTypes[name] = type;
}

void Registry::addBuiltinFunction(const std::string& name, std::shared_ptr<FunctionNode> func) {
    builtinFunctions[name].push_back(func);
}

void Registry::addStruct(const std::string& name, std::shared_ptr<StructNode> strct) {
    userStructs[name] = strct;
}
void Registry::addClass(const std::string& name, std::shared_ptr<ClassNode> cls) {
    userClasses[name] = cls;
}
std::shared_ptr<TypeNode> Registry::findType(const std::string& name) const {
    auto it = builtinTypes.find(name);
    if (it != builtinTypes.end()) return it->second;
    
    return nullptr;
}

std::shared_ptr<FunctionNode> Registry::findFunction(const std::string& name) const {
    auto it = builtinFunctions.find(name);
    if (it != builtinFunctions.end() && !it->second.empty()) return it->second.front();
    
    return nullptr;
}

std::shared_ptr<FunctionNode> Registry::findFunction(const std::string& name, const std::vector<std::shared_ptr<TypeNode>>& argTypes) const {
    auto it = builtinFunctions.find(name);
    if (it == builtinFunctions.end()) return nullptr;

    for (const auto& func : it->second) {
        if (func->parameters.size() != argTypes.size()) continue;
        bool match = true;
        for (size_t i = 0; i < argTypes.size(); ++i) {
            if (func->parameters[i].first->toString() != argTypes[i]->toString()) {
                match = false;
                break;
            }
        }
        if (match) return func;
    }
    return nullptr;
}

std::shared_ptr<StructNode> Registry::findStruct(const std::string& name) const {
    auto it = userStructs.find(name);
    if (it != userStructs.end()) return it->second;
    
    return nullptr;
}

std::shared_ptr<ClassNode> Registry::findClass(const std::string& name) const {
    auto it = userClasses.find(name);
    if (it != userClasses.end()) return it->second;
    
    return nullptr;
}