// expect-error: has, key:
[i32]main()
|   map<string,i32> m = {}
|   if has(m, 1)
|   |   return 1
|   return 0
