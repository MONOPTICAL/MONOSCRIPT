// expect-runtime-error: integer overflow at line 5
[i32]main()
|   i16 a = -32768
|   i16 b = -1
|   echo("" + a / b)
|   return 0
