// expect-runtime-error: division by zero at line 5
i32 z = 0

[struct]P
|   i32 v = 1 / 0

[i32]main()
|   P p = P()
|   return 0
