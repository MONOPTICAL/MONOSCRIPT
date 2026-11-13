// expect-output: 6
[struct]P
|   i32 age = 1
|   string name = "x"

P g = P()

[i32]main()
|   g.age = 6
|   echo(toString_int(g.age))
|   return 0
