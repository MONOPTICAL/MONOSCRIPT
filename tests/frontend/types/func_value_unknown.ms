// expect-error: Variable not found: nope
[i32]apply(func: f(i32: x) -> i32)
|   return f(1)

[i32]main()
|   return apply(nope)
