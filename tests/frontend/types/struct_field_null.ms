// expect-error: expected
[struct]In
|   i32 a = 1

[struct]P
|   In i = In()

[i32]main()
|   P p = P()
|   p.i = null
|   return 0
