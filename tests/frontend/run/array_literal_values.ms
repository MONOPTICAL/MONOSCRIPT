// expect-output: 1
// expect-output: 127
// expect-output: 128
// expect-output: 5000000000
[i32]main()
|   array<i64> a = [1,127,128,5000000000]
|   for i in range(4)
|   |   echo(toString_long(a[i]))
|   return 0
