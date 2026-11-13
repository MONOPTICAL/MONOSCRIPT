// expect-output: 0
[void]f()
|   for i in range(3)
|   |   echo(toString_int(i))
|   |   return

[i32]main()
|   f()
|   return 0
