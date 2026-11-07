// expect-output: c
// expect-output: a
// expect-output: b
// Порядок вставки, а не порядок хеша
[i32]main()
|   map<string,i32> m = {"c": 1, "a": 2, "b": 3}
|   for k in m
|   |   echo(k)
|   return 0
