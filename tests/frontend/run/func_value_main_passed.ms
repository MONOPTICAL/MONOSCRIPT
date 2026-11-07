// expect-output: 0
// Пользовательский main переименовывается, ссылка на него остаётся верной
[i32]zero()
|   return 0

[i32]call(func: f(i32: x) -> i32)
|   return f(0)

[i32]id(i32: x)
|   return x

[i32]main()
|   echo(cstr(call(id)))
|   return 0
