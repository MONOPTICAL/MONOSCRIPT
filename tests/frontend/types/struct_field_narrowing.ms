// expect-error: use explicit cast
[struct]P
|   i8 a = 1

[i32]main()
|   P p = P()
|   i32 x = 1
|   p.a = x
|   return 0
