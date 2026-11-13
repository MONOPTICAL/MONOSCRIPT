// expect-output: 9
[i32]main()
|   i32 k = 0
|   while k < 2
|   |   i32 y = k
|   |   k = k + 1
|   i32 y = 9
|   echo(toString_int(y))
|   return 0
