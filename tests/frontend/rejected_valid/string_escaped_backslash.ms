// expect: ok
// xfail: строка, заканчивающаяся на экранированный обратный слеш, не закрывается, Lexer.cpp:29
[i32]main()
|   string s = "a\\"
|   return 0
