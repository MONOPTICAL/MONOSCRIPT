// expect-error: nope
// xfail: выражение индекса не проверяется, NotFinished.cpp:72
[i32]main()
|   array<i32> a = [1]
|   i32 x = a[nope]
|   return 0
