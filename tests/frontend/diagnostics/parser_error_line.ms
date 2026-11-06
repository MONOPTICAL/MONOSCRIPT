// expect-error: 10: Unknown statement
// xfail: пустые строки выкидываются, номер считается без них, Lexer.cpp:386

// c2

[i32]main()
|   i32 x = 1


|   +++
|   return 0
