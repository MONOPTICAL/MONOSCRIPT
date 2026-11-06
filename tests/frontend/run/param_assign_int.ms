// expect-output: 2
// expect-output: 1
[i32]f(i32: a)
|   a = a + 1
|   return a

[i32]main()
|   i32 v = 1
|   echo(toString_int(f(v)))
|   echo(toString_int(v))
|   return 0
