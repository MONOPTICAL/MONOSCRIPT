// expect-output: keep7
[array<T>]wrap<T>(T: x)
|   return [x]

[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   string junk = "kill" + i
|   |   i = i + 1

[i32]main()
|   array<string> a = wrap("keep" + 7)
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(a[0])
|   return 0
