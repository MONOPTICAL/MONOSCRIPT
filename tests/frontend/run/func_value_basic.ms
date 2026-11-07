// expect-output: 3
// expect-output: 10
[i32]inc(i32: x)
|   return x + 1

[i32]twice(i32: x)
|   return x * 2

[i32]apply(func: f(i32: x) -> i32, i32: v)
|   return f(v)

[i32]main()
|   echo(cstr(apply(inc, 2)))
|   echo(cstr(apply(twice, 5)))
|   return 0
