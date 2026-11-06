// expect: ok
// xfail: счётчик x ^= 0 получает тип i1, Logic.cpp:323
[i32]main()
|   x ^= 0
|   x = 1000
|   return 0
