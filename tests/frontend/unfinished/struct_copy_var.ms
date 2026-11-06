// expect: ok
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   P p = P()
|   P q = p
|   return q.age
