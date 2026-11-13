// expect-output: 5000000
[i32]main()
|   i32 c = 0
|   for i in range(5000000)
|   |   i32 y = 1
|   |   c = c + y
|   echo(toString_int(c))
|   return 0
