// expect-error: result<void> cannot have a fallback value
[result<void>]f()
|   return ok()
[i32]main()
|   f()? else 1
|   return 0
