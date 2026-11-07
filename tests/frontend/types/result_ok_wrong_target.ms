// expect-error: ok() can only be used where a result<T> is expected
[i32]f()
|   return ok(1)

[i32]main()
|   return 0
