// expect: ok
// xfail: структуры нет в реестре типов, Types.cpp:7
[struct]P
|   i32 age = 1

[i32]main()
|   array<P> ps = []
|   return 0
