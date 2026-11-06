// expect-error: explicit cast
[i64]f()
|   return 1

[i32]main()
|   i8 b = f()
|   return 0
