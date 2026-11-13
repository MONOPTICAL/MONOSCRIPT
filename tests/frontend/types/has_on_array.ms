// expect-error: has expects a map, got array<i32>
[i32]main()
|   array<i32> a = [1]
|   if has(a, 0)
|   |   return 1
|   return 0
