// expect-error: Pure function cannot create a new reference
[array<i32>]identity(array<i32>: original) @pure
|   return original
[i32]main()
|   return 0
