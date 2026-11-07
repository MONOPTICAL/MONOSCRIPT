// expect-output: c:1
// expect-output: a:2
// expect-output: b:3
// Сортировка структур по полю: то, ради чего всё затевалось
[struct]Finding
|   string host = "?"
|   i32 score = 0

[void]sortBy<T>(array<T>: items, func: less(T: a, T: b) -> i1)
|   for i in range(1, len(items))
|   |   i32 j = i
|   |   while j > 0 and less(items[j], items[j - 1])
|   |   |   T tmp = items[j]
|   |   |   items[j] = items[j - 1]
|   |   |   items[j - 1] = tmp
|   |   |   j = j - 1

[i1]byScore(Finding: a, Finding: b)
|   return a.score < b.score

[Finding]make(string: h, i32: s)
|   Finding f = Finding()
|   f.host = h
|   f.score = s
|   return f

[i32]main()
|   array<Finding> list = [make("a", 2), make("b", 3), make("c", 1)]
|   sortBy(list, byScore)
|   for f in list
|   |   echo(f.host + ":" + f.score)
|   return 0
