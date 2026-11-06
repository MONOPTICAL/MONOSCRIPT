// expect: ok
// xfail: пустой литерал имеет тип none и не подходит к return, Statements.cpp:333
[array<i32>]f()
|   return []

[i32]main()
|   return 0
