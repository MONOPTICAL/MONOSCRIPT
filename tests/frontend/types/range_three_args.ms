// expect-error: 3: range expects 1 or 2 arguments, got 3
[i32]main()
|   for i in range(1, 2, 3)
|   |   echo("x")
|   return 0
