// expect-error: Functions can only be passed as arguments
[i32]id(i32: x)
|   return x

[i32]main()
|   g ^= [id]
|   return 0
