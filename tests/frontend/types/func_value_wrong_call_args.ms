// expect-error: Function f expects 1 arguments, got 2
[i32]apply(func: f(i32: x) -> i32)
|   return f(1, 2)

[i32]main()
|   return 0
