// expect-output: 100000
// expect-output: 4999950000
// Ёмкость растёт, данные переезжают и не теряются
[i32]main()
|   array<i32> a = []
|   for i in range(100000)
|   |   push(a, i)
|   i64 s = 0
|   for x in a
|   |   s = s + x
|   echo(cstr(len(a)))
|   echo(cstr(s))
|   return 0
