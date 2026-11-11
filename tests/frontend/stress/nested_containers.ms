// expect-output: 850
// expect-output: 60000
// expect-output: 1799970000
// Массив таблиц с массивами внутри, со сборками мусора посередине
[i32]main()
|   array<map<string, array<i64>>> groups = []
|   for g in range(50)
|   |   map<string, array<i64>> fresh = {}
|   |   push(groups, fresh)
|   for i in range(60000)
|   |   i64 k = i
|   |   i64 gi = k % 50
|   |   string key = "b" + (k % 17)
|   |   map<string, array<i64>> m = groups[gi]
|   |   if has(m, key)
|   |   |   push(m[key], k)
|   |   else
|   |   |   m[key] = [k]
|   |   if k % 20000 == 0
|   |   |   gc_collect()
|   i64 total = 0
|   i64 count = 0
|   i64 keys = 0
|   for m in groups
|   |   for key in m
|   |   |   keys = keys + 1
|   |   |   for v in m[key]
|   |   |   |   total = total + v
|   |   |   |   count = count + 1
|   echo(cstr(keys))
|   echo(cstr(count))
|   echo(cstr(total))
|   return 0
