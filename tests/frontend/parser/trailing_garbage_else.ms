// expect-error: foo
[i32]main()
|   if 1
|   |   echo("a")
|   else foo
|   |   echo("b")
|   return 0
