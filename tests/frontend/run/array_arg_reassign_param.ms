// expect-output: 1
// Новый литерал в параметре не трогает объект вызывающего
[void]f(array<i32>: a)
|   a = [9]

[i32]main()
|   array<i32> b = [1]
|   f(b)
|   echo(toString_int(b[0]))
|   return 0
