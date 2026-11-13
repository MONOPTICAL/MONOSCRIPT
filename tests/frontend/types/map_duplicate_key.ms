// expect-error: Duplicate key
[i32]main()
|   map<string,i32> m = {"a": 1, "a": 2}
|   return 0
