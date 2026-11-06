// expect-output: 9
// Присваивание и передача не копируют
[struct]P
|   i32 age = 1
|   string name = "x"

[void]set(P: x)
|   x.age = 9

[i32]main()
|   P p = P()
|   P q = p
|   set(q)
|   echo(toString_int(p.age))
|   return 0
