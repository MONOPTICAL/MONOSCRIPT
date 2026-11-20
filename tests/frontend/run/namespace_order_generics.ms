// expect-output: 2
// expect-output: 120
// expect-output: 5
// expect-output: 4
[i32]main()
|   echo(toString_int(m.f()))
|   echo(toString_int(m.fact(5)))
|   array<i32> a = [5, 6]
|   echo(toString_int(m.first(a)))
|   echo(toString_int(m.viaGeneric()))
|   return 0

[namespace] m
|   [i32]f()
|   |   return g()
|   [i32]g()
|   |   return 2
|   [i32]fact(i32: n)
|   |   if n <= 1
|   |   |   return 1
|   |   return n * fact(n - 1)
|   [i32]viaGeneric()
|   |   return id(4)
|   [T]id<T>(T: x)
|   |   return x
|   [T]first<T>(array<T>: a)
|   |   return a[0]
