// expect: ok
// xfail: переменная цикла не попадает в контекст, Statements.cpp:494
[i32]main()
|   array<i32> arr = [1,2]
|   for i in arr
|   |   i32 y = i
|   return 0
