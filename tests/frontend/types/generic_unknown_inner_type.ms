// expect-error: Unknown
// U не объявлен: это просто неизвестный тип
[T]f<T>(T: x)
|   U y = x
|   return x

[i32]main()
|   f(1)
|   return 0
