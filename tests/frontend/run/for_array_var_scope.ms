// expect-output: 1
// Имя переменной цикла свободно после цикла
[i32]main()
|   array<i32> a = [5]
|   for x in a
|   |   i32 y = x
|   i32 x = 1
|   echo(toString_int(x))
|   return 0
