// expect-output: 2
// Менять другой массив внутри цикла можно
[i32]main()
|   array<i32> a = [1, 2]
|   array<i32> b = []
|   for x in a
|   |   push(b, x)
|   echo(cstr(len(b)))
|   return 0
