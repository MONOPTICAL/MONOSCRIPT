// expect: ok
// xfail: все модули сливаются в один AST, ast_tools.cpp
use
|> lib -> pub

[i32]secret()
|   return 3

[i32]main()
|   return secret()
