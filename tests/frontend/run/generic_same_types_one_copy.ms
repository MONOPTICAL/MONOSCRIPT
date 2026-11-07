// expect-output: called
// expect-output: called
// expect-output: 2
// Глобал виден, копия одна на типы
i32 n = 0

[T]tick<T>(T: x)
|   n = n + 1
|   echo("called")
|   return x

[i32]main()
|   tick(1)
|   tick(2)
|   echo(cstr(n))
|   return 0
