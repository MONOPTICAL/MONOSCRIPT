// expect-error: explicit cast
[i32]main()
|   i8 x = later()
|   return 0

[i64]later()
|   return 1
