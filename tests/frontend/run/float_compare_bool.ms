// expect-output: true
// expect-output: false
[i32]main()
|   float a = 1.5
|   i1 c = a < 2.5
|   i1 d = a > 2.5
|   echo(toString_bool(c))
|   echo(toString_bool(d))
|   return 0
