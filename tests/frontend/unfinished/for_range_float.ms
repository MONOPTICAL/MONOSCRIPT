// expect-error: range expects an integer
[i32]main()
|   for i in range(1.5)
|   |   echo("x")
|   return 0
