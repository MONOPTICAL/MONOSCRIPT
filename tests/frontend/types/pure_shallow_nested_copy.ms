// expect-error: Pure function cannot create a new reference
[struct]Box
|   array<i32> values = [1, 2]
[void]change(Box: original) @pure
|   Box local = copy(original)
|   local.values[0] = 99
[i32]main()
|   return 0
