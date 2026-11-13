// expect-runtime-error: integer overflow at line 5
[i32]main()
|   i64 a = -9223372036854775807 - 1
|   i64 b = -1
|   echo("" + a / b)
|   return 0
