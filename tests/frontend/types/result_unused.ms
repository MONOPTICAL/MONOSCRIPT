// expect-error: 6: The result is not used
[result<void>]act()
|   return ok()

[i32]main()
|   act()
|   return 0
