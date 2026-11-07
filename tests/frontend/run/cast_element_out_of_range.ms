// expect-runtime-error: float value out of range for i8 at line 4
[i32]main()
|   array<float> a = [200.0]
|   i8 x = a[0]->i8
|   return 0
