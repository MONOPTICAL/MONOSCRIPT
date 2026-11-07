// expect-output: 4
// Структуру и метод можно использовать выше объявления
[i32]main()
|   Late l = Late()
|   echo(cstr(l.four()))
|   return 0

[struct]Late
|   [i32]four()
|   |   return 4
