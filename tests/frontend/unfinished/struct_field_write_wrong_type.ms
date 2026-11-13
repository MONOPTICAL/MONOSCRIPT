// expect-error: expected i32, got string
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   P p = P()
|   p.age = "y"
|   return 0
