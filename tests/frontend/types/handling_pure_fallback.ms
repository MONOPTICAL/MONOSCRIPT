// expect-error: Pure function cannot create a new reference
[array<i32>]f(array<i32>: borrowed) @pure
|   result<array<i32>> r = err("bad")
|   return r? else borrowed
[i32]main()
|   return 0
