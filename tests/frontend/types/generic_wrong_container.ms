// expect-error:
// Параметр array<T>, а передали число
[T]first<T>(array<T>: a)
|   return a[0]

[i32]main()
|   return first(1)
