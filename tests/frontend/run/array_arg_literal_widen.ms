// expect-output: -1
// expect-output: 5000000000
[i32]f(array<i64>: a)
|   echo(toString_long(a[0]))
|   echo(toString_long(a[1]))
|   return 0

[i32]main()
|   return f([-1,5000000000])
