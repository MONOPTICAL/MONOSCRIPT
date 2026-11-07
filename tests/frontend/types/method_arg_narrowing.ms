// expect-error: use explicit cast
[struct]P
|   [i32]f(i8: x)
|   |   return 1

[i32]main()
|   P p = P()
|   i32 n = 1
|   return p.f(n)
