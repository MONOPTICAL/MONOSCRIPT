// expect-output: 1.500000
// expect-output: 5000000000
[i32]main()
|   final float f
|   f = 1.5
|   echo(toString_float(f))
|   final i64 w
|   w = 5000000000
|   echo(toString_long(w))
|   return 0
