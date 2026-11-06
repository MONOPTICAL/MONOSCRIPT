// expect-output: 5
i64 big = 5000000000

[i32]main()
|   i32 y = 5
|   big = y
|   echo(toString_long(big))
|   return 0
