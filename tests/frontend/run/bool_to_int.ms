// expect-output: 1
// expect-output: 0
[i32]main()
|   i32 a = 7
|   i32 y = a > 5
|   i32 z = a < 5
|   echo(toString_int(y))
|   echo(toString_int(z))
|   return 0
