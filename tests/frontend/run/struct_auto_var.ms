// expect-output: 1
[struct]P
|   i32 age = 1
|   string name = "x"

[i32]main()
|   p ^= P()
|   echo(toString_int(p.age))
|   return 0
