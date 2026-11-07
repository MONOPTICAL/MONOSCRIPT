// expect: ok
// Тело проверяется при подстановке: невызванный шаблон не проверяется
[T]neverCalled<T>(T: x)
|   return x.nothing

[i32]main()
|   return 0
