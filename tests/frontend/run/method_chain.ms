// expect-output: 5
// expect-output: 2
// expect-output: 9
// expect-output: 1
// Вызов метода идёт той же цепочкой, что поля и индексы
[struct]In
|   i32 v = 5
|   [i32]get()
|   |   return self.v
|   [array<i32>]list()
|   |   return [1,2]

[struct]Out
|   In inner = In()
|   [In]me()
|   |   return self.inner

[i32]main()
|   Out o = Out()
|   echo(cstr(o.inner.get()))
|   echo(cstr(o.inner.list()[1]))
|   o.me().v = 9
|   echo(cstr(o.me().get()))
|   array<In> a = [In()]
|   echo(cstr(a[0].list()[0]))
|   return 0
