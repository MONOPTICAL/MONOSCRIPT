// expect-output: -1
// expect-output: 0
// expect-output: end
// Начало включается, конец нет. Пустой и обратный диапазоны не выполняются
[i32]main()
|   for i in range(-1, 1)
|   |   echo(cstr(i))
|   for i in range(3, 3)
|   |   echo("BAD")
|   for i in range(5, 2)
|   |   echo("BAD")
|   echo("end")
|   return 0
