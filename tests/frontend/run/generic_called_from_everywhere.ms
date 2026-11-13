// expect-output: 1
// expect-output: 2
// expect-output: 3
// expect-output: 4
// Шаблон объявлен ниже, вызов из глобала, поля, метода и цикла
i32 g = id(1)

[struct]S
|   i32 f = id(2)
|   [i32]m()
|   |   return id(3)

[i32]main()
|   echo(cstr(g))
|   S s = S()
|   echo(cstr(s.f))
|   echo(cstr(s.m()))
|   for i in range(1)
|   |   echo(cstr(id(4)))
|   return 0

[T]id<T>(T: x)
|   return x
