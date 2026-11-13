// expect-error: 3: Implicit cast is not allowed in @strict mode: i64 and i32
[i64]f(i64: a, i32: n) @strict
|   return a + n

[i32]main()
|   return 0
