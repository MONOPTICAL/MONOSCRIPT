// expect-output: fn
// expect-output: a
// expect-output: b
// Метод не конфликтует с функцией и с методом другой структуры
[string]get()
|   return "fn"

[struct]A
|   [string]get()
|   |   return "a"

[struct]B
|   [string]get()
|   |   return "b"

[i32]main()
|   echo(get())
|   A a = A()
|   B b = B()
|   echo(a.get())
|   echo(b.get())
|   return 0
