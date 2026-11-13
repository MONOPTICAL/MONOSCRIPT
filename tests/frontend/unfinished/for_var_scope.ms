// expect-error: Variable not found: i
[i32]main()
|   for i in range(2)
|   |   echo("x")
|   i32 y = i
|   return 0
