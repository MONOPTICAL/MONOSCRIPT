// expect-output: 3000000
// expect-output: 13499995500000
// expect-output: 8999997
// expect-output: 0
// expect-output: 49500000
// expect-output: 488890
// expect-output: 99999
// Рост массивов до миллионов элементов, pop до пустого, массив строк
[i32]main()
|   array<i64> a = []
|   for i in range(3000000)
|   |   i64 k = i
|   |   push(a, k * 3)
|   i64 s = 0
|   for x in a
|   |   s = s + x
|   array<i8> b = []
|   for i in range(1000000)
|   |   i64 t = i % 100
|   |   push(b, t->i8)
|   i64 sb = 0
|   while len(b) > 0
|   |   sb = sb + pop(b)
|   array<string> strs = []
|   for i in range(100000)
|   |   push(strs, "" + i)
|   i64 sl = 0
|   for t in strs
|   |   sl = sl + len(t)
|   echo(cstr(len(a)))
|   echo(cstr(s))
|   echo(cstr(a[2999999]))
|   echo(cstr(len(b)))
|   echo(cstr(sb))
|   echo(cstr(sl))
|   echo(strs[99999])
|   return 0
