// expect-output: 1
// expect-output: x
[T]id<T>(T: x)
|   return x

[T]head<T>(array<T>: a)
|   return id(a[0])

[T]twice<T>(T: x)
|   return id(id(x))

[i32]main()
|   echo(cstr(head([1,2])))
|   echo(twice("x"))
|   return 0
