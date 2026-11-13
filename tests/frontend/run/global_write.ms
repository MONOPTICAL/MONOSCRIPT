// expect-output: 6
i32 g = 5

[void]inc()
|   g = g + 1

[i32]main()
|   inc()
|   echo(toString_int(g))
|   return 0
