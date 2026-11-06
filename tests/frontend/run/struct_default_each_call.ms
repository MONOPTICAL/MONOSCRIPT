// expect-output: 1
// expect-output: 2
// expect-output: 2
// Значение по умолчанию вычисляется при каждом создании
i32 next = 0

[i32]nextId()
|   next = next + 1
|   return next

[struct]P
|   i32 id = nextId()

[i32]main()
|   P a = P()
|   P b = P()
|   echo(toString_int(a.id))
|   echo(toString_int(b.id))
|   echo(toString_int(next))
|   return 0
