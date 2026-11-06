// expect-output: 5
// Функция вернула тот же объект, а не копию
[array<i32>]same(array<i32>: a)
|   return a

[i32]main()
|   array<i32> a = [1]
|   array<i32> b = same(a)
|   b[0] = 5
|   echo(toString_int(a[0]))
|   return 0
