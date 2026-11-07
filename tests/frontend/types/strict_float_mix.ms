// expect-error: 3: Implicit cast is not allowed in @strict mode: float and i32
[float]f(float: a, i32: n) @strict
|   return a + n

[i32]main()
|   return 0
