// expect-output: 2
// expect-output: 3
// Изменить значение можно: набор ключей остаётся прежним
[i32]main()
|   map<string,i32> m = {"a": 1, "b": 2}
|   for k in m
|   |   m[k] = m[k] + 1
|   echo(toString_int(m["a"]))
|   echo(toString_int(m["b"]))
|   return 0
