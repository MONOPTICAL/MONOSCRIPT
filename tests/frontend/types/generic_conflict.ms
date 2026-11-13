// expect-error: T is both i32 and string
[T]same<T>(T: a, T: b)
|   return a

[i32]main()
|   i32 x = 1
|   string s = "s"
|   echo(same(x, s))
|   return 0
