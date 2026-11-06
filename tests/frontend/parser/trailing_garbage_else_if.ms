// expect-error: foo
[i32]main()
|   if 0
|   |   echo("a")
|   else if 1 foo
|   |   echo("b")
|   return 0
