// expect-output: a
// expect-output: 1
// expect-output: 2.5
// expect-output: x
[T]first<T>(array<T>: a)
|   return a[0]

[i32]main()
|   echo(first(["a","b"]))
|   echo(cstr(first([1,2])))
|   array<f64> d = [2.5]
|   echo(cstr(first(d)))
|   array<char> c = ['x']
|   echo(cstr(first(c)))
|   return 0
