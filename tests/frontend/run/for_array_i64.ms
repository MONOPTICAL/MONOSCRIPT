// expect-output: 5000000000
[i32]main()
|   array<i64> a = [5000000000]
|   for v in a
|   |   echo(toString_long(v))
|   return 0
