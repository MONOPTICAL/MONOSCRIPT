// expect: ok
// xfail: коллекцию можно инициализировать только литералом или вызовом, Utils.cpp:380
[i32]main()
|   array<array<i32>> a = [[1]]
|   array<i32> b = a[0]
|   return 0
