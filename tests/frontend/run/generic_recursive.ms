// expect-output: 3
// expect-output: 6.0
[T]last<T>(array<T>: a, i32: i)
|   if i == len(a) - 1
|   |   return a[i]
|   return last(a, i + 1)

[T]mul<T>(T: x, i32: n)
|   if n <= 1
|   |   return x
|   return x + mul(x, n - 1)

[i32]main()
|   echo(cstr(last([1,2,3], 0)))
|   f64 d = 2.0
|   echo(cstr(mul(d, 3)))
|   return 0
