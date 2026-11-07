// expect-error: Cannot cast void
[result<void>]action()
|   return ok()
[result<void>]work()
|   i32 value = action()?->i32
|   return ok()
[i32]main()
|   return 0
