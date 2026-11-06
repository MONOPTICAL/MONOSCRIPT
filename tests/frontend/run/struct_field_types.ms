// expect-output: 5000000000
// expect-output: 1.500000
// expect-output: true
// expect-output: -3
// expect-output: 300
[struct]T
|   i64 big = 5000000000
|   float f = 1.5
|   i1 flag = 1
|   i8 small = -3
|   i16 mid = 300

[i32]main()
|   T t = T()
|   echo(toString_long(t.big))
|   echo(toString_float(t.f))
|   echo(toString_bool(t.flag))
|   i32 s = t.small
|   i32 m = t.mid
|   echo(toString_int(s))
|   echo(toString_int(m))
|   return 0
