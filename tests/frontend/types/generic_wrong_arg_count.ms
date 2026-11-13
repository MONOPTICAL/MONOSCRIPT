// expect-error: Function id expects 1 arguments, got 2
[T]id<T>(T: x)
|   return x

[i32]main()
|   id(1, 2)
|   return 0
