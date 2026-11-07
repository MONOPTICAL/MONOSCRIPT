// expect-runtime-error: float value out of range for i32 at line 4
[i32]main()
|   f64 a = 3000000000.5
|   i32 x = a->i32
|   return x
