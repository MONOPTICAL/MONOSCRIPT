// expect-output: 7
// expect-output: 1
// expect-output: 8
// expect-output: 1
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   array<P> ps = [P(), P()]
|   ps[0].age = 7
|   echo(toString_int(ps[0].age))
|   echo(toString_int(ps[1].age))
|   for p in ps
|   |   p.age = p.age + 1
|   ps[1].age = 1
|   echo(toString_int(ps[0].age))
|   echo(toString_int(ps[1].age))
|   return 0
