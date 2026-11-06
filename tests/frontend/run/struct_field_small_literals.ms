// expect-output: 0
// expect-output: -1
[struct]T
|   i32 a = 5
|   i32 b = 5

[i32]main()
|   T t = T()
|   t.a = 0
|   t.b = -1
|   echo(toString_int(t.a))
|   echo(toString_int(t.b))
|   return 0
