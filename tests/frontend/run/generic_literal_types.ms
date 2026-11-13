// expect-output: 1
// expect-output: 2.5
// expect-output: s
// expect-output: true
// expect-output: 5000000000
// expect-output: q
// Литерал без контекста: 1 это i32, а не i1
[T]id<T>(T: x)
|   return x

[i32]main()
|   echo(cstr(id(1)))
|   echo(cstr(id(2.5)))
|   echo(id("s"))
|   echo(cstr(id(true)))
|   echo(cstr(id(5000000000)))
|   echo(cstr(id('q')))
|   return 0
