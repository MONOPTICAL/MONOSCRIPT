// expect-output: keep7
[struct]A
|   string s = "x"
|   [string]make()
|   |   return "keep" + 7

[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   string junk = "kill" + i
|   |   i = i + 1

[i32]main()
|   A a = A()
|   a.s = a.make()
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(a.s)
|   return 0
