// expect-output: 0
// expect-output: 1
[array<T>]orEmpty<T>(T: dummy, array<T>: a)
|   return a

[i32]main()
|   array<i32> e = orEmpty(1, [])
|   echo(cstr(len(e)))
|   array<i32> f = orEmpty(1, [5])
|   echo(cstr(len(f)))
|   return 0
