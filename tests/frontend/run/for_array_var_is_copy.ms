// expect-output: 1
// expect-output: 2
// Запись в переменную цикла массив не меняет
[i32]main()
|   array<i32> a = [1,2]
|   for x in a
|   |   x = 9
|   echo(toString_int(a[0]))
|   echo(toString_int(a[1]))
|   return 0
