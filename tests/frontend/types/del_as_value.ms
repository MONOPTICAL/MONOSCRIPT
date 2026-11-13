// expect-error: 4:
[i32]main()
|   map<string,i32> m = {"a": 1}
|   i32 x = del(m, "a")
|   return x
