// expect-error: Cannot bind void value
[result<void>]action()
|   return ok()
[result<void>]work()
|   value ^= action()?
|   return ok()
[i32]main()
|   return 0
