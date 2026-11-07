// expect-output: 1
// Запись i32 в ячейку значения не оставляет мусор от старого значения
[i32]main()
|   map<string,i32> m = {"k": -1}
|   m["k"] = 1
|   i64 w = m["k"]
|   echo(toString_long(w))
|   return 0
