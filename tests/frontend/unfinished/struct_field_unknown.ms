// expect-error: nope
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   P p = P()
|   i32 a = p.nope
|   return a
