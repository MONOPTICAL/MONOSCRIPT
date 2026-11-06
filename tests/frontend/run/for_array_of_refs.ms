// expect-output: 5
// expect-output: 5
// Вложенный массив это ссылка: запись через переменную цикла меняет его
[i32]main()
|   array<array<i32>> a = [[1],[2]]
|   for row in a
|   |   row[0] = 5
|   echo(toString_int(a[0][0]))
|   echo(toString_int(a[1][0]))
|   return 0
