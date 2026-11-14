// expect-output: 1024
// expect-output: 512
// expect-output: 18
// expect-output: 81
// expect-output: 1
// expect-output: -8
// expect-output: 1099511627776
// expect-output: 1.4142135
[i32]main()
|   i32 a = 2 ** 10
|   echo(toString_int(a))
|   echo(toString_int(2 ** 3 ** 2))
|   echo(toString_int(2 * 3 ** 2))
|   i32 b = 3
|   i32 e = 4
|   echo(toString_int(b ** e))
|   echo(toString_int(b ** 0))
|   echo(toString_int((0 - 2) ** 3))
|   i64 big = 2
|   echo(cstr(big ** 40))
|   float f = 2.0 ** 0.5
|   echo(cstr(f))
|   return 0
