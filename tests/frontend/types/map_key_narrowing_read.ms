// expect-error: use explicit cast
[i32]main()
|   map<i32,i32> m = {}
|   i64 k = 5000000000
|   return m[k]
