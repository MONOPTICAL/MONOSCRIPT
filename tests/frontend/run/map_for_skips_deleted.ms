// expect-output: 2
// Считаем ключи: expect-output не заметит лишнюю строку в выводе
[i32]main()
|   map<string,i32> m = {"a": 1, "b": 2, "c": 3}
|   del(m, "b")
|   i32 n = 0
|   for k in m
|   |   n = n + 1
|   echo(toString_int(n))
|   return 0
