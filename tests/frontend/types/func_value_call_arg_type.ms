// expect-error: Function f, argument 1
[i32]apply(func: f(i32: x) -> i32)
|   return f("s")

[i32]main()
|   return 0
