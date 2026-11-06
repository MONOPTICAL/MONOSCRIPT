// expect-parse: (var x (+ 1 (- 2)))
// xfail: +- склеивается, остаток строки теряется, Lexer.cpp:164
[i32]main()
|   i32 x = 1+-2
|   return 0
