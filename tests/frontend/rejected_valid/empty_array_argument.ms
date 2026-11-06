// expect: ok
// xfail: пустой литерал имеет тип none и не подходит к параметру array, Statements.cpp:639
[array<i32>]id(array<i32>: a)
|   return a

[i32]main()
|   array<i32> b = id([])
|   return 0
