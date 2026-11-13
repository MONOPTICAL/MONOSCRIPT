// expect-output: 1
// expect-output: x
// expect-output: 5
// expect-output: y
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   P p = P()
|   echo(toString_int(p.age))
|   echo(p.name)
|   p.age = 5
|   p.name = "y"
|   echo(toString_int(p.age))
|   echo(p.name)
|   return 0
