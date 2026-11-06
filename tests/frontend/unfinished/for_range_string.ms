// expect-error: range expects an integer
[i32]main()
|   for i in range("a")
|   |   echo("x")
|   return 0
