// expect: ok
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   P p = P()
|   i32 a = p.age
|   string n = p.name
|   return a
