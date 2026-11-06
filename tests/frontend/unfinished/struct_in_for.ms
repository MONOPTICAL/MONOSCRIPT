// expect: ok
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   array<P> ps = [P()]
|   for p in ps
|   |   i32 a = p.age
|   return 0
