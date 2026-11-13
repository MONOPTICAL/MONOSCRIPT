// expect-error:
[T]id<T>(T: x)
|   return x

[i32]main()
|   return id<i32>(1)
