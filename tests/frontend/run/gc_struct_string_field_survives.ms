// expect-output: keep7
// Строка живёт только в поле: память объекта сборщик обязан просматривать
[struct]S
|   string s = "keep" + 7

[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   string junk = "kill" + i
|   |   i = i + 1

[i32]main()
|   S o = S()
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(o.s)
|   return 0
