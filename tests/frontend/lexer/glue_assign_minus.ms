// expect-parse: (var x (- 1))
// xfail: =- склеивается в один оператор, x остаётся без значения, Lexer.cpp:164
[i32]main()
|   i32 x=-1
|   return 0
