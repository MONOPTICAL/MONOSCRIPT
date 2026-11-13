// expect-output: 1
// expect-output: 9
// Запись в элемент во время обхода видна следующей итерации
[i32]main()
|   array<i32> a = [1,2]
|   for x in a
|   |   echo(toString_int(x))
|   |   a[1] = 9
|   return 0
