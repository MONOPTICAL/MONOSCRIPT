// expect: ok
[i64]f(i64: a, i32: n) @strict
|   i64 x = a + 1
|   i64 y = 2 * a
|   i64 z = a + n->i64
|   if a > 0
|   |   return x + y + z
|   return 0

[f64]g(f64: d) @strict
|   return d + 0.5

[i32]main()
|   return 0
