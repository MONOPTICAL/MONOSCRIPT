// expect-error: Type mismatch
[result<i32>]f()
|   return ok(1)

[i32]main()
|   result<string> r = f()
|   return 0
