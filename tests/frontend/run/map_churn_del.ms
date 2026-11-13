// expect-output: 0
// expect-output: 1
// Много вставок и удалений: удалённые ячейки не забивают индекс
[i32]main()
|   map<i32,i32> m = {}
|   for i in range(200000)
|   |   m[i] = i
|   |   del(m, i)
|   echo(toString_int(len(m)))
|   m[5] = 1
|   echo(toString_int(m[5]))
|   return 0
