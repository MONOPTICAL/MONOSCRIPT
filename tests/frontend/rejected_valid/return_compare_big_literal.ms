// expect: ok
// xfail: сравнение кастуется в i1 вместе с операндами, Statements.cpp:325
[i1]f(i32: a)
|   return a > 1000

[i32]main()
|   return 0
