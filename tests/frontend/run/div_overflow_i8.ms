// expect-runtime-error: integer overflow at line 5
[i32]main()
|   i8 a = -128
|   i8 b = -1
|   echo("" + a / b)
|   return 0
