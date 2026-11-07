// expect-error: Pure function cannot create a new reference
[result<void>]mutate(result<array<i32>>: r) @pure
|   array<i32> a = r?
|   a[0] = 9
|   return ok()
[i32]main()
|   return 0
