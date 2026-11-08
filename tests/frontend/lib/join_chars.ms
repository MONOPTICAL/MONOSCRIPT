// expect-output: test
// Правая часть сама создаёт блоки: деление и вложенный and
[i32]main()
|   array<char> x = toArray("test")
|   echo(cstr(x))
|   return 0