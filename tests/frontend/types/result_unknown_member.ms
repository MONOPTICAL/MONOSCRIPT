// expect-error: result has no member
[result<i32>]f()
|   return ok(1)

[i32]main()
|   result<i32> r = f()
|   return r.data
