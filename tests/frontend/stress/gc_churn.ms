// expect-output: 10000
// expect-output: 10000
// expect-output: 0
// expect-output: 300185056
// Много мусора вперемешку с живыми объектами и явными сборками: живое не должно потеряться
[struct]Item
|   i64 id = 0
|   string name = ""
|   array<i64> data = []

[i32]main()
|   array<Item> keep = []
|   map<string, i64> index = {}
|   for i in range(30000)
|   |   i64 k = i
|   |   string junk = "junk" + k + "-" + (k * 3)
|   |   array<i64> tmp = []
|   |   for j in range(8)
|   |   |   i64 jj = j
|   |   |   push(tmp, k + jj)
|   |   if k % 3 == 0
|   |   |   Item it = Item()
|   |   |   it.id = k
|   |   |   it.name = "item" + k
|   |   |   push(it.data, k)
|   |   |   push(it.data, len(junk))
|   |   |   push(it.data, tmp[7])
|   |   |   push(keep, it)
|   |   |   index[it.name] = len(keep) - 1
|   |   if k % 5000 == 0
|   |   |   gc_collect()
|   gc_collect()
|   i64 bad = 0
|   i64 sum = 0
|   for it in keep
|   |   if it.name != "item" + it.id
|   |   |   bad = bad + 1
|   |   if keep[index[it.name]].id != it.id
|   |   |   bad = bad + 1
|   |   sum = sum + it.data[0] + it.data[1] + it.data[2]
|   echo(cstr(len(keep)))
|   echo(cstr(len(index)))
|   echo(cstr(bad))
|   echo(cstr(sum))
|   return 0
