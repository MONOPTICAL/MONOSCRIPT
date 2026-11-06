// expect: ok
// expect-quiet
// xfail: без отладки печатаются ic| и дамп AST, Statements.cpp:252
[i32]main()
|   map<string,i32> m = {"a": 1}
|   return 0
