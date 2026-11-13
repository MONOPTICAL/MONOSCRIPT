// expect-error: 6: expected array<i32>, got null
[i32]f(array<i32>: a)
|   return 0

[i32]main()
|   return f(null)
