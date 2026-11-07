// expect-output: 5
// expect-output: 5
// expect-output: x
// T может быть структурой: копия работает с её полями и методами
[struct]P
|   i32 v = 5
|   [i32]get()
|   |   return self.v

[T]first<T>(array<T>: a)
|   return a[0]

[i32]value<T>(T: x)
|   return x.get()

[T]id<T>(T: x)
|   return x

[i32]main()
|   P p = P()
|   P q = first([p])
|   echo(cstr(q.v))
|   echo(cstr(value(p)))
|   echo(id("x"))
|   return 0
