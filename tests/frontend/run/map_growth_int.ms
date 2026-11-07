// expect-output: 100000
// expect-output: 19999800000
// Таблица растёт, все ключи на месте
[i32]main()
|   map<i32,i64> m = {}
|   for i in range(100000)
|   |   m[i] = i * 2
|   i64 s = 0
|   for k in m
|   |   s = s + m[k]
|   echo(toString_int(len(m)))
|   s = s + 9999900000
|   echo(toString_long(s))
|   return 0
