// expect-output: keep7
[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   string junk = "kill" + i
|   |   i = i + 1

[i32]main()
|   array<string> a = ["keep" + 7]
|   array<string> b = copy(a)
|   a = []
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(b[0])
|   return 0
