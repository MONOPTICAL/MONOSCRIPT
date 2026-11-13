// expect-output: 0
// expect-output: 1
// Переменная объявлена в теле цикла: на каждой итерации она новая
[i32]main()
|   for i in range(2)
|   |   final i32 x
|   |   x = i
|   |   echo(toString_int(x))
|   return 0
