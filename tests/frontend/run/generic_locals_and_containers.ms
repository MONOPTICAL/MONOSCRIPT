// expect-output: 2
// expect-output: b
// expect-output: 3
// expect-output: 9
// T внутри тела: локальные, массивы, map
[array<T>]pair<T>(T: a, T: b)
|   T x = a
|   array<T> out = [x, b]
|   return out

[i32]count<T>(array<T>: a)
|   map<string,T> m = {}
|   i32 n = 0
|   for x in a
|   |   m["k" + n] = x
|   |   n = n + 1
|   return len(m)

[T]lastOf<T>(array<array<T>>: rows)
|   array<T> row = rows[len(rows) - 1]
|   return row[len(row) - 1]

[i32]main()
|   array<string> p = pair("a", "b")
|   echo(cstr(len(p)))
|   echo(p[1])
|   echo(cstr(count([1,2,3])))
|   echo(cstr(lastOf([[1],[8,9]])))
|   return 0
