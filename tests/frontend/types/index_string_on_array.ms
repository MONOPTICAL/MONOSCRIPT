// expect-error:
// xfail: тип индекса массива не проверяется, NotFinished.cpp:72
[i32]main()
|   array<i32> a = [1]
|   i32 x = a["str"]
|   return 0
