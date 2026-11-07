// expect-output: 2
// expect-output: a
[T]keep<T, U>(T: t, U: u)
|   return t

[U]other<T, U>(T: t, U: u)
|   return u

[i32]main()
|   echo(cstr(keep(2, "x")))
|   echo(other(1, "a"))
|   return 0
