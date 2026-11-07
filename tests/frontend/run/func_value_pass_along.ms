// expect-output: 6
// Параметр типа func передаётся дальше
[i32]triple(i32: x)
|   return x * 3

[i32]inner(func: f(i32: x) -> i32, i32: v)
|   return f(v)

[i32]outer(func: f(i32: x) -> i32, i32: v)
|   return inner(f, v)

[i32]main()
|   echo(cstr(outer(triple, 2)))
|   return 0
