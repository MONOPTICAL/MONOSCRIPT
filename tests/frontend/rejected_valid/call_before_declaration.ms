// expect: ok
// xfail: нет прохода по объявлениям, Statements.cpp:596
[i32]main()
|   i32 x = g()
|   return x

[i32]g()
|   return 1
