// expect-error: 7:
[T]id<T>(T: x)
|   return x

[i32]main()
|   string s = "s"
|   i32 n = id(s)
|   return n
