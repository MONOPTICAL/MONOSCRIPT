// expect-output: 990000
// expect-output: 199990599970000
// expect-output: 199990000
// expect-output: 99995000
// expect-output: 10000
// expect-output: 1399930000
// expect-output: 108890
// expect-output: 599970000
// expect-output: 490000
// expect-output: 123000369
// expect-output: 23
// expect-output: s123
// expect-output: 246
// Поля всех размеров вперемешку: размер объекта и смещения полей должны считаться по одной раскладке
[struct]M
|   i8 a = 0
|   i64 b = 0
|   i16 c = 0
|   f64 d = 0.0
|   i1 e = 0
|   i32 f = 0
|   string s = ""
|   array<i64> arr = []
|   i8 g = 0

[i32]main()
|   array<M> ms = []
|   for i in range(20000)
|   |   i64 k = i
|   |   M m = M()
|   |   i64 ta = k % 100
|   |   m.a = ta->i8
|   |   m.b = k * 1000003
|   |   i64 tc = k % 30000
|   |   m.c = tc->i16
|   |   m.d = k->f64 * 0.5
|   |   m.e = k % 2 == 0
|   |   i64 tf = k * 7
|   |   m.f = tf->i32
|   |   m.s = "s" + k
|   |   push(m.arr, k)
|   |   push(m.arr, k * 2)
|   |   i64 tg = k % 50
|   |   m.g = tg->i8
|   |   push(ms, m)
|   i64 sa = 0
|   i64 sb = 0
|   i64 sc = 0
|   f64 sd = 0.0
|   i64 se = 0
|   i64 sf = 0
|   i64 sl = 0
|   i64 sr = 0
|   i64 sg = 0
|   for m in ms
|   |   sa = sa + m.a
|   |   sb = sb + m.b
|   |   sc = sc + m.c
|   |   sd = sd + m.d
|   |   if m.e
|   |   |   se = se + 1
|   |   sf = sf + m.f
|   |   sl = sl + len(m.s)
|   |   sr = sr + m.arr[0] + m.arr[1]
|   |   sg = sg + m.g
|   M q = copy(ms[123])
|   q.b = 5
|   q.g = 9
|   echo(cstr(sa))
|   echo(cstr(sb))
|   echo(cstr(sc))
|   echo(cstr(sd->i64))
|   echo(cstr(se))
|   echo(cstr(sf))
|   echo(cstr(sl))
|   echo(cstr(sr))
|   echo(cstr(sg))
|   echo(cstr(ms[123].b))
|   echo(cstr(ms[123].g))
|   echo(q.s)
|   echo(cstr(q.arr[1]))
|   return 0
