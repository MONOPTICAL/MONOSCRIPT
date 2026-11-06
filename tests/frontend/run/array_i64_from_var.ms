// expect-output: 7
[i32]main()
|   i32 x = 7
|   array<i64> a = [x]
|   echo(toString_long(a[0]))
|   return 0
