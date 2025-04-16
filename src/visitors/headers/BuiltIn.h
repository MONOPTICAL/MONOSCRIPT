#ifndef BUILTIN_H
#define BUILTIN_H

#include "../../parser/headers/AST.h"
#include "Register.h"

void registerBuiltInFunctions(Registry& registry);
void registerBuiltInTypes(Registry& registry);


#endif // BUILTIN_H