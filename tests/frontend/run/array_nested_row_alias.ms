// expect-output: 7
[i32]main()
|   array<array<i32>> a = [[1,2],[3,4]]
|   array<i32> row = a[0]
|   row[0] = 7
|   echo(toString_int(a[0][0]))
|   return 0
