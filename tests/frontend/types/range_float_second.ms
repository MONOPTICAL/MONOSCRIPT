// expect-error: 3: range expects an integer, got f64
[i32]main()
|   for i in range(1, 2.5)
|   |   echo("x")
|   return 0
