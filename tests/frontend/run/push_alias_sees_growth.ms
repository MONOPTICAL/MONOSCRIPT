// expect-output: 2
// Массив это ссылка: рост виден и через другую переменную
[void]add(array<i32>: a)
|   push(a, 1)

[i32]main()
|   array<i32> a = [0]
|   array<i32> b = a
|   add(b)
|   echo(cstr(len(a)))
|   return 0
