// expect-runtime-error: float value out of range for i64 at line 4
[i32]main()
|   float a = 10000000000000000000.0
|   echo("" + a->i64)
|   return 0
