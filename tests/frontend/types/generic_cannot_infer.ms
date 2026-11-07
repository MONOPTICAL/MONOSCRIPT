// expect-error: Cannot infer type T of f from its arguments
[i32]f<T>(array<T>: a)
|   return 1

[i32]main()
|   return f([])
