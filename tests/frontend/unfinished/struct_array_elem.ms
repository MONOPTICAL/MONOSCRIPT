// expect: ok
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   array<P> ps = [P(), P()]
|   P a = ps[0]
|   return ps[1].age
