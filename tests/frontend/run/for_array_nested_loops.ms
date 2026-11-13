// expect-output: 10
[i32]main()
|   array<array<i32>> a = [[1,2],[3,4]]
|   i32 s = 0
|   for row in a
|   |   for x in row
|   |   |   s = s + x
|   echo(toString_int(s))
|   return 0
