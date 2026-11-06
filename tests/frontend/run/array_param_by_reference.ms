// expect-output: 5
// expect-output: 1
// Массив передаётся по ссылке: функция меняет тот же объект
[void]set(array<i32>: a)
|   a[0] = 5

[i32]main()
|   array<i32> b = [1]
|   set(b)
|   echo(toString_int(b[0]))
|   echo(toString_int(len(b)))
|   return 0
