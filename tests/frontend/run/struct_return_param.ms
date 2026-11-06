// expect-output: 3
// expect-output: 1
[struct]P
|   i32 age = 1
|   string name = "x"

[P]mk(i32: a)
|   P p = P()
|   p.age = a
|   return p

[P]fresh()
|   return P()

[i32]main()
|   P a = mk(3)
|   echo(toString_int(a.age))
|   P b = fresh()
|   echo(toString_int(b.age))
|   return 0
