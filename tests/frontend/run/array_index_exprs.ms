// expect-output: 30
// expect-output: 20
// expect-output: 10
[i32]main()
|   array<i32> a = [10,20,30]
|   i64 w = 2
|   i8 n = 1
|   echo(toString_int(a[w]))
|   echo(toString_int(a[n]))
|   echo(toString_int(a[1-1]))
|   return 0
