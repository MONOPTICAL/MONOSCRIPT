// expect-output: 5
// Массив это ссылка: присваивание не копирует
[i32]main()
|   array<i32> a = [1]
|   array<i32> b = a
|   b[0] = 5
|   echo(toString_int(a[0]))
|   return 0
