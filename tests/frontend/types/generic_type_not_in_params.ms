// expect-error: Type T of generic function make must appear in its parameters
[T]make<T>()
|   return 1

[i32]main()
|   return 0
