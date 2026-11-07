// expect-error: 6:
[i32]id(i32: x)
|   return x

[i32]f()
|   return id

[i32]main()
|   return 0
