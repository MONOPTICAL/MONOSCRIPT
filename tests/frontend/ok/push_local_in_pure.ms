// expect: ok
// Свой массив внутри чистой функции менять можно
[i32]f() @pure
|   array<i32> a = []
|   push(a, 1)
|   return pop(a)

[i32]main()
|   return 0
