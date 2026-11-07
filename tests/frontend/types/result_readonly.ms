// expect-error: Fields of a result are read-only
[result<i32>]f()
|   return ok(1)

[i32]main()
|   result<i32> r = f()
|   r.ok = 0
|   return 0
