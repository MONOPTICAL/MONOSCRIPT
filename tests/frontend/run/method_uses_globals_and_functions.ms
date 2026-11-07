// expect-output: 11
// Метод видит глобальные переменные и функции, как обычная функция
i32 base = 10

[i32]one()
|   return 1

[struct]A
|   [i32]sum2()
|   |   return base + one()

[i32]main()
|   A a = A()
|   echo(cstr(a.sum2()))
|   return 0
