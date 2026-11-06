// expect-output: 1
// expect-output: 2
// expect-output: 7
// Массив вычисляется один раз: новый объект в той же переменной на обход не влияет
[i32]main()
|   array<i32> a = [1,2]
|   for x in a
|   |   echo(toString_int(x))
|   |   a = [7]
|   echo(toString_int(a[0]))
|   return 0
