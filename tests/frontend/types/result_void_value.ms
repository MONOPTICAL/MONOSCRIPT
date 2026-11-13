// expect-error: result<void> has no value
[result<void>]f()
|   return ok()

[i32]main()
|   result<void> r = f()
|   if r.value
|   |   return 1
|   return 0
