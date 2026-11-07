// expect-output: 1
// expect-output: 2
// Вызов из цикла не считается циклом для final внутри шаблона
[T]once<T>(T: x)
|   final T y
|   y = x
|   return y

[i32]main()
|   for i in range(1, 3)
|   |   echo(cstr(once(i)))
|   return 0
