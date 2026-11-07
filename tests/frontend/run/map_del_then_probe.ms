// expect-output: 1
// Удаление не должно обрывать цепочку поиска для других ключей
[i32]main()
|   map<i32,i32> m = {}
|   for i in range(1000)
|   |   m[i] = 1
|   for i in range(999)
|   |   del(m, i)
|   echo(toString_int(m[999]))
|   return 0
