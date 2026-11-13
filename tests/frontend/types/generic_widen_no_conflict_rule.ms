// expect-error: T is both i64 and i32
// Шаблон не приводит типы сам: нужен каст
[T]same<T>(T: a, T: b)
|   return a

[i32]main()
|   i64 w = 1
|   i32 x = 1
|   same(w, x)
|   return 0
