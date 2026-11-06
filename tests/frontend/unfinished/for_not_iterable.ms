// expect-error: for expects an array or range
[i32]main()
|   i32 x = 1
|   for i in x
|   |   echo("x")
|   return 0
