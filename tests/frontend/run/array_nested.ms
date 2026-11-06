// expect-output: 3
// expect-output: 9
// expect-output: 2
[i32]main()
|   array<array<i32>> a = [[1,2],[3,4]]
|   echo(toString_int(a[1][0]))
|   a[1][1] = 9
|   echo(toString_int(a[1][1]))
|   echo(toString_int(len(a[0])))
|   return 0
