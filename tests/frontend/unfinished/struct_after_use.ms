// expect: ok
[i32]main()
|   P p = P()
|   return p.age

[struct]P
|   i32 age = 1
|   string name = "x"

