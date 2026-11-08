// expect-error: result has no member 'nope'
[result<i32>]divide(i32: a, i32: b)
|   return ok(a / b)
[i32]main()
|   x ^= divide(4, 2).nope
|   return 0
