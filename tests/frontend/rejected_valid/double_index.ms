// expect: ok
// xfail: a[0][1] остаётся array<i32>, Logic.cpp:640
[i32]main()
|   array<array<i32>> a = [[1,2],[3,4]]
|   i32 x = a[0][1]
|   return 0
