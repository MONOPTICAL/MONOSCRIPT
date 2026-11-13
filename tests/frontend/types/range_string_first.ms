// expect-error: 3: range expects an integer, got string
[i32]main()
|   for i in range("a", 2)
|   |   echo("x")
|   return 0
