// expect-runtime-error: float value out of range for u8
[i32]main()
|   f64 f = 0.0 - 1.0
|   u8 b = f->u8
|   return 0
