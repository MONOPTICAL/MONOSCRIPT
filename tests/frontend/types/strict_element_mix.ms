// expect-error: 5: Implicit cast is not allowed in @strict
[i64]f() @strict
|   array<i32> a = [1]
|   i64 b = 1
|   return b + a[0]

[i32]main()
|   return 0
