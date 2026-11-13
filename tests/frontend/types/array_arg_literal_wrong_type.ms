// expect-error: 6:
[i32]f(array<i32>: a)
|   return 0

[i32]main()
|   return f(["x"])
