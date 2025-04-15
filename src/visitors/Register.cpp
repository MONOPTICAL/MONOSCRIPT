#include "headers/Register.h"
#include "../parser/headers/AST.h"

void Registry::addBuiltinType(const std::string& name, std::shared_ptr<TypeNode> type) {
    builtinTypes[name] = type;
}
void Registry::addBuiltinFunction(const std::string& name, std::shared_ptr<FunctionNode> func) {
    builtinFunctions[name] = func;
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
    if (it != builtinFunctions.end()) return it->second;
    
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