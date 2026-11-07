// expect-output: ok
// Правая часть сама создаёт блоки: деление и вложенный and
[i32]main()
|   i32 a = 4
|   i32 b = 2
|   if a > 0 and (a / b == 2 and b > 1)
|   |   echo("ok")
|   return 0
