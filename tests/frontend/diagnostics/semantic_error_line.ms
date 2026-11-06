// expect-error: 10: Variable not found: y
// xfail: пустые строки выкидываются, номер считается без них, Lexer.cpp:386

// c2

[i32]main()
|   i32 x = 1


|   y = 2
|   return 0
