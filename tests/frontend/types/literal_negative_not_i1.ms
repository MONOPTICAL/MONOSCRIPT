// expect: ok
// xfail: любой унарный минус выводит i1, Statements.cpp:190
[i32]main()
|   x ^= -5
|   x = 100
|   return 0
