// expect-error: 7: use explicit cast
[i32]f(array<i8>: a)
|   return 0

[i32]main()
|   i32 x = 1
|   return f([x])
