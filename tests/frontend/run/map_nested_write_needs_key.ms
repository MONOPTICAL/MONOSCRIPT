// expect-runtime-error: key not found at line 5
// Запись создаёт только последний ключ
[i32]main()
|   map<string,array<i32>> m = {}
|   m["x"][0] = 1
|   return 0
