// expect-output: 2.500000
// expect-output: 5000000000
// expect-output: true
[void]f(float: a, i64: b, i1: c)
|   echo(toString_float(a))
|   echo(toString_long(b))
|   echo(toString_bool(c))

[i32]main()
|   f(2.5, 5000000000, 1 == 1)
|   return 0
