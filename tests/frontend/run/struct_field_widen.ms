// expect-output: 7
// expect-output: 2.0
[struct]T
|   i64 big = 1
|   float f = 1

[i32]main()
|   T t = T()
|   i32 x = 7
|   t.big = x
|   t.f = 2
|   echo(toString_long(t.big))
|   echo(toString_float(t.f))
|   return 0
