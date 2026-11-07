// expect-runtime-error: key not found at line 4
[i32]main()
|   map<string,i32> m = {"a": 1}
|   return m["b"]
