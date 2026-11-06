// expect: ok
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   P p = P()
|   p.age = 5
|   p.name = "y"
|   return 0
