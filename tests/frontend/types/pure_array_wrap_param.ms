// expect-error: Pure function cannot create a new reference
[void]change(array<i32>: original) @pure
|   array<array<i32>> local = [original]
|   local[0][0] = 99
[i32]main()
|   return 0
