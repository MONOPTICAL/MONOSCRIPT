// expect-error:
// xfail: тип ключа map не проверяется, NotFinished.cpp:72
[i32]main()
|   map<string,i32> m = {"a": 1}
|   i32 x = m[5]
|   return 0
