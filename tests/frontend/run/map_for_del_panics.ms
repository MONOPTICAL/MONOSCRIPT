// expect-runtime-error: map changed during iteration at line 4
[i32]main()
|   map<string,i32> m = {"a": 1, "b": 2}
|   for k in m
|   |   del(m, k)
|   return 0
