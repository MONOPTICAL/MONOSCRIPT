// expect-output: keep7
[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   string junk = "trash" + i
|   |   i = i + 1

[i32]main()
|   string s = "keep" + 7
|   gc_collect()
|   churn()
|   gc_collect()
|   echo(s)
|   return 0
