// expect-output: 5
[i32]f(i64: big, i32: y)
|   big = y
|   echo(toString_long(big))
|   return 0

[i32]main()
|   return f(5000000000, 5)
