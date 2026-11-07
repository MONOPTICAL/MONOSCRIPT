// expect-runtime-error: float value out of range for i8 at line 3
[struct]P
|   i8 v = 300.5->i8

[i32]main()
|   P p = P()
|   return 0
