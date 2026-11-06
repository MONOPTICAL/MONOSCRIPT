// expect-output: keep7
// expect-output: keep8
// Сборка внутри цикла: массив держит только временное значение цикла
[array<string>]mk()
|   return ["keep" + 7, "keep" + 8]

[void]churn()
|   i32 i = 0
|   while i < 100000
|   |   string junk = "kill" + i
|   |   i = i + 1

[i32]main()
|   for s in mk()
|   |   gc_collect()
|   |   churn()
|   |   echo(s)
|   return 0
