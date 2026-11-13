// expect-error: has expects 2 arguments
[i32]main()
|   map<string,i32> m = {}
|   if has(m)
|   |   return 1
|   return 0
