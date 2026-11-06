// expect-error: secret
// xfail: выборочный импорт открывает весь модуль, ast_tools.cpp
use
|> lib -> pub

[i32]main()
|   return secret()
