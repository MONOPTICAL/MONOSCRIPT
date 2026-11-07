// expect-runtime-error: integer overflow at line 5
[i32]main()
|   i32 a = -2147483648
|   i32 b = -1
|   echo("" + a / b)
|   return 0
