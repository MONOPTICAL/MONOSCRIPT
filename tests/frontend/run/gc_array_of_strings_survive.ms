// expect-output: keep7
// expect-output: keep8
// Строки живут только в массиве: блок данных со ссылками сборщик обязан просматривать
[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   string junk = "kill" + i
|   |   i = i + 1

[i32]main()
|   array<string> a = ["keep" + 7, "keep" + 8]
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(a[0])
|   echo(a[1])
|   return 0
