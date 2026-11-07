// expect-output: 5000000000
// expect-output: 5000000001
// expect-output: 3
// expect-output: 4
// Один i64 делает счётчик i64
[i32]main()
|   i64 big = 5000000000
|   for i in range(big, big + 2)
|   |   echo(cstr(i))
|   i8 s = 3
|   i16 e = 5
|   for j in range(s, e)
|   |   echo(cstr(j))
|   return 0
