// expect-output: 100000
// expect-output: 16000400000
// expect-output: 5835
// expect-output: 156061591
// Таблицы с вставками, удалениями и ростом по кругу
[i32]main()
|   map<i64,i64> m = {}
|   for rr in range(5)
|   |   i64 r = rr
|   |   for i in range(40000)
|   |   |   i64 k = i
|   |   |   m[k * 7 + r] = k + r
|   |   for i in range(40000)
|   |   |   i64 k = i
|   |   |   if k % 2 == 0
|   |   |   |   i64 key = k * 7 + r
|   |   |   |   if has(m, key)
|   |   |   |   |   del(m, key)
|   i64 s = 0
|   for k in m
|   |   s = s + k + m[k]
|   echo(cstr(len(m)))
|   echo(cstr(s))
|   map<string,i64> sm = {}
|   for i in range(30000)
|   |   i64 k = i
|   |   sm["k" + (k % 7000)] = k
|   |   if k % 3 == 0
|   |   |   string d = "k" + ((k * 13) % 7000)
|   |   |   if has(sm, d)
|   |   |   |   del(sm, d)
|   i64 ss = 0
|   for k in sm
|   |   ss = ss + sm[k] + len(k)
|   echo(cstr(len(sm)))
|   echo(cstr(ss))
|   return 0
