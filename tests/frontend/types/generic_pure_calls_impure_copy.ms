// expect-error: Pure function cannot call impure function
[T]id<T>(T: x)
|   return x

[i32]f(i32: a) @pure
|   return id(a)

[i32]main()
|   return 0
