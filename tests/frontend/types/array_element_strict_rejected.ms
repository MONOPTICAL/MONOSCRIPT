// expect-error: 5: @strict
[i32]main() @strict
|   array<i64> a = [1]
|   i32 x = 1
|   a[0] = x
|   return 0
