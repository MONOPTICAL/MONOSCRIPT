// expect-runtime-error: float value out of range for i32 at line 4
[i32]main()
|   float a = 3000000000.0
|   echo("" + a->i32)
|   return 0
