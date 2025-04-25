#ifndef TOMLSTD_H
#define TOMLSTD_H

#include <string>
#include <vector>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include "../../includes/toml.hpp"
#include "CodeGenContext.h"

inline llvm::Function* declareFunctionFromTOML(
    const std::string& funcName,
    llvm::Module* module,
    llvm::LLVMContext& ctx,
    const std::string& tomlPath)
{
    const auto data = toml::parse(tomlPath);
    if (!data.contains("function")) return nullptr;

    const auto& functions = toml::find(data, "function").as_array();
    for (const auto& func : functions) {
        const auto& name = toml::find<std::string>(func, "name");
        if (name != funcName) continue;

        const auto& retStr = toml::find<std::string>(func, "ret");
        const auto& argsArr = toml::find<std::vector<std::string>>(func, "args");

        llvm::Type* retType = CodeGenContext::getLLVMType(std::make_shared<SimpleTypeNode>(retStr), ctx);
        if (!retType) return nullptr;

        std::vector<llvm::Type*> argTypes;
        for (const auto& argStr : argsArr) {
            llvm::Type* argType = CodeGenContext::getLLVMType(std::make_shared<SimpleTypeNode>(argStr), ctx);
            if (!argType) return nullptr;
            argTypes.push_back(argType);
        }

        llvm::FunctionType* ftype = llvm::FunctionType::get(retType, argTypes, false);
        return llvm::cast<llvm::Function>(module->getOrInsertFunction(name, ftype).getCallee());
    }
    return nullptr;
}

#endif // TOMLSTD_H