// expect-output: 7
// Встроенную функцию тоже можно передать
[string]via(func: f(i32: x) -> string, i32: v)
|   return f(v)

[i32]main()
|   echo(via(toString_int, 7))
|   return 0
