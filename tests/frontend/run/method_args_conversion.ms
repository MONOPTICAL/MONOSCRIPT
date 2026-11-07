// expect-output: 7
// expect-output: 1.5
// expect-output: 3
// expect-output: 2
// expect-output: 0
[struct]A
|   [i64]wide(i64: x)
|   |   return x
|   [f64]fl(f64: x)
|   |   return x
|   [i32]pick(array<i32>: a, i32: i)
|   |   return a[i]
|   [i32]size(map<string,i32>: m)
|   |   return len(m)

[i32]main()
|   A a = A()
|   i32 n = 7
|   echo(cstr(a.wide(n)))
|   echo(cstr(a.fl(1.5)))
|   echo(cstr(a.pick([1,2,3], 2)))
|   echo(cstr(a.pick([5,2], 1)))
|   echo(cstr(a.size({})))
|   return 0
