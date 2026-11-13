// expect-error: Unsupported operand types
[result<i32>]f()
|   return ok(1)

[i32]main()
|   result<i32> a = f()
|   result<i32> b = f()
|   if a == b
|   |   return 1
|   return 0
