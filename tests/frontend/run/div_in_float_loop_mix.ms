// expect-output: 3
[i32]main()
|   i32 c = 0
|   for i in range(3)
|   |   for j in range(2)
|   |   |   c = c + (j + 1) / (j + 1)
|   |   c = c - 1
|   echo(toString_int(c))
|   return 0
