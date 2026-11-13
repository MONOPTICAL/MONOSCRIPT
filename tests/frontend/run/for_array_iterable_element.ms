// expect-output: 3
// expect-output: 4
[i32]main()
|   array<array<i32>> a = [[1],[3,4]]
|   for x in a[1]
|   |   echo(toString_int(x))
|   return 0
