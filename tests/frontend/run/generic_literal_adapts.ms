// expect-output: 5000000000
// expect-output: 1
// expect-output: 2.5
// Литерал подстраивается под значение настоящего типа, в любом порядке
[T]big<T>(T: a, T: b)
|   if a > b
|   |   return a
|   return b

[i32]main()
|   i64 w = 5000000000
|   echo(cstr(big(w, 1)))
|   i8 s = 0
|   i8 r = big(1, s)
|   echo(cstr(r))
|   f64 d = 2.5
|   echo(cstr(big(1, d)))
|   return 0
