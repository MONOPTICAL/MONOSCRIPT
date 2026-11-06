// expect-error: expected string, got i32
[i32]main()
|   array<i32> arr = [1,2]
|   for x in arr
|   |   string s = x
|   return 0
