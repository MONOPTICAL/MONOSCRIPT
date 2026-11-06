// expect-runtime-error: division by zero at line 3
i32 z = 0
i32 g = 1 / z

[i32]main()
|   echo("BAD")
|   return 0
