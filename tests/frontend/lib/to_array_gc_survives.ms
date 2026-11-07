// expect-output: keep
// Массив от функции на D переживает сборку
[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   string junk = "kill" + i
|   |   i = i + 1

[i32]main()
|   array<char> x = toArray("keep")
|   gc_collect()
|   churn()
|   gc_collect()
|   string s = joinChars(x)
|   churn()
|   gc_collect()
|   echo(s)
|   return 0
