// expect-runtime-error: division by zero at line 6
[i32]main()
|   echo("before")
|   i32 a = 7
|   i32 b = 0
|   echo(toString_int(a / b))
|   echo("BAD")
|   return 0
