// expect-error: Generic function id cannot be passed as a value yet
[T]id<T>(T: x)
|   return x

[i32]apply(func: f(i32: x) -> i32)
|   return f(1)

[i32]main()
|   return apply(id)
