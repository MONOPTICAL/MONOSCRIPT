// expect-runtime-error: float value out of range for i8 at line 3
float f = 200.0
i8 g = f->i8

[i32]main()
|   echo("BAD")
|   return 0
