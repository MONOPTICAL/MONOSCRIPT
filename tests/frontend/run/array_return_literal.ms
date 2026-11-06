// expect-output: 7
// expect-output: -1
[array<i64>]mk()
|   return [7,-1]

[i32]main()
|   array<i64> a = mk()
|   echo(toString_long(a[0]))
|   echo(toString_long(a[1]))
|   return 0
