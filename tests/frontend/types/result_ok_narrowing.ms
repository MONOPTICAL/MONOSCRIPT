// expect-error: use explicit cast
[result<i8>]f()
|   i32 x = 1
|   return ok(x)

[i32]main()
|   return 0
