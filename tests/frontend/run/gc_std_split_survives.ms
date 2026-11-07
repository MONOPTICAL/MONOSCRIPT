// expect-output: keep
// Части живут только в массиве, который вернула функция на D
[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   string junk = "kill" + i
|   |   i = i + 1

[i32]main()
|   array<string> parts = split("keep,x", ",")
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(parts[0])
|   return 0
