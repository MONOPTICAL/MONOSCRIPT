// expect-error: ok() for result<void> takes no value
[result<void>]f()
|   return ok(1)

[i32]main()
|   return 0
