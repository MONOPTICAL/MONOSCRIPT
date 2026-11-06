// expect-error: expected i32, got string
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   P p = P()
|   i32 a = p.name
|   return a
