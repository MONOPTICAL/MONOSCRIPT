// expect-runtime-error: map changed during iteration at line 4
[i32]main()
|   map<string,i32> m = {"a": 1}
|   for k in m
|   |   m["new"] = 2
|   return 0
