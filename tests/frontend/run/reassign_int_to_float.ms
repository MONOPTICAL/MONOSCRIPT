// expect-output: 5.0
// expect-output: 2.0
[i32]main()
|   float f = 1.5
|   i32 y = 5
|   f = y
|   echo(toString_float(f))
|   f = 2
|   echo(toString_float(f))
|   return 0
