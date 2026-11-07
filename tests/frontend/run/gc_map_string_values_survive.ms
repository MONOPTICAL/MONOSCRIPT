// expect-output: keep7
// Значение живёт только в map
[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   string junk = "kill" + i
|   |   i = i + 1

[i32]main()
|   map<string,string> m = {"a": "keep" + 7}
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(m["a"])
|   return 0
