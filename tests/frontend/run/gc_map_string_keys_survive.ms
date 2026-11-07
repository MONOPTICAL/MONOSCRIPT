// expect-output: 1
// expect-output: keep7
// Ключ живёт только в map, значения числа: память пар всё равно надо просматривать
[void]churn()
|   i32 i = 0
|   while i < 200000
|   |   string junk = "kill" + i
|   |   i = i + 1

[i32]main()
|   map<string,i32> m = {}
|   m["keep" + 7] = 1
|   gc_collect()
|   churn()
|   gc_collect()
|   churn()
|   echo(toString_int(m["keep7"]))
|   for k in m
|   |   echo(k)
|   return 0
