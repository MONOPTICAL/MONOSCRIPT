// expect-output: 1
// expect-output: -7
// expect-output: 2.5
// expect-output: true
// expect-output: 3
// expect-output: false
// expect-output: abc
// Литерал 1 это число, а не true
[i32]main()
|   echo(cstr(1))
|   echo(cstr(-7))
|   echo(cstr(2.5))
|   echo(cstr(true))
|   echo(cstr(1 + 2))
|   echo(cstr(1 > 2))
|   echo(cstr("abc"))
|   return 0
