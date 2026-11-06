// expect-output: 3
[i32]main()
|   i64 n = 3
|   i64 c = 0
|   for i in range(n)
|   |   c = c + 1
|   echo(toString_long(c))
|   return 0
