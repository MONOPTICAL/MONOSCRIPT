// expect-error: Method P.f, argument 1
[struct]P
|   [i32]f(i32: x)
|   |   return x

[i32]main()
|   P p = P()
|   return p.f("s")
