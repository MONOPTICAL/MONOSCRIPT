// expect-output: 450015000
// Глубокая рекурсия без переполнения стека
[i64]down(i64: n)
|   if n == 0
|   |   return 0
|   return down(n - 1) + n

[i32]main()
|   echo(cstr(down(30000)))
|   return 0
