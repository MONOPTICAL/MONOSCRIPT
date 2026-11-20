// expect-error: Struct Box expects 1 type arguments, got 2
[struct]Box<T>
|   T v

[i32]main()
|   Box<i32, u8> b = Box()
|   return 0
