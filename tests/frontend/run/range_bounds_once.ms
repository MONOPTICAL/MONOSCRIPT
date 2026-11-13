// expect-output: a
// expect-output: b
// expect-output: 1
// expect-output: 2
// Границы считаются один раз, слева направо
[i32]a()
|   echo("a")
|   return 1

[i32]b()
|   echo("b")
|   return 3

[i32]main()
|   for i in range(a(), b())
|   |   echo(cstr(i))
|   return 0
