// expect-output: 7
[i32]main()
|   i32 c = 0
|   for i in range(7)
|   |   i32 d = i + 1
|   |   c = c + d / d
|   echo(toString_int(c))
|   return 0
