// expect: ok
[struct]P
|   i32 age = 1
|   string name = "x"

[P]mk()
|   return P()

[i32]main()
|   P p = mk()
|   return p.age
