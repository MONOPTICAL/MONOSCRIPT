// expect-output: 1
// expect-output: 5
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   P a = P()
|   P b = copy(a)
|   b.age = 5
|   echo(toString_int(a.age))
|   echo(toString_int(b.age))
|   return 0
