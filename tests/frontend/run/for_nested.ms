// expect-output: 6
[i32]main()
|   i32 c = 0
|   for i in range(2)
|   |   for j in range(3)
|   |   |   c = c + 1
|   echo(toString_int(c))
|   return 0
