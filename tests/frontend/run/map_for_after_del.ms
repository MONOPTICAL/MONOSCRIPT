// expect-output: a
// expect-output: c
// expect-output: b
// Удалённый ключ пропускается, а добавленный заново идёт в конец
[i32]main()
|   map<string,i32> m = {"a": 1, "b": 2, "c": 3}
|   del(m, "b")
|   m["b"] = 9
|   for k in m
|   |   echo(k)
|   return 0
