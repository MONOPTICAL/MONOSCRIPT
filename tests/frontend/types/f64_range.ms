// expect-error: range expects an integer, got f64
[i32]main()
|   f64 n = 2.0
|   for i in range(n)
|   |   echo("x")
|   return 0
