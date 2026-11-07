// expect-output: keep7
// expect-output: keep9
// Строки живут только в выросшем массиве: новый блок данных сборщик обязан видеть
[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   string junk = "kill" + i
|   |   i = i + 1

[i32]main()
|   array<string> a = []
|   for i in range(10)
|   |   push(a, "keep" + i)
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(a[7])
|   echo(a[9])
|   return 0
