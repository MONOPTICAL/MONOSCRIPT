// expect-output: 3
// expect-output: 2
[i32]main()
|   for i in range(3)
|   |   i32 y = i + 1
|   |   if y == 3
|   |   |   echo(toString_int(y))
|   for i in range(2)
|   |   i32 y = i + 1
|   |   if y == 2
|   |   |   echo(toString_int(y))
|   return 0
