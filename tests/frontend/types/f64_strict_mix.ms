// expect-error: 3: Implicit cast is not allowed in @strict
[f64]f(f64: a, i32: n) @strict
|   return a + n

[i32]main()
|   return 0
