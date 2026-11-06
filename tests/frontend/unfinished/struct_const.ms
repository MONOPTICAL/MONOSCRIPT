// expect-error: constant
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   const P p = P()
|   p.age = 2
|   return 0
